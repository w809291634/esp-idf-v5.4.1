/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cmd_system.h"
#include "sdkconfig.h"
#include "board.h"

#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "cmd_system_common";

static void register_free(void);
static void register_heap(void);
static void register_meminfo(void);
static void register_version(void);
static void register_restart(void);
#if WITH_TASKS_INFO
static void register_tasks(void);
#endif
static void register_log_level(void);

void register_system_common(void)
{
    register_free();
    register_heap();
    register_meminfo();
    register_version();
    register_restart();
#if WITH_TASKS_INFO
    register_tasks();
#endif
    register_log_level();
}


/* 'version' command */
static int get_version(int argc, char **argv)
{
    const char *model;
    esp_chip_info_t info;
    uint32_t flash_size;
    esp_chip_info(&info);

    switch(info.model) {
        case CHIP_ESP32:
            model = "ESP32";
            break;
        case CHIP_ESP32S2:
            model = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            model = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            model = "ESP32-C3";
            break;
        case CHIP_ESP32H2:
            model = "ESP32-H2";
            break;
        case CHIP_ESP32C2:
            model = "ESP32-C2";
            break;
        case CHIP_ESP32P4:
            model = "ESP32-P4";
            break;
        case CHIP_ESP32C5:
            model = "ESP32-C5";
            break;
        default:
            model = "Unknown";
            break;
    }

    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return 1;
    }
    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", model);
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%"PRIu32"%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           flash_size / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
    return 0;
}

