#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include <sys/param.h>
#include "esp32_cam.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
// #define LOG_LOCAL_LEVEL ESP_LOG_INFO
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#define LOG_LOCAL_LEVEL ESP_LOG_WARN
#include "esp_log.h"

#define DBG_TAG           "stream_c"
#define DBG_LVL           DBG_INFO
// #define DBG_LVL           DBG_LOG
//#define DBG_LVL           DBG_NODBG
#include <mydbg.h>          // must after of DBG_LVL, DBG_TAG or other options

#define TAG     DBG_TAG
#define MAX_HTTP_RECV_BUFFER 4096

// 替换为你实际的ESP32 IP地址和路径
#define STREAM_URL "http://192.168.4.1/stream"
#define PART_BOUNDARY "123456789000000000000987654321"
#define BOUNDARY_STR "\r\n--" PART_BOUNDARY "\r\n"
#define BOUNDARY_STR_LEN (strlen(BOUNDARY_STR))
#define STREAM_HEADER_PATTERN "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n"
#define MAX_HEADER_LEN 128

#define MAX_FARME_BUF_NUM       2

/*  定义  */

// 客户端状态上下文
typedef struct {
    bool in_header;         // 正在处理 收集头部信息
    bool in_frame;          // 正在处理帧

    // 头部信息 buf 
    char header_buf[MAX_HEADER_LEN];
    int header_len;
    // 内容长度
    int content_length;
    // 时间戳
    int timestamp_sec;
    int timestamp_usec;

    // 处理接受的图像数据
    int frame_count;
    int bytes_received;
    // 图像队列
    QueueHandle_t xQueueIFrame;
    // 图形缓存区指针 
    // uint8_t curr_buf_num;
    // uint8_t * buf_p[MAX_FARME_BUF_NUM];
} stream_ctx_t;

/*  变量  */
static stream_ctx_t g_ctx = {
    .frame_count = 0,
    .content_length = -1,
    .bytes_received = 0,
    .in_frame = false,
    .in_header = false,
    .header_len = 0,
    // 图形缓存
    .xQueueIFrame = NULL,
    // .curr_buf_num = 0,
    // .buf_p = {NULL,},
};
esp_http_client_handle_t g_steam_client=NULL;
static uint8_t * rgb_buf = NULL;
static camera_fb_t* camera_fb_p = NULL;

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
            ESP_LOGI(TAG, "\r\nChunk len: %d, Data len: %d", chunk_len, evt->data_len);
            
            const char *data = evt->data;
            size_t data_len = evt->data_len;
            size_t data_processed = 0;
            
            /* 调试数据 */
            // log_draw("data:\r\n");
            // for (size_t i = 0; i < evt->data_len; i++) {
            //     log_draw("%c", data[i]); 
            //     if(i>100)break; // 图像数据不打印过多
            // }
            // log_draw("\r\n");

            while (data_processed < data_len) {
                /* 处理数据边界 */
                if (!ctx->in_frame && !ctx->in_header) {
                    ESP_LOGI(TAG, "Start Collect header information");
                    // 检查边界标记
                    if (data_len - data_processed >= BOUNDARY_STR_LEN &&
                        memcmp(data + data_processed, BOUNDARY_STR, BOUNDARY_STR_LEN) == 0) {
                        ESP_LOGI(TAG, "Found boundary marker");
                        data_processed += BOUNDARY_STR_LEN;
                        
                        /* 检查发送队列是否有空余 */
                        UBaseType_t uxBusySlots = uxQueueMessagesWaiting(ctx->xQueueIFrame);
                        if(uxBusySlots >= MAX_FARME_BUF_NUM){
                            ESP_LOGW(TAG, "request Queue is full");
                            continue;
                        }

                        /* 准备接收数据头 */
                        ctx->in_header = true;
                        ctx->header_len = 0;
                        memset(ctx->header_buf, 0, sizeof(ctx->header_buf));
                        continue;
                    } else {
                        ESP_LOGI(TAG, "Skip non-boundary data");
                        // 跳过非边界数据
                        data_processed = data_len;
                        break;
                    }
                }
                
                /* 处理数据头 */
                if (ctx->in_header) {
                    ESP_LOGI(TAG, "Start Collect header information");
                    // 收集头部信息
                    size_t to_copy = MIN(data_len - data_processed, sizeof(ctx->header_buf) - ctx->header_len - 1);
                    if (to_copy > 0) {
                        memcpy(ctx->header_buf + ctx->header_len, data + data_processed, to_copy);
                        ctx->header_len += to_copy;
                        data_processed += to_copy;
                        ctx->header_buf[ctx->header_len] = '\0';
                        log_draw("header_buf:%s\r\n",ctx->header_buf);
                    }
                    
                    // 检查头部结束标记
                    char *header_end = strstr(ctx->header_buf, "\r\n\r\n");
                    if (header_end) {
                        *header_end = '\0'; // 终止字符串
                        parse_header(ctx);  // 解析 头部信息
                        
                        ESP_LOGI(TAG, "Frame header: %s", ctx->header_buf);
                        ESP_LOGI(TAG, "Content-Length: %d, Timestamp: %d.%06d", 
                                ctx->content_length, ctx->timestamp_sec, ctx->timestamp_usec);
                        
                        // 创建 buffer 保存图像数据
                        // ctx->buf_p[ctx->curr_buf_num] = heap_caps_malloc(ctx->content_length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                        // if (NULL == ctx->buf_p[ctx->curr_buf_num]) {
                        //     ESP_LOGE(TAG, "malloc for rgb buffer failed");
                        //     ctx->in_header = false;
                        //     continue;
                        // }
                        // ctx->curr_buf_num++;

                        camera_fb_p = heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                        if (NULL == camera_fb_p) {
                            ESP_LOGE(TAG, "malloc for camera_fb_p failed");
                            ctx->in_header = false;
                            continue;
                        }

                        rgb_buf = heap_caps_malloc(ctx->content_length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
                        if (NULL == rgb_buf) {
                            heap_caps_free(camera_fb_p);
                            ESP_LOGE(TAG, "malloc for rgb buffer failed");
                            ctx->in_header = false;
                            continue;
                        }
                        
                        /* 填充结构体 */
                        camera_fb_p->buf = rgb_buf;
                        camera_fb_p->len = ctx->content_length;
                        camera_fb_p->format = PIXFORMAT_JPEG;
                        camera_fb_p->height = 240;  // 定义好
                        camera_fb_p->width = 240;

                        /* 进入下一步存储图像 */
                        ctx->in_header = false;
                        ctx->in_frame = true;
                        ctx->bytes_received = 0;
                        ctx->frame_count++;
                    } else if (ctx->header_len >= sizeof(ctx->header_buf) - 1) {
                        ESP_LOGE(TAG, "Header too long or malformed");
                        return ESP_FAIL;
                    }
                } 
                /* 处理图像数据 */
                else if (ctx->in_frame) {
                    ESP_LOGI(TAG, "Start Processing image data");
                    size_t to_write = MIN(data_len - data_processed, ctx->content_length - ctx->bytes_received);
                    if (to_write > 0) {
                        // 接收每个图像数据片段
                        data_processed += to_write;
                        configASSERT(rgb_buf != NULL);
                        memcpy(rgb_buf + ctx->bytes_received,data,to_write);
                        ctx->bytes_received += to_write;
                    }
                    
                    // 检查帧是否完整
                    if (ctx->bytes_received >= ctx->content_length) {
                        ctx->in_frame = false;
                        configASSERT(ctx->xQueueIFrame != NULL);
                        configASSERT(camera_fb_p != NULL);
                        if (xQueueSend(ctx->xQueueIFrame, &camera_fb_p, portMAX_DELAY) != pdTRUE) {
                            // 发送失败，释放内存
                            heap_caps_free(rgb_buf);
                            heap_caps_free(camera_fb_p);
                        }
                        ESP_LOGW(TAG, "Frame %d complete, %d bytes saved", 
                                ctx->frame_count, ctx->bytes_received);
                    }
                }
            }
            break;
        }
        
        case HTTP_EVENT_DISCONNECTED: {
            ESP_LOGI(TAG, "Disconnected from server");
            break;
        }
        
        case HTTP_EVENT_ERROR: {
            ESP_LOGE(TAG, "HTTP client error");
            break;
        }
        
        default:
            break;
    }
    return ESP_OK;
}

