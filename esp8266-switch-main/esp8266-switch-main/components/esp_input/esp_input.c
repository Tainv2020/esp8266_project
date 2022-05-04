#include "esp_input.h"
#include <stdbool.h>

void input_create(gpio_num_t pin, void *cb)
{
    static bool install_isr_service = false;
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1 << pin);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&gpio_conf);
    if(!install_isr_service){
        install_isr_service = true;
        gpio_install_isr_service(0);
    }
    gpio_isr_handler_add(pin, cb, (void *)pin);
}

int input_read(gpio_num_t pin)
{
    return gpio_get_level(pin);
}