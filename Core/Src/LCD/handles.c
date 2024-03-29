/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "FreeRTOS.h"
#include "dma2d.h"
#include "cmsis_os.h"
#include "ltdc.h"
#include "lvgl.h"
#include "main.h"
#include "task.h"
#include "Common/utils.h"

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

static void lv_logger(const char *buf) { LOGF("%s", buf); }

void LCD_INIT_DMA2D(void) {
    LCD_CLEAR();
    // lvgl process
    lv_init();
    // register log
    lv_log_register_print_cb(lv_logger);
    LV_LOG_WARN("Test logger for lvgl.");
    disp_init();
}

void LCD_INIT_GPIO() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
    Touch_Init();
}

void LCD_INIT_DISP() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

// tick hook for lvgl
// void LCD_MS_TICK() { lv_tick_inc(1); }

_Noreturn static void task_lvgl() {
    TickType_t       xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        lv_task_handler();
    }
    vTaskDelete(NULL);
}

// init for FreeRTOS
void LCD_INIT_RTOS() {
    xTaskCreate(task_lvgl, "LCD-LVGL", 1024, NULL, tskIDLE_PRIORITY, NULL);
}