void http_stream_task(void *pvParameters) {
    esp_http_client_config_t config = {
        .url = STREAM_URL,
        .event_handler = http_event_handler,
        .user_data = &g_ctx,
        .buffer_size = 4096,  // 使用较大缓冲区提高效率
        .buffer_size_tx = 1024,
        .timeout_ms = 30000,  // 30秒超时
        .disable_auto_redirect = false,
        .max_redirection_count = 3,
        .keep_alive_enable = true,  // 启用长连接
        .skip_cert_common_name_check = true  // 开发环境中跳过证书检查
    };

    g_steam_client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(g_steam_client);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    } else {
        int status_code = esp_http_client_get_status_code(g_steam_client);
        ESP_LOGI(TAG, "HTTP Status = %d", status_code);
    }
    
    esp_http_client_cleanup(g_steam_client);
    vTaskDelete(NULL);
}

camera_fb_t* stream_client_fb_get(void)
{
    camera_fb_t* fb=NULL;
    if (xQueueReceive(g_ctx.xQueueIFrame, &fb, portMAX_DELAY)) {
        return fb;
    } 
    return fb;
}

void stream_client_fb_return(camera_fb_t *fb)
{
    if(fb != NULL) {
        if(fb->buf != NULL){
            heap_caps_free(fb->buf);
        }
        heap_caps_free(fb);
    }
}

esp_err_t start_stream_client(void)
{
    // 创建图像的接收队列
    g_ctx.xQueueIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    // 创建任务并绑定到 CPU0
    BaseType_t task_created = xTaskCreatePinnedToCore(&http_stream_task, "http_cli_task", 8192, NULL, 5, NULL, 0);
    // 使用 ESP_ERROR_CHECK 检查是否成功创建任务
    ESP_ERROR_CHECK(task_created == pdPASS ? ESP_OK : ESP_FAIL);
    ESP_LOGI(TAG, "start stream client");
    return ESP_OK;
}

esp_err_t stop_stream_client(void)
{
    if(g_steam_client == NULL) return ESP_FAIL;
    esp_http_client_close(g_steam_client);
    ESP_LOGI(TAG, "stop stream client");
    return ESP_OK;
}

/*************************************************
 * // 测试 http 的断开和链接
        static int __count = 0;
        if(__count > 5 && __count <= 6 ){
            esp_http_client_close(g_steam_client);
            ESP_LOGI(TAG, "client close");
        }else if(__count > 10 && __count <= 11 ){
            // esp_http_client_close(g_steam_client);
            // ESP_LOGI(TAG, "client close");
            TEST_ESP_OK(start_stream_client());
            ESP_LOGI(TAG, "restart client");
            __count = 0;
        }
        __count++ ;
*************************************************/