#include <stdio.h>
#include "drv_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define DBG_TAG           "drv_led"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

/*********************************************************************************************
* 名称：led_pin_init()
* 功能：LED引脚初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void led_pin_init(void)
{
#if defined(BOARD_CONFIG_HAL_LED0_GPIO)
    gpio_reset_pin(BOARD_CONFIG_HAL_LED0_GPIO);
    gpio_set_direction(BOARD_CONFIG_HAL_LED0_GPIO, GPIO_MODE_OUTPUT);
#endif
#if defined(BOARD_CONFIG_HAL_LED1_GPIO)
    gpio_reset_pin(BOARD_CONFIG_HAL_LED1_GPIO);
    gpio_set_direction(BOARD_CONFIG_HAL_LED1_GPIO, GPIO_MODE_OUTPUT);
#endif
#if defined(BOARD_CONFIG_HAL_LED2_GPIO)
    gpio_reset_pin(BOARD_CONFIG_HAL_LED2_GPIO);
    gpio_set_direction(BOARD_CONFIG_HAL_LED2_GPIO, GPIO_MODE_OUTPUT);
#endif
#if defined(BOARD_CONFIG_HAL_LED3_GPIO)
    gpio_reset_pin(BOARD_CONFIG_HAL_LED3_GPIO);
    gpio_set_direction(BOARD_CONFIG_HAL_LED3_GPIO, GPIO_MODE_OUTPUT);
#endif
}

// cmd：每位控制一个LED灯，为1时LED点亮 为0熄灭
void led_ctrl(unsigned char cmd)
{
#if defined(BOARD_CONFIG_HAL_LED0_GPIO)
  gpio_set_level(BOARD_CONFIG_HAL_LED0_GPIO, 
    (uint32_t )(BOARD_CONFIG_HAL_LED0_ACTIVATE ? (cmd & _HAL_LED0_NUM) : !(cmd & _HAL_LED0_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED1_GPIO)
  gpio_set_level(BOARD_CONFIG_HAL_LED1_GPIO, 
    (uint32_t)(BOARD_CONFIG_HAL_LED1_ACTIVATE ? (cmd & _HAL_LED1_NUM) : !(cmd & _HAL_LED1_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED2_GPIO)
  gpio_set_level(BOARD_CONFIG_HAL_LED2_GPIO, 
    (uint32_t)(BOARD_CONFIG_HAL_LED2_ACTIVATE ? (cmd & _HAL_LED2_NUM) : !(cmd & _HAL_LED2_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED3_GPIO)
  gpio_set_level(BOARD_CONFIG_HAL_LED3_GPIO, 
    (uint32_t)(BOARD_CONFIG_HAL_LED3_ACTIVATE ? (cmd & _HAL_LED3_NUM) : !(cmd & _HAL_LED3_NUM)));
#endif
}

// index LED 索引，value：为1时LED点亮 为0熄灭
void led_write(unsigned char index ,unsigned char value)
{
  value = !!value;
  switch(index){
    case 0:
#if defined(BOARD_CONFIG_HAL_LED0_GPIO)
    gpio_set_level(BOARD_CONFIG_HAL_LED0_GPIO, 
      (uint32_t)(BOARD_CONFIG_HAL_LED0_ACTIVATE ? value : !value));
#endif
    break;
    
    case 1:
#if defined(BOARD_CONFIG_HAL_LED1_GPIO)
    gpio_set_level(BOARD_CONFIG_HAL_LED1_GPIO, 
      (uint32_t)(BOARD_CONFIG_HAL_LED1_ACTIVATE ? value : !value));
#endif
    break;
    
    case 2:
#if defined(BOARD_CONFIG_HAL_LED2_GPIO)
    gpio_set_level(BOARD_CONFIG_HAL_LED2_GPIO, 
      (uint32_t)(BOARD_CONFIG_HAL_LED2_ACTIVATE ? value : !value));
#endif
    break;
    
    case 3:
#if defined(BOARD_CONFIG_HAL_LED3_GPIO)
    gpio_set_level(BOARD_CONFIG_HAL_LED3_GPIO, 
      (uint32_t)(BOARD_CONFIG_HAL_LED3_ACTIVATE ? value : !value));
#endif
    break;
    default:
    logfn_e("index error");
    break;
  }
}

