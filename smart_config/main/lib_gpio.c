#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/hw_timer.h"
#include "lib_gpio.h"

static const char *TAG = "lib_gpio";
lib_gpio_callback_func_t call_back_func = NULL;

static bool isPress = false;
static uint32_t press_duration = 0;

/* Function handle interrupt event */
static void lib_gpio_input_handler(void *arg)
{
   int pin = (uint32_t) arg;
   call_back_func(pin);
}

static void lib_gpio_hw_timer_callback(void *arg)
{
   if(isPress)
   {
      press_duration += 1;
   }
}

void lib_gpio_input_init(gpio_num_t pin, gpio_pull_mode_t pull_type, gpio_int_type_t interrupt_type)
{
   gpio_set_direction(pin, GPIO_MODE_INPUT);
   gpio_set_pull_mode(pin, pull_type);
   gpio_set_intr_type(pin, interrupt_type);
   gpio_install_isr_service(0);
   gpio_isr_handler_add(pin, lib_gpio_input_handler, (void *)pin); /* Register interrupt function */
}

void lib_gpio_output_init(gpio_num_t pin)
{
   gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

void lib_gpio_write(gpio_num_t pin, uint32_t level)
{
   gpio_set_level(pin, level);
}

int lib_gpio_read(gpio_num_t pin)
{
   return gpio_get_level(pin);
}

void lib_gpio_toggled(gpio_num_t pin)
{
   uint32_t value = gpio_get_level(pin);

   gpio_set_level(pin, !value);
}

void lib_gpio_set_callback(void *func)
{
   call_back_func = func;
}

lib_gpio_button_click_state_type lib_gpio_button_click(gpio_num_t pin)
{
   uint8_t press_time = 0;
   bool start_progress = false;
   lib_gpio_button_click_state_type retVal = is_unknow;

   /* Press */
   if(!lib_gpio_read(pin))
   {
      vTaskDelay(10 / portTICK_RATE_MS);
      if(!lib_gpio_read(pin))
      {
         isPress = true;
         start_progress = true;
         press_time += 1;
         /* Enable hw timer */
         hw_timer_init(lib_gpio_hw_timer_callback, NULL);
         hw_timer_alarm_us(100000, true); /* 100ms */
      }
   }

   while(!lib_gpio_read(pin));

   if(start_progress)
   {
      if(press_duration < SINGLE_CLICK_TIME)
      {
         while(press_duration < N_CLICK_TIME)
         {
            if(!lib_gpio_read(pin))
            {
               vTaskDelay(10 / portTICK_RATE_MS);
               if(!lib_gpio_read(pin))
                  press_time += 1;
               while(!lib_gpio_read(pin));
            }
         }

         ESP_LOGI(TAG, "Time click: %d", press_time);
         switch(press_time)
         {
            case 2:
            {
               retVal = is_double_click;
               break;
            }
            case 3:
            {
               retVal = is_triple_click;
               break;
            }
            default:
               break;
         }
      }
      else
      {
         if(press_duration > MAX_TIME_PRESS)
         {
            ESP_LOGI(TAG, "Unknow click");
            retVal = is_unknow;
         }
         else if(press_duration > LONG_CLICK_TIME)
         {
            ESP_LOGI(TAG, "Long click");
            retVal = is_long_click;
         }
         else
         {
            ESP_LOGI(TAG, "Single click");
            retVal = is_single_click;
         }
      }

      /* Clear variable */
      isPress = false;
      press_duration = 0;
      /* Disable hw timer */
      hw_timer_disarm();
      hw_timer_deinit();
   }
   

   return retVal;
}
