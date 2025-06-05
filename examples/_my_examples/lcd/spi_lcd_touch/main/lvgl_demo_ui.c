/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// This demo UI is adapted from LVGL official example: https://docs.lvgl.io/master/examples.html#loader-with-arc

#include "lvgl.h"
#include "lv_demos.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

static const char *TAG = "UI";

static lv_obj_t * btn;
static lv_display_rotation_t rotation = LV_DISP_ROTATION_0;

void example_lvgl_demo_ui(lv_display_t *disp)
{
    lv_demo_widgets();
    ESP_LOGI( TAG, "lv_demo_widgets complete" );
}
