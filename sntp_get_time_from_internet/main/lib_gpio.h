#ifndef __LIB_GPIO_H__
#define __LIB_GPIO_H__

/*************************************** DEFINE **************************************************/
#define GPIO_LED1        16
#define GPIO_LED2        15
#define GPIO_LED3        13
#define GPIO_BUTTON1     5
#define GPIO_BUTTON2     14
#define GPIO_BUTTON3     12

#define ON  0
#define OFF 1

/*************************************** ENUM  **************************************************/

/*************************************** FUNCTION ************************************************/
/* Init input pin*/
void lib_gpio_input_init(gpio_num_t pin, gpio_pull_mode_t pull_type, gpio_int_type_t interrupt_type);

/* Init output pin*/
void lib_gpio_output_init(gpio_num_t pin);

/* Write pin*/
void lib_gpio_write(gpio_num_t pin, uint32_t level);

/* Read pin*/
int lib_gpio_read(gpio_num_t pin);

/* Toggle pin*/
void lib_gpio_toggled(gpio_num_t pin);

/* Init pin*/
void lib_gpio_init(void);

#endif /* __LIB_GPIO_H__ */
