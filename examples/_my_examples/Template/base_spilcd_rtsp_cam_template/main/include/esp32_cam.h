#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_http_server.h"
#include "esp_timer.h"

void esp_cam_stream_init(void);
void app_wifi_main();
/* declarations go here */

#ifdef __cplusplus
}
#endif
