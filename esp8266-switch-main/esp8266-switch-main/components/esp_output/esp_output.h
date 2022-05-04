#ifndef ESP_OUTPUT_H
#define ESP_OUTPUT_H
#include "esp_err.h"
#include "driver/gpio.h"

typedef enum{
    OUTPUT_LEVEL_HIGH = 1,
    OUTPUT_LEVEL_LOW = 0
}   output_level_t;

esp_err_t output_create(gpio_num_t gpio_num, output_level_t level);
esp_err_t output_set_level(gpio_num_t gpio_num, output_level_t level);
esp_err_t output_toggle_level(gpio_num_t gpio_num);
#endif
