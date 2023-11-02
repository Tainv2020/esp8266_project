#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/hw_timer.h"
#include "lib_timer.h"

/*************************************** DEFINE **************************************************/

/*************************************** VARIABLES  **********************************************/
TimerHandle_t xTimers[MAX_TIMER];

/*************************************** FUNCTION ************************************************/
void lib_sw_timer_init(void *callback)
{
    /* Create timer */
    xTimers[TIMER0_ID] = xTimerCreate("timer task 0", pdMS_TO_TICKS(10), pdTRUE, (void *)0, callback);
    xTimers[TIMER1_ID] = xTimerCreate("timer task 1", pdMS_TO_TICKS(10), pdTRUE, (void *)1, callback);
    xTimers[TIMER2_ID] = xTimerCreate("timer task 2", pdMS_TO_TICKS(10), pdTRUE, (void *)2, callback);
}

void lib_sw_timer_start(uint8_t instance)
{
    xTimerStart(xTimers[instance], 0);
}

void lib_sw_timer_stop(uint8_t instance)
{
    xTimerStop(xTimers[instance], 0);
}

void lib_hw_timer_start(void *callback)
{
    hw_timer_init(callback, NULL);
    hw_timer_alarm_us(10000, true); /* 10ms */
}

void lib_hw_timer_stop(void)
{
    /* Disable hw timer */
    hw_timer_disarm();
    hw_timer_deinit();
}