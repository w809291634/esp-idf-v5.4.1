#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Using SPI2 in the example
#define LCD_HOST  SPI2_HOST

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (80 * 1000 * 1000)       // 默认 20 ，可以设置到30
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

#ifdef CONFIG_LCD_SPI_USE_IOMUX_PINS
// 标准 IO MUX 管脚
#define EXAMPLE_PIN_NUM_SCLK           GPIO_NUM_12
#define EXAMPLE_PIN_NUM_MOSI           GPIO_NUM_11
#define EXAMPLE_PIN_NUM_MISO           GPIO_NUM_13
#define EXAMPLE_PIN_NUM_LCD_DC         GPIO_NUM_4
#define EXAMPLE_PIN_NUM_LCD_RST        GPIO_NUM_5
#define EXAMPLE_PIN_NUM_LCD_CS         GPIO_NUM_10
#define EXAMPLE_PIN_NUM_BK_LIGHT       GPIO_NUM_6

#define EXAMPLE_PIN_NUM_TOUCH_CS       -1
#else
// GPIO 交换矩阵 任意管脚 , 需要 SPICOMMON_BUSFLAG_GPIO_PINS 标记
#define EXAMPLE_PIN_NUM_SCLK           GPIO_NUM_1
#define EXAMPLE_PIN_NUM_MOSI           GPIO_NUM_2
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         GPIO_NUM_42
#define EXAMPLE_PIN_NUM_LCD_RST        GPIO_NUM_41
#define EXAMPLE_PIN_NUM_LCD_CS         GPIO_NUM_47
#define EXAMPLE_PIN_NUM_BK_LIGHT       GPIO_NUM_21
#endif

#define EXAMPLE_PIN_NUM_TOUCH_CS       -1

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              320
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              240
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_ST7789
#define EXAMPLE_LCD_H_RES              240
#define EXAMPLE_LCD_V_RES              320
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define EXAMPLE_LVGL_DRAW_BUF_LINES    60 // number of display lines in each draw buffer
                                            // 越大 lvgl 计算时间越短，默认20,越大速度快很多
                                            // 与 wifi AP 一起使用时，这里SRAM 不能够使用太多，会导致 wifi 无法连接成功，这里可以设置到 60 
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (6 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY     2

#define USE_CAM_MODE                     3  // 0:使用rtsp推流 1：使用LCD显示（不使用lvgl） 2：使用LCD显示（使用lvgl） 3:获取lvgl的绘图buffer

#define DRAW_BUFFER_STATIC_ALLOC
// 启动了 psram 后，这里可以使用静态分配，其他动态内存使用 psram
#ifdef DRAW_BUFFER_STATIC_ALLOC
#define DRAW_BUFFER_SIZE    (size_t)(EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t))
#endif

void lvgl_init(void);
/* declarations go here */

#ifdef __cplusplus
}
#endif
