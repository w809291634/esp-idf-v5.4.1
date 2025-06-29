#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "drv_led.h"
#include "apl_console.h"
#include "apl_utility.h"
#include "lvgl_app.h"
#include "esp32_cam.h"

#define DBG_TAG           "main"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

void app_init(void)
{
    // led_pin_init();
    // lvgl_init();
    // esp_cam_stream_init();
}

void app_main(void)
{
    hw_board_init();
    app_init();
}
