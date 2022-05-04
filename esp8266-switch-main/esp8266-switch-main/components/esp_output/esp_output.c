#include <stdio.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "esp_output.h"

esp_err_t output_create(gpio_num_t gpio_num, output_level_t level)
{
 /* Configure power */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
    };
    uint64_t pin_mask = ((uint64_t)1 << gpio_num );
    io_conf.pin_bit_mask = pin_mask;
    /* Configure the GPIO */
    gpio_config(&io_conf);
    gpio_set_level(gpio_num, level);    
    return ESP_OK;
}

esp_err_t output_set_level(gpio_num_t gpio_num, output_level_t level)
{
    gpio_set_level(gpio_num, level);  
    return ESP_OK;  
}

esp_err_t output_toggle_level(gpio_num_t gpio_num)
{
    int level = gpio_get_level(gpio_num);
    level = 1 - level;
    gpio_set_level(gpio_num, level);  
    return ESP_OK;  
}