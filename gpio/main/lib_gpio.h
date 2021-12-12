#ifndef __LIB_GPIO_H__
#define __LIB_GPIO_H__

/*************************************** DEFINE **************************************************/
#define MAX_TIME_PRESS         50 /* 5s */
#define LONG_CLICK_TIME        30 /* 3s */
#define SINGLE_CLICK_TIME      10 /* 1s */
#define N_CLICK_TIME           20 /* 2s */

/*************************************** ENUM  **************************************************/
typedef enum
{
    is_long_click = 0,
    is_single_click,
    is_double_click,
    is_triple_click,
    is_unknow
} lib_gpio_button_click_state_type;

/*************************************** FUNCTION ************************************************/
typedef void (*lib_gpio_callback_func_t) (int);

void lib_gpio_input_init(gpio_num_t pin, gpio_pull_mode_t pull_type, gpio_int_type_t interrupt_type);

void lib_gpio_output_init(gpio_num_t pin);

void lib_gpio_write(gpio_num_t pin, uint32_t level);

int lib_gpio_read(gpio_num_t pin);

void lib_gpio_toggled(gpio_num_t pin);

void lib_gpio_set_callback(void *func);

lib_gpio_button_click_state_type lib_gpio_button_click(gpio_num_t pin);

#endif /* __LIB_GPIO_H__ */