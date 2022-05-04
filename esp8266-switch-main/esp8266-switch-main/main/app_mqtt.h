#ifndef __APP_MQTT_H
#define __APP_MQTT_H
#include <stdint.h>
#include <stddef.h>
#include "esp_mqtt.h"
void app_mqtt_init(void);
void app_mqtt_start(void);
void app_mqtt_publish(const char *topic, uint8_t *data, int data_len);
void app_mqtt_subscriber(const char *topic, esp_mqtt_subscribe_cb_t cb);
#endif
