#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_http_client.h"

#define TAG "HTTP_STREAM_CLIENT"
#define MAX_HTTP_RECV_BUFFER 4096

// 替换为你实际的ESP32 IP地址和路径
#define STREAM_URL "http://192.168.4.1/stream"

// 边界字符串（必须与服务器一致）
#define BOUNDARY "--123456789000000000000987654321"

static int jpg_buf_len = 0;
static uint8_t *jpg_buf = NULL;

// 查找子字符串位置（忽略大小写）
char* strcasechr(const char* str, int c) {
    while (*str) {
        if (tolower(*str) == tolower(c)) return (char*)str;
        str++;
    }
    return NULL;
}

// 解析 multipart 数据流
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *buf = NULL;
    static int buf_pos = 0;
    static int content_start = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "Received data of %d bytes", evt->data_len);

            if (!buf) {
                buf = malloc(MAX_HTTP_RECV_BUFFER);
                if (!buf) {
                    ESP_LOGE(TAG, "Failed to allocate buffer");
                    return ESP_FAIL;
                }
            }

            memcpy(buf + buf_pos, evt->data, evt->data_len);
            buf_pos += evt->data_len;
            buf[buf_pos] = '\0';

            // 寻找图像边界
            char *start = strstr(buf, BOUNDARY);
            if (!start) break;

            start += strlen(BOUNDARY);
            char *image_start = strstr(start, "\r\n\r\n");
            if (!image_start) break;

            image_start += 4;

            char *end = strstr(image_start, BOUNDARY);
            if (!end) break;

            int img_len = end - image_start;

            if (img_len > 0 && img_len < MAX_HTTP_RECV_BUFFER) {
                if (!jpg_buf) {
                    jpg_buf = malloc(img_len);
                } else {
                    jpg_buf = realloc(jpg_buf, img_len);
                }

                if (jpg_buf) {
                    memcpy(jpg_buf, image_start, img_len);
                    jpg_buf_len = img_len;
                    ESP_LOGI(TAG, "JPEG frame received, size: %d bytes", jpg_buf_len);

                    // TODO: 处理图片，比如保存到文件或发送给显示屏
                    FILE *fp = fopen("/sdcard/frame.jpg", "wb");
                    if (fp) {
                        fwrite(jpg_buf, 1, jpg_buf_len, fp);
                        fclose(fp);
                        ESP_LOGI(TAG, "Saved frame.jpg");
                    }
                }
            }

            // 移除已处理的部分
            int remaining = buf + buf_pos - end;
            memmove(buf, end, remaining);
            buf_pos = remaining;
            buf[buf_pos] = '\0';

            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected, clean up");
            if (buf) free(buf);
            buf = NULL;
            buf_pos = 0;
            if (jpg_buf) free(jpg_buf);
            jpg_buf = NULL;
            jpg_buf_len = 0;
            break;

        default:
            break;
    }

    return ESP_OK;
}

void http_stream_task(void *pvParameters) {
    esp_http_client_config_t config = {
        .url = STREAM_URL,
        .event_handler = http_event_handler,
        .buffer_size = MAX_HTTP_RECV_BUFFER,
        .user_agent = "ESP32-MJPEG-Client",
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;

    ESP_LOGI(TAG, "Connecting to stream URL: %s", STREAM_URL);

    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        goto error;
    }

    int status_code = esp_http_client_get_status_code(client);
    int content_length = esp_http_client_get_content_length(client);

    ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d", status_code, content_length);

    char *buffer = malloc(MAX_HTTP_RECV_BUFFER);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate read buffer");
        goto error;
    }

    int read_len;
    while ((read_len = esp_http_client_read(client, buffer, MAX_HTTP_RECV_BUFFER)) > 0) {
        ESP_LOGD(TAG, "Read %d bytes from stream", read_len);
    }

    free(buffer);

error:
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    vTaskDelete(NULL); // 删除当前任务
}

esp_err_t start_stream_client(void)
{
    xTaskCreate(&http_stream_task, "http_stream_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}