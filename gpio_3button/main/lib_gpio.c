#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/hw_timer.h"
#include "lib_gpio.h"
#include "lib_timer.h"

/*************************************** VARIABLES **************************************************/
static const char *TAG = "lib_gpio";
static lib_gpio_button_status_type stt_button[3] = {is_not_press, is_not_press, is_not_press};
static uint32_t time_press_button[3] = {0, 0, 0};

/*************************************** STATIC FUNCTION ************************************************/
/* Function handle interrupt event */
static void lib_gpio_input_handler(void *arg);
/* Timer call backfunction */
static void lib_gpio_hw_timer_callback(void *arg);
/* Read time press button */
static lib_gpio_button_click_type lib_gpio_stt_button(gpio_num_t pin);

/*************************************** FUNCTION ************************************************/
/* Function handle interrupt event */
static void lib_gpio_input_handler(void *arg)
{
}

/* Timer call backfunction */
static void lib_gpio_hw_timer_callback(void *arg)
{
    if(stt_button[0] == is_press)
    {
        time_press_button[0] += 1;
    }
    if(stt_button[1] == is_press)
    {
        time_press_button[1] += 1;
    }
    if(stt_button[2] == is_press)
    {
        time_press_button[2] += 1;
    }
}

/* Read time press button */
static lib_gpio_button_click_type lib_gpio_stt_button(gpio_num_t pin)
{
   lib_gpio_button_click_type retVal = is_unknow_click;

   switch (pin)
   {
      case GPIO_BUTTON1:
      {
         /* Button 1 */
         if(stt_button[0] == is_press)
         {
            if(time_press_button[0] > MAX_TIME_PRESS)
            {
               ESP_LOGI(TAG, "1 Pair click");
               retVal = is_pair_click;
            }
            else if(time_press_button[0] > LONG_CLICK_TIME)
            {
               ESP_LOGI(TAG, "1 Long click");
               retVal = is_long_click;
            }
            else if(time_press_button[0] > SHORT_CLICK_TIME)
            {
               ESP_LOGI(TAG, "1 Short click");
               retVal = is_short_click;
            }
            else if(time_press_button[0] < UNKONW_CLICK_TIME)
            {
               ESP_LOGI(TAG, "1 Unknow click");
               retVal = is_unknow_click;
            }

            /* Clear variable */
            stt_button[0] = is_not_press;
            time_press_button[0] = 0;
         }
         break;
      }
      case GPIO_BUTTON2:
      {
         /* Button 2 */
         if(stt_button[1] == is_press)
         {
            if(time_press_button[1] > MAX_TIME_PRESS)
            {
               ESP_LOGI(TAG, "2 Pair click");
               retVal = is_pair_click;
            }
            else if(time_press_button[1] > LONG_CLICK_TIME)
            {
               ESP_LOGI(TAG, "2 Long click");
               retVal = is_long_click;
            }
            else if(time_press_button[1] > SHORT_CLICK_TIME)
            {
               ESP_LOGI(TAG, "2 Short click");
               retVal = is_short_click;
            }
            else if(time_press_button[1] < UNKONW_CLICK_TIME)
            {
               ESP_LOGI(TAG, "2 Unknow click");
               retVal = is_unknow_click;
            }

            /* Clear variable */
            stt_button[1] = is_not_press;
            time_press_button[1] = 0;
         }
         break;
      }
      case GPIO_BUTTON3:
      {
         /* Button 3 */
         if(stt_button[2] == is_press)
         {
            if(time_press_button[2] > MAX_TIME_PRESS)
            {
               ESP_LOGI(TAG, "3 Pair click");
               retVal = is_pair_click;
            }
            else if(time_press_button[2] > LONG_CLICK_TIME)
            {
               ESP_LOGI(TAG, "3 Long click");
               retVal = is_long_click;
            }
            else if(time_press_button[2] > SHORT_CLICK_TIME)
            {
               ESP_LOGI(TAG, "3 Short click");
               retVal = is_short_click;
            }
            else if(time_press_button[2] < UNKONW_CLICK_TIME)
            {
               ESP_LOGI(TAG, "3 Unknow click");
               retVal = is_unknow_click;
            }

            /* Clear variable */
            stt_button[2] = is_not_press;
            time_press_button[2] = 0;
         }
         break;
      }
   
      default:
         break;
   }

   return retVal;
}

/* Init input pin*/
void lib_gpio_input_init(gpio_num_t pin, gpio_pull_mode_t pull_type, gpio_int_type_t interrupt_type)
{
   gpio_set_direction(pin, GPIO_MODE_INPUT);
   gpio_set_pull_mode(pin, pull_type);
   gpio_set_intr_type(pin, interrupt_type);
}

