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

#define DBG_TAG           "take_picture"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

#define TEST_ESP_OK(ret) assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)

static bool auto_jpeg_support = false; // whether the camera sensor support compression or JPEG encode
static QueueHandle_t xQueueIFrame = NULL;

static const char *TAG = "video s_server";

esp_err_t start_stream_server(const QueueHandle_t frame_i, const bool return_fb);

static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count)
{
    camera_config_t camera_config = {
        .pin_pwdn = CAMERA_PIN_PWDN,
        .pin_reset = CAMERA_PIN_RESET,
        .pin_xclk = CAMERA_PIN_XCLK,
        .pin_sscb_sda = CAMERA_PIN_SIOD,
        .pin_sscb_scl = CAMERA_PIN_SIOC,

        .pin_d7 = CAMERA_PIN_D7,
        .pin_d6 = CAMERA_PIN_D6,
        .pin_d5 = CAMERA_PIN_D5,
        .pin_d4 = CAMERA_PIN_D4,
        .pin_d3 = CAMERA_PIN_D3,
        .pin_d2 = CAMERA_PIN_D2,
        .pin_d1 = CAMERA_PIN_D1,
        .pin_d0 = CAMERA_PIN_D0,
        .pin_vsync = CAMERA_PIN_VSYNC,
        .pin_href = CAMERA_PIN_HREF,
        .pin_pclk = CAMERA_PIN_PCLK,

        //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0, // // This is only valid on ESP32/ESP32-S2. ESP32-S3 use LCD_CAM interface.
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    //QQVGA-UXGA, sizes above QVGA are not been recommended when not JPEG format.

        .jpeg_quality = 10, //0-63,数值越小，图形质量越好，占用空间越大
        .fb_count = fb_count,       // For ESP32/ESP32-S2, if more than one, i2s runs in continuous mode. Use only with JPEG.
        .grab_mode = CAMERA_GRAB_LATEST,
        .fb_location = CAMERA_FB_IN_PSRAM
    };

    //initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);//flip it back
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_saturation(s, -2);//lower the saturation
    }

    if (s->id.PID == OV3660_PID || s->id.PID == OV2640_PID) {
        s->set_vflip(s, 1); //flip it back
    } else if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 0);
    } else if (s->id.PID == GC032A_PID) {
        s->set_vflip(s, 1);
    }

    camera_sensor_info_t *s_info = esp_camera_sensor_get_info(&(s->id));

    if (ESP_OK == ret && PIXFORMAT_JPEG == pixel_format && s_info->support_jpeg == true) {
        auto_jpeg_support = true;
    }

    return ret;
}

static void esp_cam_stream_task(void *arg)
{
    camera_fb_t *frame;

    // 用于计算帧率
    static uint32_t frame_count = 0;
    static uint32_t start_time = 0;
    uint32_t current_time;
    
    ESP_LOGI(TAG, "Begin capture frame");
    // 初始化计时器
    start_time = pdTICKS_TO_MS(xTaskGetTickCount());
    while (true) {
        frame = esp_camera_fb_get();
        if (frame) {
            xQueueSend(xQueueIFrame, &frame, portMAX_DELAY);
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

void esp_cam_stream_init(void)
{
    app_wifi_main();

    xQueueIFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    /* It is recommended to use a camera sensor with JPEG compression to maximize the speed */
    // FRAMESIZE_HQVGA,    // 240x176
    // FRAMESIZE_240X240,  // 240x240
    // FRAMESIZE_QVGA,     // 320x240
    // FRAMESIZE_320X320,  // 320x320
    TEST_ESP_OK(init_camera(48 * 1000000, PIXFORMAT_JPEG, FRAMESIZE_240X240, 2));

    TEST_ESP_OK(start_stream_server(xQueueIFrame, true));

    // 创建任务并绑定到 CPU1
    BaseType_t task_created = xTaskCreatePinnedToCore(&esp_cam_stream_task, "cam_stream", 6144, NULL, 5, NULL, 1);
    // 使用 ESP_ERROR_CHECK 检查是否成功创建任务
    ESP_ERROR_CHECK(task_created == pdPASS ? ESP_OK : ESP_FAIL);
}