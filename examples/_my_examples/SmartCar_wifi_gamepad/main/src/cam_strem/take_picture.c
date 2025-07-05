/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "camera_pin.h"
#include "esp_camera.h"
#include "esp32_cam.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "lvgl.h"
#include "lvgl_app.h"

#if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
#include "esp_lcd_ili9341.h"
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
#include "esp_lcd_gc9a01.h"
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_ST7789
#include "esp_lcd_panel_st7789.h"
#endif

#if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
#include "esp_lcd_touch_stmpe610.h"
#endif

#define DBG_TAG           "take_picture"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

#define TEST_ESP_OK(ret) assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)

static const char *TAG = "video s_client";

extern uint8_t DRAM_ATTR lvgl_draw_buf1[DRAW_BUFFER_SIZE] __attribute__((aligned(16)));
extern uint8_t DRAM_ATTR lvgl_draw_buf2[DRAW_BUFFER_SIZE] __attribute__((aligned(16)));

static void esp_cam_stream_task(void *arg)
{
    camera_fb_t *frame;
    uint8_t *out_buf;
    uint32_t out_length;
    extern esp_lcd_panel_handle_t panel_handle;

    // 用于计算帧率
    static uint32_t frame_count = 0;
    static uint32_t start_time = 0;
    uint32_t current_time;
    
    ESP_LOGI(TAG, "Begin capture frame");

    // 初始化计时器
    start_time = pdTICKS_TO_MS(xTaskGetTickCount());
    while (true) {
        frame = stream_client_fb_get();
        if (frame) {
            /* 获取lvgl的显示buffer */
            if(jpg_decode2rgb565(frame->buf,frame->len,frame->width,frame->height,&out_buf,&out_length)==0){
                // because SPI LCD is big-endian, we need to swap the RGB bytes order
                lv_draw_sw_rgb565_swap(out_buf, out_length/2);      // 像素数量
                static uint8_t buf_switch = 0;
                for (int y = 0; y < 240; y += EXAMPLE_LVGL_DRAW_BUF_LINES) {
                    if(buf_switch%2 == 0)
                        memcpy(lvgl_draw_buf1,out_buf+(size_t)(EXAMPLE_LCD_H_RES * y * sizeof(lv_color16_t)),DRAW_BUFFER_SIZE);
                    else
                        memcpy(lvgl_draw_buf2,out_buf+(size_t)(EXAMPLE_LCD_H_RES * y * sizeof(lv_color16_t)),DRAW_BUFFER_SIZE);

                    // 80mhz的话，用时极其短。有可能运行到下面卡死
                    // x宽度240，高度也是显示240行，需要分段 拷贝到 buf1 ，最后显示到一帧里面
                    _lock_acquire(&lvgl_api_lock);
                    if(buf_switch%2 == 0)
                        esp_lcd_panel_draw_bitmap(panel_handle,0,y,240,y+EXAMPLE_LVGL_DRAW_BUF_LINES, lvgl_draw_buf1);  // 末行和末列会减1
                    else
                        esp_lcd_panel_draw_bitmap(panel_handle,0,y,240,y+EXAMPLE_LVGL_DRAW_BUF_LINES, lvgl_draw_buf2);  // 末行和末列会减1
                    _lock_release(&lvgl_api_lock);
                    buf_switch ++ ;
                }
                heap_caps_free(out_buf);
            }
            stream_client_fb_return(frame);

            // 帧计数增加
            frame_count++;
            // 获取当前时间（单位：ms）
            current_time = pdTICKS_TO_MS(xTaskGetTickCount());
            // 每过一秒打印一次帧率
            if ((current_time - start_time) >= 1000) {
                float fps = (float)frame_count / ((current_time - start_time) / 1000.0f);
                ESP_LOGI(TAG, "FPS: %.2f frame: %zu bytes", fps, frame->len);
                // 重置计数器
                frame_count = 0;
                start_time = current_time;
            }
        }
    }
    vTaskDelete(NULL);
}

#ifdef ESP_CAM_CLIENT_STREAM_ENABLE
static void esp_cam_stream_test_task(void *arg)
{
    uint32_t start_time = 0;
    // 初始化计时器
    start_time = pdTICKS_TO_MS(xTaskGetTickCount());
    while (true) {
#if 0
        static int __count = 0;
        if(__count > 5 && __count <= 6 ){
            stop_stream_client();
            ESP_LOGI(TAG, "client close");
        }else if(__count > 10 && __count <= 11 ){
            start_stream_client();
            ESP_LOGI(TAG, "restart client");
            __count = 0;
        }
        __count++ ;
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#if 1
        /* 进行复杂的动态切换测试 */
        stop_stream_client();
        start_stream_client();
        stop_stream_client();
        start_stream_client();
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    }
    vTaskDelete(NULL);
}
#endif

void esp_cam_client_stream_init(void)
{
    app_wifi_main();

    TEST_ESP_OK(start_stream_client());

    // 创建任务并绑定到 CPU1
    BaseType_t task_created = xTaskCreatePinnedToCore(&esp_cam_stream_task, "cam_show", 6144, NULL, 6, NULL, 1);
    // 使用 ESP_ERROR_CHECK 检查是否成功创建任务
    ESP_ERROR_CHECK(task_created == pdPASS ? ESP_OK : ESP_FAIL);

#ifdef ESP_CAM_CLIENT_STREAM_ENABLE
    task_created = xTaskCreatePinnedToCore(&esp_cam_stream_test_task, "cam_show_test", 6144, NULL, 7, NULL, 1);
    // 使用 ESP_ERROR_CHECK 检查是否成功创建任务
    ESP_ERROR_CHECK(task_created == pdPASS ? ESP_OK : ESP_FAIL);
#endif
}