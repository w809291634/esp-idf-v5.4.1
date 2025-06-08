/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "board_config.h"

void hw_board_init(void);
uint32_t vTaskGetStackSize(TaskHandle_t TaskHandle);
void vTaskResetRunTimeCounter(TaskHandle_t TaskHandle);
void vTaskSetCpuUsagePercent(TaskHandle_t TaskHandle,float CpuUsagePercent);
float vTaskGetCpuUsagePercent(TaskHandle_t TaskHandle);

#ifdef __cplusplus
}
#endif


