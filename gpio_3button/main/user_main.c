/* gpio example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "lib_gpio.h"
#include "lib_timer.h"

static const char *TAG = "main";

static void button_task(void *arg)
{
    lib_gpio_stt_3button_t stt;
    while(1)
    {
        stt = lib_gpio_check_button();
        lib_gpio_display_led(GPIO_LED1, stt);
        
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    lib_gpio_init();

    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
}


