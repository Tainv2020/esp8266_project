#ifndef __LIB_GPIO_H__
#define __LIB_GPIO_H__

/*************************************** DEFINE **************************************************/
#define GPIO_LED1        4
#define GPIO_LED2        15
#define GPIO_LED3        13
#define GPIO_BUTTON1     5
#define GPIO_BUTTON2     14
#define GPIO_BUTTON3     12

#define MAX_TIME_PRESS         500 /* 5s */
#define LONG_CLICK_TIME        300 /* 3s */
#define SHORT_CLICK_TIME       12 /* 120ms */
#define UNKONW_CLICK_TIME      7 /* 70ms */

#define DELAY_LED_STT          50

/*************************************** ENUM  **************************************************/
typedef enum
{
    is_long_click = 0,
    is_single_click,
    is_short_click,
    is_pair_click,
    is_2button_click,
    is_3button_click,
    is_unknow_click
} lib_gpio_button_click_type;

typedef enum
{
    is_not_press = 0,
    is_press
} lib_gpio_button_status_type;

typedef struct
{
    lib_gpio_button_click_type button1;
    lib_gpio_button_click_type button2;
    lib_gpio_button_click_type button3;
} lib_gpio_stt_3button_t;


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

/* Display led */
void lib_gpio_display_led(gpio_num_t pin, lib_gpio_stt_3button_t stt);

/* Check button */
lib_gpio_stt_3button_t lib_gpio_check_button(void);

#endif /* __LIB_GPIO_H__ */