/* Init output pin */
void lib_gpio_output_init(gpio_num_t pin)
{
   gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

/* Write pin */
void lib_gpio_write(gpio_num_t pin, uint32_t level)
{
   gpio_set_level(pin, level);
}

/* Read pin */
int lib_gpio_read(gpio_num_t pin)
{
   return gpio_get_level(pin);
}

/* Toggle pin */
void lib_gpio_toggled(gpio_num_t pin)
{
   uint32_t value = gpio_get_level(pin);

   gpio_set_level(pin, !value);
}

/* Init pin */
void lib_gpio_init(void)
{
   lib_gpio_output_init(GPIO_LED1);
   lib_gpio_output_init(GPIO_LED2);
   lib_gpio_output_init(GPIO_LED3);
   lib_gpio_input_init(GPIO_BUTTON1, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);
   lib_gpio_input_init(GPIO_BUTTON2, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);
   lib_gpio_input_init(GPIO_BUTTON3, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);
}

/* Check button */
lib_gpio_stt_3button_t lib_gpio_check_button(void)
{
   bool start_progress = false;
   lib_gpio_stt_3button_t retVal;

   /* Press */
   if((!lib_gpio_read(GPIO_BUTTON1)) || !lib_gpio_read(GPIO_BUTTON2) || !lib_gpio_read(GPIO_BUTTON3))
   {
      vTaskDelay(10 / portTICK_RATE_MS);
      if(!lib_gpio_read(GPIO_BUTTON1))
      {
         stt_button[0] = is_press;
      }
      if(!lib_gpio_read(GPIO_BUTTON2))
      {
         stt_button[1] = is_press;
      }
      if(!lib_gpio_read(GPIO_BUTTON3))
      {
         stt_button[2] = is_press;
      }
      start_progress = true;
      /* Enable hw timer */
      lib_hw_timer_start(&lib_gpio_hw_timer_callback);
   }

   while((!lib_gpio_read(GPIO_BUTTON1)) || (!lib_gpio_read(GPIO_BUTTON2) || (!lib_gpio_read(GPIO_BUTTON3))))
   {
      /* Press */
      /* Button 1 */
      if(!lib_gpio_read(GPIO_BUTTON1))
      {
         vTaskDelay(10 / portTICK_RATE_MS);
         if(!lib_gpio_read(GPIO_BUTTON1))
         {
            stt_button[0] = is_press;
         }
      }
      else
      {
         if(stt_button[0] == is_press)
         {
            retVal.button1 = lib_gpio_stt_button(GPIO_BUTTON1);
            stt_button[0] = is_not_press;
         }
      }
      /* Button 2 */
      if(!lib_gpio_read(GPIO_BUTTON2))
      {
         vTaskDelay(10 / portTICK_RATE_MS);
         if(!lib_gpio_read(GPIO_BUTTON2))
         {
            stt_button[1] = is_press;
         }
      }
      else
      {
         if(stt_button[1] == is_press)
         {
            retVal.button2 = lib_gpio_stt_button(GPIO_BUTTON2);
            stt_button[1] = is_not_press;
         }
      }
      /* Button 3 */
      if(!lib_gpio_read(GPIO_BUTTON3))
      {
         vTaskDelay(10 / portTICK_RATE_MS);
         if(!lib_gpio_read(GPIO_BUTTON3))
         {
            stt_button[2] = is_press;
         }
      }
      else
      {
         if(stt_button[2] == is_press)
         {
            retVal.button3 = lib_gpio_stt_button(GPIO_BUTTON3);
            stt_button[2] = is_not_press;
         }
      }

      /* If is Part click */
      if(time_press_button[0] > MAX_TIME_PRESS)
      {
         retVal.button1 = is_pair_click;
      }
      if(time_press_button[1] > MAX_TIME_PRESS)
      {
         retVal.button2 = is_pair_click;
      }
      if(time_press_button[2] > MAX_TIME_PRESS)
      {
         retVal.button3 = is_pair_click;
      }
   }

   if(start_progress)
   {
      ESP_LOGI(TAG, "time button1 %d, time button2 %d, time button3 %d", time_press_button[0], time_press_button[1], time_press_button[2]);
      
      retVal.button1 = lib_gpio_stt_button(GPIO_BUTTON1);
      retVal.button2 = lib_gpio_stt_button(GPIO_BUTTON2);
      retVal.button3 = lib_gpio_stt_button(GPIO_BUTTON3);

      /* Disable hw timer */
      lib_hw_timer_stop();
   }

   return retVal;
}

/* Display led */
void lib_gpio_display_led(gpio_num_t pin, lib_gpio_stt_3button_t stt)
{
   switch(stt.button1)
   {
      case is_pair_click:
      {
         for(uint8_t i = 0; i < 4; i ++)
         {
            lib_gpio_toggled(pin);
            vTaskDelay(DELAY_LED_STT / portTICK_RATE_MS);
         }
         break;
      }
      case is_short_click:
      {
         lib_gpio_toggled(pin);
         break;
      }
      case is_long_click:
      {
         break;
      }
      case is_2button_click:
      {
         break;
      }
      case is_3button_click:
      {
         break;
      }
      default:
         break;
   }
}
