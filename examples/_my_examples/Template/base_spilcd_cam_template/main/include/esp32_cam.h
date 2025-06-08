#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_http_server.h"
#include "esp_timer.h"

esp_err_t camera_init(void);
esp_err_t jpg_httpd_handler(httpd_req_t *req);
/* declarations go here */

#ifdef __cplusplus
}
#endif
