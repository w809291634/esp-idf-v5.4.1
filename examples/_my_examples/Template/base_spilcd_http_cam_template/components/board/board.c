#include <stdio.h>
#include <inttypes.h>
#include "board.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "apl_console.h"
#include "apl_utility.h"

#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
volatile unsigned long CPU_RunTime = 0UL;

// 软件定时器回调函数
void vTimerCallback(TimerHandle_t xTimer)
{
    configRUN_TIME_COUNTER_TYPE _uxTotalRunTime;
    static configRUN_TIME_COUNTER_TYPE last_uxTotalRunTime=0;
    // 获取当前系统中的任务数量
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    
    // 创建一个数组来保存所有任务的状态信息
    TaskStatus_t *pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray != NULL)
    {
        configRUN_TIME_COUNTER_TYPE uxTotalRunTime;
        
        // 使用 uxTaskGetSystemState 填充 pxTaskStatusArray 数组
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &_uxTotalRunTime);
        uxTotalRunTime = _uxTotalRunTime - last_uxTotalRunTime;

//        // 打印表头
//        printf("Task Name            | Run Count | CPU Usage %% %d\r\n",uxTotalRunTime);
//        printf("-------------------------------------------------\r\n");
        // 遍历所有任务并打印其 ulRunTimeCounter
        for (UBaseType_t x = 0; x < uxArraySize; x++)
        {
            TaskHandle_t currentTaskHandle = pxTaskStatusArray[x].xHandle;
            configRUN_TIME_COUNTER_TYPE runTimeCounter = pxTaskStatusArray[x].ulRunTimeCounter;
            
          
            float cpuUsagePercent = ((float)runTimeCounter / (float)uxTotalRunTime) * 100.0f;
            // printf("runTimeCounter:%ld TotalRunTime:%ld Percent:%f\r\n",runTimeCounter,uxTotalRunTime,cpuUsagePercent);
            vTaskSetCpuUsagePercent(currentTaskHandle,cpuUsagePercent);
            vTaskResetRunTimeCounter(currentTaskHandle);
          
//            printf("%-20s| %9lu | %6.1f%%\r\n",
//                   pxTaskStatusArray[x].pcTaskName,
//                   pxTaskStatusArray[x].ulRunTimeCounter,
//                   cpuUsagePercent);
        }
        
        // 释放分配的内存
        vPortFree(pxTaskStatusArray);
    }
    else
    {
        printf("Failed to allocate memory for task status array.\r\n");
    }
    last_uxTotalRunTime = _uxTotalRunTime;
}

// 初始化函数
void setupCpuUsageMonitor(void)
{
    // 创建一个1秒周期的软件定时器
    TimerHandle_t xTimer = xTimerCreate(
        "CpuUsageTimer",           // 名称
        pdMS_TO_TICKS(1000),       // 周期为1秒
        pdTRUE,                    // 自动重载
        (void *)0,                 // 定时器ID
        vTimerCallback             // 回调函数
    );

    BaseType_t ret = xTimerStart(xTimer, 0);
    ESP_ERROR_CHECK(ret == pdPASS ? ESP_OK : ESP_FAIL);
}
#endif

/**
 * This function will initial STM32 board.
 */
void hw_board_init(void)
{
#ifdef CONFIG_APP_ENABLE_CONSOLE
    apl_console_init();
#endif
    /* setup Cpu Usage Monitor */
#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    setupCpuUsageMonitor();
#endif
}