static void register_version(void)
{
    const esp_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "reboot",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/** 'free' command prints available heap memory */

static int free_mem(int argc, char **argv)
{
    printf("%"PRIu32"\n", esp_get_free_heap_size());
    return 0;
}

static void register_free(void)
{
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/* 'heap' command prints minimum heap size */
static int heap_size(int argc, char **argv)
{
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    printf("min heap size: %"PRIu32"\n", heap_size);
    return 0;
}

static void register_heap(void)
{
    const esp_console_cmd_t heap_cmd = {
        .command = "heap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &heap_size,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&heap_cmd) );

}

/** 'meminfo' command prints free memory in SRAM and PSRAM */

static int mem_info(int argc, char **argv)
{
    // 获取内部 SRAM 的剩余内存大小
    uint32_t sram_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    
    // 初始化 PSRAM 剩余内存大小为 0
    uint32_t psram_free = 0;

#if CONFIG_SPIRAM
    // 如果启用了 PSRAM，则获取其剩余内存大小
    psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
#endif

    // 计算总剩余内存
    uint32_t total_free = sram_free + psram_free;

    // 打印结果
    printf("Memory Usage Summary:\n");
    printf("  Internal SRAM Free:    %8lu B (%8lu KB)\n", sram_free, sram_free / 1024);

#if CONFIG_SPIRAM
    printf("  External PSRAM Free:   %8lu B (%8lu KB)\n", psram_free, psram_free / 1024);
#else
    printf("  External PSRAM Free:   Not Available (PSRAM not enabled)\n");
#endif

    printf("  Total Free:            %8lu B (%8lu KB / %.2f MB)\n",
           total_free, total_free / 1024, total_free / (1024.0 * 1024.0));

    return 0;
}

static void register_meminfo(void)
{
    const esp_console_cmd_t cmd = {
        .command = "mem",
        .help = "Show free memory in SRAM and PSRAM",
        .hint = NULL,
        .func = &mem_info,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/** 'tasks' command prints the list of tasks and related information */
#if WITH_TASKS_INFO

// static int tasks_info(int argc, char **argv)
// {
//     const size_t bytes_per_task = 40; /* see vTaskList description */
//     char *task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
//     if (task_list_buffer == NULL) {
//         ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
//         return 1;
//     }
//     fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
// #ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
//     fputs("\tAffinity", stdout);
// #endif
//     fputs("\n", stdout);
//     vTaskList(task_list_buffer);
//     fputs(task_list_buffer, stdout);
//     free(task_list_buffer);
//     return 0;
// }

static int tasks_info(int argc, char **argv)
{
    // 获取当前系统中的任务数量
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();

    // 创建一个数组来保存所有任务的状态信息
    TaskStatus_t *pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray != NULL)
    {
        configRUN_TIME_COUNTER_TYPE uxTotalRunTime;

        // 使用 uxTaskGetSystemState 填充 pxTaskStatusArray 数组
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &uxTotalRunTime);
        
#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        // 打印表头（包含 Core 和 Cpu Usage）
        printf(" Task Name       | Pri | State   | Core | Water Mask | Stack | Usage | Cpu Usage\r\n");
        printf("---------------------------------------------------------------------------\r\n");
#else
        // 打印表头（只包含 Core）
        printf(" Task Name       | Pri | State   | Core | Water Mask | Stack | Usage\r\n");
        printf("---------------------------------------------------------------\r\n");
#endif
        // 遍历所有任务并打印其状态和栈使用情况
        for (UBaseType_t x = 0; x < uxArraySize; x++)
        {
            const char* stateStr;
            switch (pxTaskStatusArray[x].eCurrentState)
            {
                case eRunning:    stateStr = "Running";    break;
                case eReady:      stateStr = "Ready";      break;
                case eBlocked:    stateStr = "Blocked";    break;
                case eSuspended:  stateStr = "Suspend";    break;
                case eDeleted:    stateStr = "Deleted";    break;
                default:          stateStr = "Unknown";    break;
            }

            // 获取任务句柄
            TaskHandle_t xTask = pxTaskStatusArray[x].xHandle;

            // 获取栈总大小
            size_t stackTotal = vTaskGetStackSize(xTask);

            // 计算栈使用百分比
            float stackUsedPercent = 0.0f;
            if (stackTotal > pxTaskStatusArray[x].usStackHighWaterMark)
            {
                stackUsedPercent = (100.0f - ((float)pxTaskStatusArray[x].usStackHighWaterMark / (float)stackTotal) * 100.0f);
            }
            
            // 获取该任务最后运行的核心
            BaseType_t lastRunCore = xTaskGetCoreID(xTask);
            const char* coreStr = (lastRunCore == 0 || lastRunCore == 1) ? 
                                  (lastRunCore == 0 ? "CPU0" : "CPU1") : "?";
#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
            // 打印表格
            printf("%-16s| %-3d | %-7s | %-4s | %9lu | %5lu | %5.1f%% | %5.1f%%\r\n",
                   pxTaskStatusArray[x].pcTaskName,
                   (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
                   stateStr,
                   coreStr,
                   (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark,
                   (unsigned long)stackTotal,
                   stackUsedPercent,
                   vTaskGetCpuUsagePercent(xTask));
#else
            // 打印表格
            printf("%-16s| %-3d | %-7s | %-4s | %9lu | %5lu | %5.1f%%\r\n",
                   pxTaskStatusArray[x].pcTaskName,
                   (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
                   stateStr,
                   coreStr,
                   (unsigned long)pxTaskStatusArray[x].usStackHighWaterMark,
                   (unsigned long)stackTotal,
                   stackUsedPercent);
#endif
        }

        // 释放分配的内存
        vPortFree(pxTaskStatusArray);
    }
    else
    {
        printf("Failed to allocate memory for task status array.\r\n");
    }
    return 0;
}

static void register_tasks(void)
{
    const esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
    const esp_console_cmd_t cmd1 = {
        .command = "ps",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd1) );
}

#endif // WITH_TASKS_INFO

/** log_level command changes log level via esp_log_level_set */

static struct {
    struct arg_str *tag;
    struct arg_str *level;
    struct arg_end *end;
} log_level_args;

static const char* s_log_level_names[] = {
    "none",
    "error",
    "warn",
    "info",
    "debug",
    "verbose"
};

static int log_level(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &log_level_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, log_level_args.end, argv[0]);
        return 1;
    }
    assert(log_level_args.tag->count == 1);
    assert(log_level_args.level->count == 1);
    const char* tag = log_level_args.tag->sval[0];
    const char* level_str = log_level_args.level->sval[0];
    esp_log_level_t level;
    size_t level_len = strlen(level_str);
    for (level = ESP_LOG_NONE; level <= ESP_LOG_VERBOSE; level++) {
        if (memcmp(level_str, s_log_level_names[level], level_len) == 0) {
            break;
        }
    }
    if (level > ESP_LOG_VERBOSE) {
        printf("Invalid log level '%s', choose from none|error|warn|info|debug|verbose\n", level_str);
        return 1;
    }
    if (level > CONFIG_LOG_MAXIMUM_LEVEL) {
        printf("Can't set log level to %s, max level limited in menuconfig to %s. "
               "Please increase CONFIG_LOG_MAXIMUM_LEVEL in menuconfig.\n",
               s_log_level_names[level], s_log_level_names[CONFIG_LOG_MAXIMUM_LEVEL]);
        return 1;
    }
    esp_log_level_set(tag, level);
    return 0;
}

static void register_log_level(void)
{
    log_level_args.tag = arg_str1(NULL, NULL, "<tag|*>", "Log tag to set the level for, or * to set for all tags");
    log_level_args.level = arg_str1(NULL, NULL, "<none|error|warn|debug|verbose>", "Log level to set. Abbreviated words are accepted.");
    log_level_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "log_level",
        .help = "Set log level for all tags or a specific tag.",
        .hint = NULL,
        .func = &log_level,
        .argtable = &log_level_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
