#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "app_nvs_flash.h"
#include "app_mqtt.h"
#include "app_config.h"
#include "app_led_btn.h"
#include "esp_log.h"

static const char *TAG = "main app";

char HOMEID[30] = "homeid";
char USERID[30];
char CLIENTID[30];

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    // app_flash_set_userid("user12345");
    // app_flash_set_homeid("home12345");
    app_flash_get_device_info(CLIENTID, USERID, HOMEID);
    app_led_btn_init();
    app_mqtt_init();
    app_config_start();
}
