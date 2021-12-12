/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*************************************** INCLUDE **************************************************/
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lib_gpio.h"
#include "lib_smartConfig.h"
#include "lib_wifiAP.h"

/*************************************** DEFINE **************************************************/
#define LED_PIN        16
#define BUTTON_PIN     5

/*************************************** GLOBAL VARIABLE *****************************************/
static const char* TAG = "app main";

/*************************************** DEFINE FUNCTION *****************************************/

/***************************************  FUNCTION **********************************************/
void app_main()
{
    lib_gpio_button_click_state_type button_status = is_unknow;

    lib_gpio_output_init(LED_PIN);
    lib_gpio_input_init(BUTTON_PIN, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);

    while(1)
    {
        button_status = lib_gpio_button_click(BUTTON_PIN);

        if(button_status == is_long_click)
        {
            ESP_LOGW(TAG, "ESP SMARTCONFIG");
            initialise_wifi();
        }
        else if(button_status == is_single_click)
        {
            ESP_LOGW(TAG, "ESP WIFI AP MODE");
            wifi_init_softap();
        }

        vTaskDelay(NULL);
    }
}

