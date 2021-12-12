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
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "lib_gpio.h"

static const char *TAG = "main";

#define GPIO_OUTPUT_IO_1    16
#define GPIO_INPUT_IO_0     5

void app_main(void)
{
    lib_gpio_output_init(GPIO_OUTPUT_IO_1);
    lib_gpio_input_init(GPIO_INPUT_IO_0, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);

    while (1) {
        lib_gpio_button_click(GPIO_INPUT_IO_0);
        
        vTaskDelay(1 / portTICK_RATE_MS);
    }
}


