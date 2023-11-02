/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/apps/sntp.h"
#include "lib_gpio.h"

/* Time to turn on and turn off Led */
#define JAN_MONTH       0
#define OCT_MONTH       9
#define NOV_MONTH       10
#define DEC_MONTH       11

#define HOUR_WINTER_ON  17 /* 6 pm */
#define MIN_WINTER_ON   0
#define SEC_WINTER_ON   0
#define HOUR_WINTER_OFF 6 /* 6 am */
#define MIN_WINTER_OFF  0
#define SEC_WINTER_OFF  0

#define HOUR_SUMMER_ON  19 /* 7 pm */
#define MIN_SUMMER_ON   0
#define SEC_SUMMER_ON   0
#define HOUR_SUMMER_OFF 5 /* 5 am */
#define MIN_SUMMER_OFF  0
#define SEC_SUMMER_OFF  0

static const char *TAG = "sntp_example";

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

static void sntp_example_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);
    localtime_r(&now, &timeinfo);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
    }

    // Set timezone to Eastern Standard Time and print local time
    // setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    // tzset();

    // Set timezone to VietNam Standard Time
    setenv("TZ", "CST-7", 1);
    tzset();

    while (1) {
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year < (2016 - 1900)) {
            ESP_LOGE(TAG, "The current date/time error");
        } else {
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time in VietNam is: %s", strftime_buf);

            /* Time turn on/off for WINTER */
            if((timeinfo.tm_mon == JAN_MONTH) || (timeinfo.tm_mon == OCT_MONTH) || (timeinfo.tm_mon == NOV_MONTH) || (timeinfo.tm_mon == DEC_MONTH))
            {
                ESP_LOGI(TAG, "Is winter now");
                /* Time ON */
                if((timeinfo.tm_hour >= HOUR_WINTER_ON) || (timeinfo.tm_hour <= HOUR_WINTER_OFF))
                {
                    ESP_LOGI(TAG, "Led ON");
                    lib_gpio_write(GPIO_LED1, ON);
                }
                else
                {
                    ESP_LOGI(TAG, "Led OFF");
                    lib_gpio_write(GPIO_LED1, OFF);
                }
            }
            else
            {
                ESP_LOGI(TAG, "Is summer now");
                /* Time ON */
                if((timeinfo.tm_hour >= HOUR_SUMMER_ON) || (timeinfo.tm_hour <= HOUR_SUMMER_OFF))
                {
                    ESP_LOGI(TAG, "Led ON");
                    lib_gpio_write(GPIO_LED1, ON);
                }
                else
                {
                    ESP_LOGI(TAG, "Led OFF");
                    lib_gpio_write(GPIO_LED1, OFF);
                }
            }
        }

        /*ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());*/
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main()
{
    esp_err_t stt;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Setup GPIO pins*/
    lib_gpio_init();
    /* Wifi init */
    stt = example_connect();

    if(stt == ESP_OK)
    {
        lib_gpio_write(GPIO_LED2, ON);
    }

    // SNTP service uses LwIP, please allocate large stack space.
    xTaskCreate(sntp_example_task, "sntp_example_task", 2048, NULL, 10, NULL);
}
