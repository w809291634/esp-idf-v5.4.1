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
int jpg_decode2rgb565(uint8_t *jpg, uint32_t length, 
                    uint32_t img_w, uint32_t img_h,
                    uint8_t **out_buf,uint32_t* out_length);

camera_fb_t* stream_client_fb_get(void);
void stream_client_fb_return(camera_fb_t *fb);

#ifdef __cplusplus
}
#endif
