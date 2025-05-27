#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _HAL_LED0_NUM              0x01
#define _HAL_LED1_NUM              0x02
#define _HAL_LED2_NUM              0x04
#define _HAL_LED3_NUM              0x08

void led_pin_init(void);
void led_ctrl(unsigned char cmd);
void led_write(unsigned char index ,unsigned char value);
/* declarations go here */

#ifdef __cplusplus
}
#endif