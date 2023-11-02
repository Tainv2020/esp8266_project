#ifndef __LIB_TIMER_H__
#define __LIB_TIMER_H__

/*************************************** INCLUDE *************************************************/

/*************************************** DEFINE **************************************************/
#define MAX_TIMER 3
#define TIMER0_ID 0
#define TIMER1_ID 1
#define TIMER2_ID 2

/*************************************** VARIABLES  **********************************************/

/*************************************** FUNCTION ************************************************/
void lib_sw_timer_init(void *callback);

void lib_sw_timer_start(uint8_t instance);

void lib_sw_timer_stop(uint8_t instance);

void lib_hw_timer_start(void *callback);

void lib_hw_timer_stop(void);

#endif /* __LIB_TIMER_H__ */