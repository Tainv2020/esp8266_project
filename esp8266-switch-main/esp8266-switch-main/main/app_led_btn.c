#include <stdio.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "app_led_btn.h"
#include "custom_boards.h"
#include "esp_input.h"
#include "esp_output.h"

#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                             \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                                   \
        }
#define ERR_ASSERT(tag, param)  IOT_CHECK(tag, (param) == ESP_OK, ESP_FAIL)
#define POINT_ASSERT(tag, param, ret)    IOT_CHECK(tag, (param) != NULL, (ret))

static const char* TAG = "button_led";



#if TOUCHS_NUMBER > 0
static const uint8_t m_board_touch_list[TOUCHS_NUMBER] = TOUCHS_LIST;
static const uint8_t m_board_touch_mask[TOUCHS_NUMBER] = MASK_LIST;
static const uint8_t m_board_led_mask[TOUCHS_NUMBER] = MASK_LIST;
#endif

static void button_action_callback(int pin, int tick);

static EventGroupHandle_t btn_event_group;
#define BTN_PRESS_SHORT_BIT     BIT0
#define BTN_PRESS_LONG_BIT      BIT1
#define BTN_PRESS_TIMEOUT       BIT2


#define ms_to_ticks(ms)		(ms/portTICK_RATE_MS)

uint8_t button_short_press_mask = 0x00; 
uint8_t button_long_press_mask = 0x00; 
uint8_t button_timeout_mask = 0x00;
uint8_t button_press_mask = 0x00;
uint8_t led_mask = 0x00;

static uint32_t _start_press_time;
static uint32_t _end_press_time;
static uint32_t _tick_press;
static TimerHandle_t xTimer[TOUCHS_NUMBER];
static TimerHandle_t xTimerForLedEf;

static void IRAM_ATTR button_gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    int level = gpio_get_level(gpio_num);
    uint32_t rtc = xTaskGetTickCountFromISR();   // ms
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (level == 1) {
        _end_press_time = rtc;
        for(int i=0;i<TOUCHS_NUMBER;i++){
            if(gpio_num == m_board_touch_list[i]){
                xTimerStopFromISR(xTimer[i],&xHigherPriorityTaskWoken);
                button_press_mask &=~ m_board_touch_mask[i];
            }
        }        
        _tick_press = _end_press_time - _start_press_time;
        button_action_callback(gpio_num, _tick_press);
    } else {
        _start_press_time = rtc;
        for(int i=0;i<TOUCHS_NUMBER;i++){
            if(gpio_num == m_board_touch_list[i]){
                xTimerResetFromISR(xTimer[i],&xHigherPriorityTaskWoken);
                button_press_mask |= m_board_touch_mask[i];
            }
        }
    }
}

static void button_timeout_handle(TimerHandle_t pxTimer)
{
    int32_t lArrayIndex;
    // Optionally do something if the pxTimer parameter is NULL.
    configASSERT( pxTimer );
     // Which timer expired?
    lArrayIndex = ( int32_t ) pvTimerGetTimerID( pxTimer ); 
    // for(int i=0;i<TOUCHS_NUMBER;i++){
    //     if(button_press_mask & m_board_touch_mask[i]){
    button_timeout_mask |=  m_board_touch_mask[lArrayIndex];
    button_press_mask &=~ m_board_touch_mask[lArrayIndex];
        // }
    // }

    if(button_press_mask == 0x00){
        xEventGroupSetBits(btn_event_group, BTN_PRESS_TIMEOUT);
    }
}

static void button_action_callback(int pin, int tick)
{
    uint32_t __ms_press = tick * portTICK_PERIOD_MS;
    if(__ms_press < 300){
        if(pin == TOUCH0){
            button_short_press_mask |= MASK0;
        }
        else if(pin == TOUCH1){
            button_short_press_mask |= MASK1;
        }
        else if(pin == TOUCH2){
            button_short_press_mask |= MASK2;
        }
        xEventGroupSetBits(btn_event_group, BTN_PRESS_SHORT_BIT);
    }
    else if(__ms_press < 5000)
    {
        if(pin == TOUCH0){
            button_long_press_mask |= MASK0;
        }
        else if(pin == TOUCH1){
            button_long_press_mask |= MASK1;
        }
        else if(pin == TOUCH2){
            button_long_press_mask |= MASK2;
        }
        xEventGroupSetBits(btn_event_group, BTN_PRESS_LONG_BIT);
    }
} 


