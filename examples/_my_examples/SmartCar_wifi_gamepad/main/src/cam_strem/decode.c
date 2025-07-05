
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "unity.h"
#include <inttypes.h>
#include <mbedtls/base64.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "esp_camera.h"

static const char *TAG = "decode";

typedef void (*decode_func_t)(uint8_t *jpegbuffer, uint32_t size, uint8_t *outbuffer);

static void tjpgd_decode_rgb565(uint8_t *mjpegbuffer, uint32_t size, uint8_t *outbuffer)
{
    jpg2rgb565(mjpegbuffer, size, outbuffer, JPG_SCALE_NONE);
}

// jpg：jpg 图像缓存区
// length: 图像缓存区长度
// img_w: 图像宽度
// img_h: 图像高度
// out_buf 输出指针
// out_length 长度
int jpg_decode2rgb565(uint8_t *jpg, uint32_t length, 
                    uint32_t img_w, uint32_t img_h,
                    uint8_t **out_buf,uint32_t* out_length)
{
    uint32_t rbg565_size = img_w * img_h * 2;
    uint8_t *rgb_buf = heap_caps_malloc(rbg565_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (NULL == rgb_buf) {
        ESP_LOGE(TAG, "malloc for rgb buffer failed");
        return -1;
    }
    // 解码 JPG 转换图像到 RGB565
    tjpgd_decode_rgb565(jpg, length, rgb_buf);
    *out_buf = rgb_buf;
    *out_length = rbg565_size;
    return 0;
}
