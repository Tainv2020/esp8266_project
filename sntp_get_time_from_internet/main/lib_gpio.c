#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/hw_timer.h"
#include "lib_gpio.h"

/*************************************** VARIABLES **************************************************/
static const char *TAG = "lib_gpio";

/*************************************** STATIC FUNCTION ************************************************/
/* Function handle interrupt event */
static void lib_gpio_input_handler(void *arg);
/* Timer call backfunction */
static void lib_gpio_hw_timer_callback(void *arg);

/*************************************** FUNCTION ************************************************/
/* Function handle interrupt event */
static void lib_gpio_input_handler(void *arg)
{
}

/* Init input pin*/
void lib_gpio_input_init(gpio_num_t pin, gpio_pull_mode_t pull_type, gpio_int_type_t interrupt_type)
{
   gpio_set_direction(pin, GPIO_MODE_INPUT);
   gpio_set_pull_mode(pin, pull_type);
   gpio_set_intr_type(pin, interrupt_type);
}

/* Init output pin */
void lib_gpio_output_init(gpio_num_t pin)
{
   gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

/* Write pin */
void lib_gpio_write(gpio_num_t pin, uint32_t level)
{
   gpio_set_level(pin, level);
}

/* Read pin */
int lib_gpio_read(gpio_num_t pin)
{
   return gpio_get_level(pin);
}

/* Toggle pin */
void lib_gpio_toggled(gpio_num_t pin)
{
   uint32_t value = gpio_get_level(pin);

   gpio_set_level(pin, !value);
}

/* Init pin */
void lib_gpio_init(void)
{
   lib_gpio_output_init(GPIO_LED1);
   lib_gpio_output_init(GPIO_LED2);
   lib_gpio_write(GPIO_LED1, OFF);
   lib_gpio_write(GPIO_LED2, OFF);
   /*lib_gpio_output_init(GPIO_LED2);
   lib_gpio_output_init(GPIO_LED3);
   lib_gpio_input_init(GPIO_BUTTON1, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);
   lib_gpio_input_init(GPIO_BUTTON2, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);
   lib_gpio_input_init(GPIO_BUTTON3, GPIO_PULLUP_ONLY, GPIO_INTR_DISABLE);*/
}
