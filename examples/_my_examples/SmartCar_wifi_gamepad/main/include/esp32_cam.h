#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include <sys/param.h>
#include "esp_camera.h"

void app_wifi_main();
void esp_cam_client_stream_init(void);
esp_err_t start_stream_client(void);
int img_jpeg_decode(uint16_t pic_index, uint16_t lib_index, const uint8_t *buf, uint32_t length,
                    uint8_t **out_buf,uint32_t* out_length);
camera_fb_t* stream_client_fb_get(void);
void stream_client_fb_return(camera_fb_t *fb);
/* declarations go here */

#ifdef __cplusplus
}
#endif
