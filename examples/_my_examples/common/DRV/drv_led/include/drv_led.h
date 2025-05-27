#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_pin_init(void);
void led_ctrl(unsigned char cmd);
void led_write(unsigned char index ,unsigned char value);
/* declarations go here */

#ifdef __cplusplus
}
#endif