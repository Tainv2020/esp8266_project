#ifndef ESP_INPUT_H
#define ESP_INPUT_H
#include "driver/gpio.h"

void input_create(gpio_num_t pin, void *cb);
int input_read(gpio_num_t pin);
#endif
