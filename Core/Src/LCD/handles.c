/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * This handles also provided lvgl init
 */

#include "LCD/handles.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "dma2d.h"
#include "ltdc.h"
#include "lvgl.h"
#include "main.h"
#include "task.h"

extern volatile uint8_t g_gpu_state;
extern lv_disp_drv_t    g_disp_drv;

void LCD_CLEAR(void) {
    hdma2d.Init.Mode = DMA2D_R2M;
    HAL_DMA2D_Init(&hdma2d);

    hdma2d.LayerCfg[0].AlphaMode      = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[0].InputAlpha     = 0xFF;
    hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[0].InputOffset    = 0;

    if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_DMA2D_Start(&hdma2d, 0xFF000000, LCD_LAYER0_MEM_ADDR, LCD_WIDTH,
                        LCD_HEIGHT) == HAL_OK) {
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);
    }
}

void    disp_init(void);  // in lvgl.c
uint8_t Touch_Init(void); // in touchpad.c

void LCD_DMA2D_INIT(void) {
    LCD_CLEAR();
    Touch_Init();
    // lvgl process
    lv_init();
    disp_init();
}

void LCD_GPIO_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

void LCD_DISPLAY_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

// tick hook for lvgl
// void LCD_MS_TICK() { lv_tick_inc(1); }

_Noreturn static void task_lvgl() {
    TickType_t       xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(5);

    // 使用当前时间初始化变量xLastWakeTime ,注意这和vTaskDelay()函数不同
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        /* 调用系统延时函数,周期性阻塞5ms */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        lv_task_handler();
    }
    vTaskDelete(NULL);
}

static const osThreadAttr_t task_lvgl_attributes = {
    .name       = "lcd-lvgl",
    .stack_size = 2048,
    .priority   = (osPriority_t)osPriorityAboveNormal,
};

osThreadId_t pid_task_lvgl = 0;

// init for FreeRTOS
void LCD_INIT_FREERTOS() {
    pid_task_lvgl = osThreadNew(task_lvgl, NULL, &task_lvgl_attributes);
}