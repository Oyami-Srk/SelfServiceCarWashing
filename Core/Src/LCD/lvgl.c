/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: lvgl.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

// LVGL Porting to DMA2D

#include "lvgl.h"
#include "dma2d.h"
#include "main.h"
#include "GUI/handles.h"

static int32_t           x1_flush;
static int32_t           y1_flush;
static int32_t           x2_flush;
static int32_t           y2_fill;
static int32_t           y_fill_act;
static const lv_color_t *buf_to_flush;

#if 1
#define LVGL_DRAW_BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT)
static lv_color_t *disp_buf1 = (lv_color_t *)LCD_END_ADDR;
static lv_color_t *disp_buf2 =
    (lv_color_t *)(LCD_END_ADDR + LVGL_DRAW_BUFFER_SIZE * sizeof(lv_color_t));
#else
#define LVGL_DRAW_BUFFER_SIZE (LCD_WIDTH * 15)
lv_color_t disp_buf1[LVGL_DRAW_BUFFER_SIZE];
lv_color_t disp_buf2[LVGL_DRAW_BUFFER_SIZE];
#endif

uint8_t              g_gpu_state = 0;
static lv_disp_drv_t g_disp_drv;

static __IO uint16_t *my_fb = (uint16_t *)LCD_LAYER0_MEM_ADDR;

static void disp_flush(__attribute__((unused)) lv_disp_drv_t *disp_drv,
                       const lv_area_t *area, lv_color_t *color_p) {
    /*
    uint32_t h = area->y2 - area->y1;
    uint32_t w = area->x2 - area->x1;
    */

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

void DMA2D_CB(__attribute__((unused)) DMA2D_HandleTypeDef *p_hdma2d) {
    if (g_gpu_state == 1) {
        g_gpu_state = 0;
        lv_disp_flush_ready(&g_disp_drv);
    }
}

void DMA2D_CB_ERROR(DMA2D_HandleTypeDef *p_hdma2d) {}

/*
static volatile uint32_t t_saved = 0;
 */

static void monitor_cb(__attribute__((unused)) lv_disp_drv_t *d, uint32_t t,
                       __attribute__((unused)) uint32_t p) {
    //    t_saved = t;
    ((void)t);
}

uint8_t touchpad_get_xy(uint16_t *x, uint16_t *y); // in touchpad.c

static bool touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    bool     detected;
    uint16_t x;
    uint16_t y;
    detected = touchpad_get_xy(&x, &y);
    if (detected) {
        data->point.x = (int16_t)x;
        data->point.y = (int16_t)y;
        last_x        = data->point.x;
        last_y        = data->point.y;

        data->state = LV_INDEV_STATE_PR;
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state   = LV_INDEV_STATE_REL;
    }

    return false;
}

void disp_init(void) {
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf, disp_buf1, disp_buf2, LVGL_DRAW_BUFFER_SIZE);
    lv_disp_drv_init(&g_disp_drv);
    g_disp_drv.draw_buf   = &buf;
    g_disp_drv.flush_cb   = disp_flush;
    g_disp_drv.monitor_cb = monitor_cb;
    g_disp_drv.hor_res    = LCD_WIDTH;
    g_disp_drv.ver_res    = LCD_HEIGHT;
    lv_disp_drv_register(&g_disp_drv);
    // TODO: detect touchpad to set enable or not
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touchpad_read;
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
    // start main gui
    init_gui();
    // enter loading screen
    loading_switch_in();
}
