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

#if defined(BOARD_CONFIG_HAL_LED0_PORT) && defined(BOARD_CONFIG_HAL_LED0_PORT)
  _HAL_LED0_RCC();
  GPIO_Initure.Pin=_HAL_LED0_PIN;
  HAL_GPIO_Init(_HAL_LED0_PORT,&GPIO_Initure);
  led_write(0,0);
#endif
#if defined(BOARD_CONFIG_HAL_LED1_PORT) && defined(BOARD_CONFIG_HAL_LED1_PORT)
  _HAL_LED1_RCC();
  GPIO_Initure.Pin=_HAL_LED1_PIN;
  HAL_GPIO_Init(_HAL_LED1_PORT,&GPIO_Initure);
  led_write(1,0);
#endif
#if defined(BOARD_CONFIG_HAL_LED2_PORT) && defined(BOARD_CONFIG_HAL_LED2_PORT)
  _HAL_LED2_RCC();
  GPIO_Initure.Pin=_HAL_LED2_PIN;
  HAL_GPIO_Init(_HAL_LED2_PORT,&GPIO_Initure);
  led_write(2,0);
#endif
#if defined(BOARD_CONFIG_HAL_LED3_PORT) && defined(BOARD_CONFIG_HAL_LED3_PORT)
  _HAL_LED3_RCC();
  GPIO_Initure.Pin=_HAL_LED3_PIN;
  HAL_GPIO_Init(_HAL_LED3_PORT,&GPIO_Initure);
  led_write(3,0);
#endif
}

// cmd：每位控制一个LED灯，为1时LED点亮 为0熄灭
void led_ctrl(unsigned char cmd)
{
#if defined(BOARD_CONFIG_HAL_LED0_PORT) && defined(BOARD_CONFIG_HAL_LED0_PORT)
  HAL_GPIO_WritePin(_HAL_LED0_PORT, _HAL_LED0_PIN, 
    (GPIO_PinState)(BOARD_CONFIG_HAL_LED0_ACTIVATE ? (cmd & _HAL_LED0_NUM) : !(cmd & _HAL_LED0_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED1_PORT) && defined(BOARD_CONFIG_HAL_LED1_PORT)
  HAL_GPIO_WritePin(_HAL_LED1_PORT, _HAL_LED1_PIN, 
    (GPIO_PinState)(BOARD_CONFIG_HAL_LED1_ACTIVATE ? (cmd & _HAL_LED1_NUM) : !(cmd & _HAL_LED1_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED2_PORT) && defined(BOARD_CONFIG_HAL_LED2_PORT)
  HAL_GPIO_WritePin(_HAL_LED2_PORT, _HAL_LED2_PIN, 
    (GPIO_PinState)(BOARD_CONFIG_HAL_LED2_ACTIVATE ? (cmd & _HAL_LED2_NUM) : !(cmd & _HAL_LED2_NUM)));
#endif
#if defined(BOARD_CONFIG_HAL_LED3_PORT) && defined(BOARD_CONFIG_HAL_LED3_PORT)
  HAL_GPIO_WritePin(_HAL_LED3_PORT, _HAL_LED3_PIN, 
    (GPIO_PinState)(BOARD_CONFIG_HAL_LED3_ACTIVATE ? (cmd & _HAL_LED3_NUM) : !(cmd & _HAL_LED3_NUM)));
#endif
}

// index LED 索引，value：为1时LED点亮 为0熄灭
void led_write(unsigned char index ,unsigned char value)
{
  value = !!value;
  switch(index){
    case 0:
#if defined(BOARD_CONFIG_HAL_LED0_PORT) && defined(BOARD_CONFIG_HAL_LED0_PORT)
    HAL_GPIO_WritePin(_HAL_LED0_PORT, _HAL_LED0_PIN, 
      (GPIO_PinState)(BOARD_CONFIG_HAL_LED0_ACTIVATE ? value : !value));
#endif
    break;
    
    case 1:
#if defined(BOARD_CONFIG_HAL_LED1_PORT) && defined(BOARD_CONFIG_HAL_LED1_PORT)
    HAL_GPIO_WritePin(_HAL_LED1_PORT, _HAL_LED1_PIN, 
      (GPIO_PinState)(BOARD_CONFIG_HAL_LED1_ACTIVATE ? value : !value));
#endif
    break;
    
    case 2:
#if defined(BOARD_CONFIG_HAL_LED2_PORT) && defined(BOARD_CONFIG_HAL_LED2_PORT)
    HAL_GPIO_WritePin(_HAL_LED2_PORT, _HAL_LED2_PIN, 
      (GPIO_PinState)(BOARD_CONFIG_HAL_LED2_ACTIVATE ? value : !value));
#endif
    break;
    
    case 3:
#if defined(BOARD_CONFIG_HAL_LED3_PORT) && defined(BOARD_CONFIG_HAL_LED3_PORT)
    HAL_GPIO_WritePin(_HAL_LED3_PORT, _HAL_LED3_PIN, 
      (GPIO_PinState)(BOARD_CONFIG_HAL_LED3_ACTIVATE ? value : !value));
#endif
    break;
    default:
    logfn_e("index error");
    break;
  }
}

