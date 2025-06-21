
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

static void print_rgb565_img(uint8_t *img, int width, int height)
{
    uint16_t *p = (uint16_t *)img;
    const char temp2char[17] = "@MNHQ&#UJ*x7^i;.";
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            uint32_t c = p[j * width + i];
            uint8_t r = c >> 11;
            uint8_t g = (c >> 6) & 0x1f;
            uint8_t b = c & 0x1f;
            c = (r + g + b) / 3;
            c >>= 1;
            printf("%c", temp2char[15 - c]);
        }
        printf("\n");
    }
}

static void print_rgb888_img(uint8_t *img, int width, int height)
{
    uint8_t *p = (uint8_t *)img;
    const char temp2char[17] = "@MNHQ&#UJ*x7^i;.";
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            uint8_t *c = p + 3 * (j * width + i);
            uint8_t r = *c++;
            uint8_t g = *c++;
            uint8_t b = *c;
            uint32_t v = (r + g + b) / 3;
            v >>= 4;
            printf("%c", temp2char[15 - v]);
        }
        printf("\n");
    }
}

static void tjpgd_decode_rgb565(uint8_t *mjpegbuffer, uint32_t size, uint8_t *outbuffer)
{
    jpg2rgb565(mjpegbuffer, size, outbuffer, JPG_SCALE_NONE);
}

static void tjpgd_decode_rgb888(uint8_t *mjpegbuffer, uint32_t size, uint8_t *outbuffer)
{
    fmt2rgb888(mjpegbuffer, size, PIXFORMAT_JPEG, outbuffer);
}

typedef enum {
    DECODE_RGB565,
    DECODE_RGB888,
} decode_type_t;

static const decode_func_t g_decode_func[2][2] = {
    {tjpgd_decode_rgb565,},
    {tjpgd_decode_rgb888,},
};

// out_buf 输出指针
// out_length 长度
static int jpg_decode(uint8_t decoder_index, decode_type_t type, uint8_t *jpg, uint32_t length, uint32_t img_w, uint32_t img_h,
                    uint8_t **out_buf,uint32_t* out_length)
{
    // uint8_t *jpg_buf = malloc(length);
    // if (NULL == jpg_buf) {
    //     ESP_LOGE(TAG, "malloc for jpg buffer failed");
    //     return 0;
    // }
    // memcpy(jpg_buf, jpg, length);

    uint8_t *rgb_buf = heap_caps_malloc(img_w * img_h * 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (NULL == rgb_buf) {
        // free(jpg_buf);
        ESP_LOGE(TAG, "malloc for rgb buffer failed");
        return -1;
    }
    decode_func_t decode = g_decode_func[type][decoder_index];

#if 0
    decode(jpg_buf, length, rgb_buf);
    if (DECODE_RGB565 == type) {
        ESP_LOGI(TAG, "jpeg decode to rgb565");
        print_rgb565_img(rgb_buf, img_w, img_h);
    } else {
        ESP_LOGI(TAG, "jpeg decode to rgb888");
        print_rgb888_img(rgb_buf, img_w, img_h);
    }

    uint32_t times = 16;
    uint64_t t_decode[times];
    for (size_t i = 0; i < times; i++) {
        uint64_t t1 = esp_timer_get_time();
        decode(jpg, length, rgb_buf);
        t_decode[i] = esp_timer_get_time() - t1;
    }

    printf("resolution  ,  t \n");
    uint64_t t_total = 0;
    for (size_t i = 0; i < times; i++) {
        t_total += t_decode[i];
        float t = t_decode[i] / 1000.0f;
        printf("%4ld x %4ld ,  %5.2f ms \n", img_w, img_h, t);
    }

    float fps = times / (t_total / 1000000.0f);
    printf("Decode FPS Result\n");
    printf("resolution  , fps \n");
    printf("%4ld x %4ld , %5.2f  \n", img_w, img_h, fps);
#endif

    decode(jpg, length, rgb_buf);
    *out_buf = rgb_buf;
    *out_length = img_w * img_h * 3;
    // free(jpg_buf);
    // heap_caps_free(rgb_buf);
    return 0;
}

// TEST_CASE("Conversions image 227x149 jpeg decode test")  0, 0
// TEST_CASE("Conversions image 320x240 jpeg decode test")  1, 0
// TEST_CASE("Conversions image 480x320 jpeg decode test")  2, 0
int img_jpeg_decode(uint16_t pic_index, uint16_t lib_index, const uint8_t *buf, uint32_t length,
                    uint8_t **out_buf,uint32_t* out_length)
{
    struct img_t {
        uint16_t w, h;
    };
    struct img_t imgs[] = {
        {.w = 227,.h = 149,},
        {.w = 320,.h = 240,},
        {.w = 480,.h = 320,},
        {.w = 240,.h = 240,},
    };

    ESP_LOGI(TAG, "pic_index:%d", pic_index);
    ESP_LOGI(TAG, "lib_index:%d", lib_index);
    return jpg_decode(lib_index, DECODE_RGB565, buf, length, imgs[pic_index].w, imgs[pic_index].h,out_buf,out_length);
}
