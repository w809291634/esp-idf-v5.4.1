/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include "uvc_frame_config.h"
#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

#ifdef CONFIG_TINYUSB_RHPORT_HS
#   define CFG_TUSB_RHPORT1_MODE    OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED
#else
#   define CFG_TUSB_RHPORT0_MODE    OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef ESP_PLATFORM
#define ESP_PLATFORM 1
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS           OPT_OS_FREERTOS
#endif

// Espressif IDF requires "freertos/" prefix in include path
#if TU_CHECK_MCU(OPT_MCU_ESP32S2, OPT_MCU_ESP32S3, OPT_MCU_ESP32P4)
#define CFG_TUSB_OS_INC_PATH    freertos/
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG        0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED       1

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN        __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//------------- CLASS -------------//

// The number of video control interfaces
// The number of video streaming interfaces
#if CONFIG_UVC_SUPPORT_TWO_CAM
#define CFG_TUD_VIDEO  2
#define CFG_TUD_VIDEO_STREAMING  2
#else
#define CFG_TUD_VIDEO  1
#define CFG_TUD_VIDEO_STREAMING  1
#endif

// video streaming endpoint size
#ifdef UVC_CAM1_BULK_MODE
#if CONFIG_TINYUSB_RHPORT_HS
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  512
#else
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  64
#endif
#define CFG_TUD_CAM1_VIDEO_STREAMING_BULK 1
#else
#define CFG_TUD_CAM1_VIDEO_STREAMING_BULK 0
#if CONFIG_TINYUSB_RHPORT_HS
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  1023
#else
#define CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE  512
#endif
#endif

#define CFG_EXAMPLE_VIDEO_DISABLE_MJPEG (!FORMAT_MJPEG)

#if CONFIG_UVC_SUPPORT_TWO_CAM
#ifdef UVC_CAM2_BULK_MODE
#if CONFIG_TINYUSB_RHPORT_HS
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  512
#else
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  64
#endif
#define CFG_TUD_CAM2_VIDEO_STREAMING_BULK 1
#else
#define CFG_TUD_CAM2_VIDEO_STREAMING_BULK 0
#if CONFIG_TINYUSB_RHPORT_HS
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  1023
#else
#define CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE  512
#endif
#endif
#endif

#if CONFIG_UVC_SUPPORT_TWO_CAM
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE (CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE > CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE?CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE:CFG_TUD_CAM2_VIDEO_STREAMING_EP_BUFSIZE)
#else
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE CFG_TUD_CAM1_VIDEO_STREAMING_EP_BUFSIZE
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
