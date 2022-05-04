#ifndef __CUSTOM_BOARD_H
#define __CUSTOM_BOARD_H

#define TARGET  SWITCH_3
#define MASK0            0x01
#define MASK1            0x02
#define MASK2            0x04
#define MASK3            0x08

#if(TARGET == SWITCH_3)
#define TOUCHS_NUMBER    3
#define TOUCH0          5
#define TOUCH1          12
#define TOUCH2          14
#define LED_RL_0        15
#define LED_RL_1        13
#define LED_RL_2        4
#define COIL            0
#define LED_STT         16
#define TOUCHS_LIST { TOUCH0, TOUCH1, TOUCH2 }
#define MASK_LIST   {MASK0, MASK1, MASK2, MASK3};
#define BSP_LED_0       0x01
#define BSP_LED_1       0x02
#define BSP_LED_2       0x04
#define BSP_LED_3       0x08

#elif(TARGET == SWITCH_3)


#elif(TARGET == SWITCH_2)


#elif(TARGET == SWITCH_1)


#elif(TARGET == SONOFF3)


#elif(TARGET == SONOFF2)


#elif(TARGET == SONOFF1)


#endif

#endif