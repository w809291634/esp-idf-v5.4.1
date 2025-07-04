/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "camera_pin.h"

static bool auto_jpeg_support = false; // whether the camera sensor support compression or JPEG encode
static const char *TAG = "example:take_picture";

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

        // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode.
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0, // This is only valid on ESP32/ESP32-S2. ESP32-S3 use LCD_CAM interface.
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    // QQVGA-UXGA, sizes above QVGA are not been recommended when not JPEG format.

        .jpeg_quality = 30, // 0-63, used only with JPEG format.
        .fb_count = fb_count,       // For ESP32/ESP32-S2, if more than one, i2s runs in continuous mode.
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_PSRAM
    };

    // initialize the camera sensor
    esp_err_t ret = esp_camera_init(&camera_config);
    if (ret != ESP_OK) {
        return ret;
    }

    // Get the sensor object, and then use some of its functions to adjust the parameters when taking a photo.
    // Note: Do not call functions that set resolution, set picture format and PLL clock,
    // If you need to reset the appeal parameters, please reinitialize the sensor.
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); // flip it back
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 1); // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }

    if (s->id.PID == OV3660_PID || s->id.PID == OV2640_PID) {
        s->set_vflip(s, 1); // flip it back
    } else if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 0);
    } else if (s->id.PID == GC032A_PID) {
        s->set_vflip(s, 1);
    }

    // Get the basic information of the sensor.
    camera_sensor_info_t *s_info = esp_camera_sensor_get_info(&(s->id));

    if (ESP_OK == ret && PIXFORMAT_JPEG == pixel_format && s_info->support_jpeg == true) {
        auto_jpeg_support = true;
    }

    return ret;
}

void app_main()
{
    // 用于计算帧率
    static uint32_t frame_count = 0;
    static uint32_t start_time = 0;
    uint32_t current_time;

    if (ESP_OK != init_camera(48 * 1000000, PIXFORMAT_JPEG, FRAMESIZE_240X240, 2)) {
        ESP_LOGE(TAG, "init camrea sensor fail");
        return;
    }
    // 初始化计时器
    start_time = pdTICKS_TO_MS(xTaskGetTickCount());
    // take a picture every two seconds and print the size of the picture.
    while (1) {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        if (pic) {
            // use pic->buf to access the image
            ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
            // To enable the frame buffer can be reused again.
            // Note: If you don't call fb_return(), the next time you call fb_get() you may get
            // an error "Failed to get the frame on time!"because there is no frame buffer space available.
            esp_camera_fb_return(pic);
        }

        // 帧计数增加
        frame_count++;
        // 获取当前时间（单位：ms）
        current_time = pdTICKS_TO_MS(xTaskGetTickCount());
        // 每过一秒打印一次帧率
        if ((current_time - start_time) >= 1000) {
            float fps = (float)frame_count / ((current_time - start_time) / 1000.0f);
            ESP_LOGI(TAG, "FPS: %.2f", fps);
            // 重置计数器
            frame_count = 0;
            start_time = current_time;
        }
        // vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
