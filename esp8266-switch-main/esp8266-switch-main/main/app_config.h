#ifndef _APP_CONFIG_H
#define _APP_CONFIG_H

typedef enum{
    ESP_PROVISION_MODE_ACCESSPOINT = 0,
    ESP_PROVISION_MODE_SMARTCONFIG = 1
}   esp_provision_mode_t;

void app_config_start(void);

#endif