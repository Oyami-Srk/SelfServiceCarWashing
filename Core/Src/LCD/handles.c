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
#include "cmsis_os2.h"
#include "dma2d.h"
#include "ltdc.h"
#include "lvgl.h"
#include "main.h"
#include "tim.h"

volatile uint8_t g_gpu_state = 0;
lv_disp_drv_t    g_disp_drv;

#define LVGL_DRAW_BUFFER_SIZE LCD_WIDTH * 200

static lv_color_t *disp_buf1 = LCD_END_ADDR;
static lv_color_t *disp_buf2 =
    LCD_END_ADDR + LVGL_DRAW_BUFFER_SIZE * sizeof(lv_color_t);

void DMA2D_CB(DMA2D_HandleTypeDef *p_hdma2d) {
    if (g_gpu_state == 1) {
        g_gpu_state = 0;
        lv_disp_flush_ready(&g_disp_drv);
    }
}

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

void LCD_DMA2D_INIT(void) {
    LCD_CLEAR();
    // test lv init
    lv_init();
    disp_init();
    // start timer
    HAL_TIM_Base_Start_IT(&htim2);
}

void LCD_GPIO_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

void LCD_DISPLAY_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

static int32_t           x1_flush;
static int32_t           y1_flush;
static int32_t           x2_flush;
static int32_t           y2_fill;
static int32_t           y_fill_act;
static const lv_color_t *buf_to_flush;

// LVGL
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                       lv_color_t *color_p) {
    uint32_t h = area->y2 - area->y1;
    uint32_t w = area->x2 - area->x1;

    uint32_t OffLineSrc = LCD_WIDTH - (area->x2 - area->x1 + 1);
    uint32_t addr       = LCD_LAYER0_MEM_ADDR +
                    LCD_LAYER0_PIXEL_BYTES * (LCD_WIDTH * area->y1 + area->x1);
    DMA2D->CR      = 0x00000000UL | (1 << 9);
    DMA2D->FGMAR   = (uint32_t)(uint16_t *)(color_p);
    DMA2D->OMAR    = (uint32_t)addr;
    DMA2D->FGOR    = 0;
    DMA2D->OOR     = OffLineSrc;
    DMA2D->FGPFCCR = DMA2D_OUTPUT_RGB565;
    DMA2D->OPFCCR  = DMA2D_OUTPUT_RGB565;
    DMA2D->NLR = (area->y2 - area->y1 + 1) | ((area->x2 - area->x1 + 1) << 16);
    DMA2D->CR |= DMA2D_IT_TC | DMA2D_IT_TE | DMA2D_IT_CE;
    DMA2D->CR |= DMA2D_CR_START;
    g_gpu_state = 1;
}

static volatile uint32_t t_saved = 0;
static monitor_cb(lv_disp_drv_t *d, uint32_t t, uint32_t p) { t_saved = t; }

void disp_init(void) {
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf, disp_buf1, disp_buf2, LCD_WIDTH * 50);
    lv_disp_drv_init(&g_disp_drv);
    g_disp_drv.draw_buf   = &buf;
    g_disp_drv.flush_cb   = disp_flush;
    g_disp_drv.monitor_cb = monitor_cb;
    g_disp_drv.hor_res    = LCD_WIDTH;
    g_disp_drv.ver_res    = LCD_HEIGHT;
    lv_disp_drv_register(&g_disp_drv);
}

// tick hook for lvgl
void LCD_MS_TICK() { lv_tick_inc(1); }

_Noreturn static void task_lvgl() {
    for (;;) {
        lv_task_handler();
        HAL_Delay(5);
    }
}

static const osThreadAttr_t task_lvgl_attributes = {
    .name       = "lcd-lvgl",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityNormal,
};

osThreadId_t pid_task_lvgl = 0;

// init for FreeRTOS
void LCD_INIT_FREERTOS() {
    pid_task_lvgl = osThreadNew(task_lvgl, NULL, &task_lvgl_attributes);
}