static void app_button_task(void * parm)
{
    EventBits_t uxBits;
    while(1)
    {
        uxBits = xEventGroupWaitBits(
            btn_event_group,   /* The event group being tested. */
            BTN_PRESS_SHORT_BIT | BTN_PRESS_LONG_BIT | BTN_PRESS_TIMEOUT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_1 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            ms_to_ticks(3000) );/* Wait a maximum of 100ms for either bit to be set. */
        if(uxBits & BTN_PRESS_SHORT_BIT)
        {
            if(button_short_press_mask & MASK0){
                output_toggle_level(LED_RL_0);
                button_short_press_mask &=~MASK0;
            }
            if(button_short_press_mask & MASK1){
                output_toggle_level(LED_RL_1);
                button_short_press_mask &=~MASK1;
            }
            if(button_short_press_mask & MASK2){
                output_toggle_level(LED_RL_2);
                button_short_press_mask &=~MASK2;
            }
        }
        if(uxBits & BTN_PRESS_LONG_BIT)
        {
            if(button_long_press_mask & MASK0){
                output_toggle_level(LED_RL_0);
                button_long_press_mask &=~MASK0;
            }
            if(button_long_press_mask & MASK1){
                output_toggle_level(LED_RL_1);
                button_long_press_mask &=~MASK1;
            }
            if(button_long_press_mask & MASK2){
                output_toggle_level(LED_RL_2);
                button_long_press_mask &=~MASK2;
            }
        }
        if(uxBits & BTN_PRESS_TIMEOUT)
        {
            if(button_timeout_mask == 0b00000111){
                app_led_set_effect_blink(BSP_LED_0|BSP_LED_1|BSP_LED_2, 100);
            }   
            else if(button_timeout_mask == 0b00000011){
               app_led_set_effect_blink(BSP_LED_0|BSP_LED_1, 100);
            }   
            else if(button_timeout_mask == 0b00000101){
               app_led_set_effect_blink(BSP_LED_0|BSP_LED_2, 100);
            }               
            else if(button_timeout_mask == 0b00000110){
                app_led_set_effect_blink(BSP_LED_1|BSP_LED_2, 100);
            }
            else if(button_timeout_mask == 0b00000100){
                app_led_set_effect_blink(BSP_LED_2, 100);
            }
            else if(button_timeout_mask == 0b00000010){
                app_led_set_effect_blink(BSP_LED_1, 100);
            }
            else if(button_timeout_mask == 0b00000001){
                app_led_set_effect_blink(BSP_LED_0, 100);
            }
            button_timeout_mask = 0;
        }
    }
}

static void led_blink_timer_handle(TimerHandle_t xTimer)
{
    static uint8_t __cnt = 0;
    if(led_mask & MASK0){
        output_toggle_level(LED_RL_0);
    }
    if(led_mask & MASK1){
        output_toggle_level(LED_RL_1);
    }
    if(led_mask & MASK2){
        output_toggle_level(LED_RL_2);
    }
    if(__cnt++ >= 9){
        __cnt = 0;
        led_mask = 0;
        xTimerStop(xTimerForLedEf, 0);
    }
}

void app_led_set_effect_blink(uint8_t m_led_mask, int interval)
{
    led_mask = m_led_mask;
    xTimerChangePeriod(xTimerForLedEf,interval/portTICK_PERIOD_MS,0);
    xTimerReset(xTimerForLedEf,0);
}

void app_led_btn_init(void)
{
    for(int i=0;i<TOUCHS_NUMBER;i++){
        char name[10];
        sprintf(name, "xTimer[%d]", i); 
        xTimer[i] = xTimerCreate(name,                      // Just a text name, not used by the kernel.
                                (5000 / portTICK_PERIOD_MS ),    // 1000ms.
                                pdFALSE,                        // The timers will auto-reload themselves when they expire.
                                (void *) i,                     // Assign each timer a unique id equal to its array index.
                                button_timeout_handle);                // Each timer calls the same callback when it expires.
    }

    xTimerForLedEf = xTimerCreate("xTimerForLedEf",                      // Just a text name, not used by the kernel.
                                (100 / portTICK_PERIOD_MS ),    // 1000ms.
                                pdTRUE,                        // The timers will auto-reload themselves when they expire.
                                (void *) NULL,                     // Assign each timer a unique id equal to its array index.
                                led_blink_timer_handle); 

    output_create(LED_RL_0, 0);
    output_create(LED_RL_1, 0);
    output_create(LED_RL_2, 0);
    input_create(TOUCH0, button_gpio_isr_handler);
    input_create(TOUCH1, button_gpio_isr_handler);
    input_create(TOUCH2, button_gpio_isr_handler);
    btn_event_group = xEventGroupCreate();
    xTaskCreate(app_button_task, "app_button_task", 2048*2, NULL, 5, NULL);
}

