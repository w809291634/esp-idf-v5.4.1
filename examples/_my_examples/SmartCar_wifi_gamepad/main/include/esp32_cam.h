#pragma once
#include "board.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_timer.h"

void esp_cam_client_stream_init(void);
esp_err_t start_stream_client(void);
void app_wifi_main();
int img_jpeg_decode(uint16_t pic_index, uint16_t lib_index, const uint8_t *buf, uint32_t length,
                    uint8_t **out_buf,uint32_t* out_length);
/* declarations go here */

#ifdef __cplusplus
}
#endif
