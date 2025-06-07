#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "drv_led.h"
#include "apl_console.h"
#include "apl_utility.h"

#define DBG_TAG           "main"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

void app_init(void){
    hw_board_init();
    app_info_dump();
    led_pin_init();
}

void app_main(void)
{
    app_init();
    for (;;) {
        logf_i("here");
        led_ctrl(0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        logfn_i("here");
        led_ctrl(0xff);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
