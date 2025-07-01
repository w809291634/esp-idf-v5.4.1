#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_http_client.h"
#include <sys/param.h>

#define DBG_TAG           "stream_c"
//#define DBG_LVL           DBG_INFO
#define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

#define TAG "HTTP_STREAM_CLIENT"
#define MAX_HTTP_RECV_BUFFER 4096

// 替换为你实际的ESP32 IP地址和路径
#define STREAM_URL "http://192.168.4.1/stream"
#define PART_BOUNDARY "123456789000000000000987654321"
#define BOUNDARY_STR "\r\n--" PART_BOUNDARY "\r\n"
#define BOUNDARY_STR_LEN (strlen(BOUNDARY_STR))
#define STREAM_HEADER_PATTERN "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n"
#define MAX_HEADER_LEN 128

// 客户端状态上下文
typedef struct {
    FILE *jpg_file;
    int frame_count;
    int content_length;
    int bytes_received;
    int timestamp_sec;
    int timestamp_usec;
    bool in_frame;
    bool in_header;
    char header_buf[MAX_HEADER_LEN];
    int header_len;
} stream_ctx_t;

// 解析HTTP头信息
static void parse_header(stream_ctx_t *ctx) {
    char *ptr = ctx->header_buf;
    ctx->content_length = -1;
    ctx->timestamp_sec = 0;
    ctx->timestamp_usec = 0;

    // 查找内容长度
    char *cl_start = strstr(ptr, "Content-Length: ");
    if (cl_start) {
        cl_start += strlen("Content-Length: ");
        ctx->content_length = atoi(cl_start);
    }

    // 查找时间戳
    char *ts_start = strstr(ptr, "X-Timestamp: ");
    if (ts_start) {
        ts_start += strlen("X-Timestamp: ");
        sscanf(ts_start, "%d.%d", &ctx->timestamp_sec, &ctx->timestamp_usec);
    }
}

// HTTP事件处理器
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    stream_ctx_t *ctx = (stream_ctx_t *)evt->user_data;
    
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA: {
            // 获取分块长度（如果使用分块传输）
            int chunk_len;
            esp_http_client_get_chunk_length(evt->client,&chunk_len);
            ESP_LOGI(TAG, "Chunk length: %d, Data len: %d", chunk_len, evt->data_len);
            
            // 获取内容长度（对于非分块传输）
            int content_len = esp_http_client_get_content_length(evt->client);
            ESP_LOGI(TAG, "Content length: %d", content_len);
            
            const char *data = evt->data;
            size_t data_len = evt->data_len;
            size_t data_processed = 0;
            
            while (data_processed < data_len) {
                if (!ctx->in_frame) {
                    // 检查边界标记
                    if (data_len - data_processed >= BOUNDARY_STR_LEN &&
                        memcmp(data + data_processed, BOUNDARY_STR, BOUNDARY_STR_LEN) == 0) {
                        ESP_LOGI(TAG, "Found boundary marker");
                        data_processed += BOUNDARY_STR_LEN;
                        ctx->in_header = true;
                        ctx->header_len = 0;
                        memset(ctx->header_buf, 0, sizeof(ctx->header_buf));
                        continue;
                    } else {
                        // 跳过非边界数据
                        data_processed = data_len;
                        break;
                    }
                }
                
                if (ctx->in_header) {
                    // 收集头部信息
                    size_t to_copy = MIN(data_len - data_processed, sizeof(ctx->header_buf) - ctx->header_len - 1);
                    if (to_copy > 0) {
                        memcpy(ctx->header_buf + ctx->header_len, data + data_processed, to_copy);
                        ctx->header_len += to_copy;
                        data_processed += to_copy;
                        ctx->header_buf[ctx->header_len] = '\0';
                    }
                    
                    // 检查头部结束标记
                    char *header_end = strstr(ctx->header_buf, "\r\n\r\n");
                    if (header_end) {
                        *header_end = '\0'; // 终止字符串
                        parse_header(ctx);
                        
                        ESP_LOGI(TAG, "Frame header: %s", ctx->header_buf);
                        ESP_LOGI(TAG, "Content-Length: %d, Timestamp: %d.%06d", 
                                ctx->content_length, ctx->timestamp_sec, ctx->timestamp_usec);
                        
                        // 创建文件保存JPEG
                        char filename[64];
                        snprintf(filename, sizeof(filename), "/sdcard/frame_%03d_%d.%06d.jpg", 
                                 ctx->frame_count, ctx->timestamp_sec, ctx->timestamp_usec);
                        ctx->jpg_file = fopen(filename, "wb");
                        if (!ctx->jpg_file) {
                            ESP_LOGE(TAG, "Failed to open file: %s", filename);
                            return ESP_FAIL;
                        }
                        
                        ctx->in_header = false;
                        ctx->in_frame = true;
                        ctx->bytes_received = 0;
                        ctx->frame_count++;
                    } else if (ctx->header_len >= sizeof(ctx->header_buf) - 1) {
                        ESP_LOGE(TAG, "Header too long or malformed");
                        return ESP_FAIL;
                    }
                } else if (ctx->in_frame) {
                    // 处理图像数据
                    size_t to_write = MIN(data_len - data_processed, ctx->content_length - ctx->bytes_received);
                    if (to_write > 0) {
                        if (fwrite(data + data_processed, 1, to_write, ctx->jpg_file) != to_write) {
                            ESP_LOGE(TAG, "File write error");
                            fclose(ctx->jpg_file);
                            ctx->jpg_file = NULL;
                            return ESP_FAIL;
                        }
                        data_processed += to_write;
                        ctx->bytes_received += to_write;
                    }
                    
                    // 检查帧是否完整
                    if (ctx->bytes_received >= ctx->content_length) {
                        fclose(ctx->jpg_file);
                        ctx->jpg_file = NULL;
                        ctx->in_frame = false;
                        ESP_LOGI(TAG, "Frame %d complete, %d bytes saved", 
                                ctx->frame_count, ctx->bytes_received);
                    }
                }
            }
            break;
        }
        
        case HTTP_EVENT_DISCONNECTED: {
            ESP_LOGI(TAG, "Disconnected from server");
            if (ctx->jpg_file) {
                fclose(ctx->jpg_file);
                ctx->jpg_file = NULL;
            }
            break;
        }
        
        case HTTP_EVENT_ERROR: {
            ESP_LOGE(TAG, "HTTP client error");
            if (ctx->jpg_file) {
                fclose(ctx->jpg_file);
                ctx->jpg_file = NULL;
            }
            break;
        }
        
        default:
            break;
    }
    return ESP_OK;
}

void http_stream_task(void *pvParameters) {
    stream_ctx_t ctx = {
        .jpg_file = NULL,
        .frame_count = 0,
        .content_length = -1,
        .bytes_received = 0,
        .in_frame = false,
        .in_header = false,
        .header_len = 0
    };

    esp_http_client_config_t config = {
        .url = STREAM_URL,
        .event_handler = http_event_handler,
        .user_data = &ctx,
        .buffer_size = 4096,  // 使用较大缓冲区提高效率
        .buffer_size_tx = 1024,
        .timeout_ms = 30000,  // 30秒超时
        .disable_auto_redirect = false,
        .max_redirection_count = 3,
        .keep_alive_enable = true,  // 启用长连接
        .skip_cert_common_name_check = true  // 开发环境中跳过证书检查
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP Status = %d", status_code);
    }
    
    esp_http_client_cleanup(client);
}

esp_err_t start_stream_client(void)
{
    xTaskCreate(&http_stream_task, "http_stream_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}