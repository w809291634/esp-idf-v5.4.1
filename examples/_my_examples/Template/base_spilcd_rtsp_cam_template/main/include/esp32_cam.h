#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_http_server.h"
#include "esp_timer.h"

esp_err_t camera_init(void);
esp_err_t init_camera(void);
esp_err_t camera_capture(void);
esp_err_t jpg_httpd_handler(httpd_req_t *req);
void esp_http_server_init(void);
void esp_strem_cam_init(void);
/* declarations go here */

#ifdef __cplusplus
}
#endif
