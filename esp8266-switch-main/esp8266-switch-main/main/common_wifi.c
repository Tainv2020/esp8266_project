#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include "common_wifi.h"

static const char *TAG = "common_wifi";
#define AC_SSID_DEFAULT  "NhaAi"
#define AC_PASS_DEFAULT  "" 

void start_wifi_ap(void)
{
    /* Build WiFi configuration for AP mode */
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AC_SSID_DEFAULT,
            .ssid_len = strlen(AC_SSID_DEFAULT),
            .password = AC_PASS_DEFAULT,
            .max_connection = 3,
            .authmode = WIFI_AUTH_OPEN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void start_wifi_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}