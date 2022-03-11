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
#include "ltdc.h"
#include "fatfs.h"
#include "Common/utils.h"

static int32_t           x1_flush;
static int32_t           y1_flush;
static int32_t           x2_flush;
static int32_t           y2_fill;
static int32_t           y_fill_act;
static const lv_color_t *buf_to_flush;

#define LVGL_DRAW_BUFFER_SDRAM 1
//#define USE_LTDC_ADDR_SW

static uint8_t *font1_buffer = (uint8_t *)0xD0234000; // 4MB size
static uint8_t *font2_buffer = (uint8_t *)0xD0634000; // <9MB size
#if LVGL_DRAW_BUFFER_SDRAM
#define LVGL_DRAW_BUFFER_SIZE (LCD_WIDTH * LCD_HEIGHT)
// static lv_color_t *disp_buf1 = (lv_color_t *)LCD_END_ADDR;
static lv_color_t *disp_buf1 = (lv_color_t *)0xD00BC000;
static lv_color_t *disp_buf2 = (lv_color_t *)0xD0178000;
//    (lv_color_t *)(LCD_END_ADDR + LVGL_DRAW_BUFFER_SIZE * sizeof(lv_color_t));
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
#ifndef USE_LTDC_ADDR_SW
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
#else
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)color_p, 0);
    HAL_LTDC_ProgramLineEvent(&hltdc, 480);
    HAL_Delay(1);
//    g_gpu_state = 1;
#endif
}

void LTDC_IRQ_HANDLER() {
#ifdef USE_LTDC_ADDR_SW
    //    if (g_gpu_state == 1) {
    //        g_gpu_state = 0;
    lv_disp_flush_ready(&g_disp_drv);
//    }
#endif
}

void DMA2D_CB(__attribute__((unused)) DMA2D_HandleTypeDef *p_hdma2d) {
#ifndef USE_LTDC_ADDR_SW
    if (g_gpu_state == 1) {
        g_gpu_state = 0;
        lv_disp_flush_ready(&g_disp_drv);
    }
#endif
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

HAL_StatusTypeDef load_font(const char *path, uint8_t *buffer) {
    static FIL     file;
    static FRESULT f_res;

    FILINFO info;
    f_res = f_stat(path, &info);

    if (f_res != RES_OK) {
        LOG("[FATFS] Stat file failed.");
        return HAL_ERROR;
    }
    LOGF("[FATFS] Found Font %s, file size: %d bytes.", path, info.fsize);

    f_res = f_open(&file, path, FA_READ);
    if (f_res != RES_OK) {
        LOGF("[FATFS] Open Font file %s failed.", path);
        return HAL_ERROR;
    }

    UINT br;
    f_res = f_read(&file, buffer, info.fsize, &br);
    if (f_res != RES_OK) {
        LOGF("[FATFS] Load Font file %s failed.", path);
        return HAL_ERROR;
    }
    LOGF("[FATFS] Load Font file %s succeed, loaded %d bytes.", path, br);

    f_res = f_close(&file);
    if (f_res != RES_OK) {
        LOG("[FATFS] Close file failed.");
        return HAL_OK;
    }
    return HAL_OK;
}

const lv_font_t *NotoSansCJK_24;
const lv_font_t *NotoSansCJK_36;

void disp_init(void) {
    static lv_disp_draw_buf_t buf;
#if LVGL_DRAW_BUFFER_SDRAM
//    lv_color_t *disp_buf1 =
//        (lv_color_t *)lv_mem_alloc(LVGL_DRAW_BUFFER_SIZE *
//        sizeof(lv_color_t));
//    lv_color_t *disp_buf2 =
//        (lv_color_t *)lv_mem_alloc(LVGL_DRAW_BUFFER_SIZE *
//        sizeof(lv_color_t));
#endif
    lv_disp_draw_buf_init(&buf, disp_buf1, disp_buf2, LVGL_DRAW_BUFFER_SIZE);
    lv_disp_drv_init(&g_disp_drv);
    g_disp_drv.draw_buf   = &buf;
    g_disp_drv.flush_cb   = disp_flush;
    g_disp_drv.monitor_cb = monitor_cb;
    g_disp_drv.hor_res    = LCD_WIDTH;
    g_disp_drv.ver_res    = LCD_HEIGHT;
#ifdef USE_LTDC_ADDR_SW
    g_disp_drv.full_refresh = 1;
//    g_disp_drv.direct_mode  = 1;
#endif
    lv_disp_drv_register(&g_disp_drv);
    // TODO: detect touchpad to set enable or not
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb =
        (void (*)(struct _lv_indev_drv_t *, lv_indev_data_t *))touchpad_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    // Load fonts from SPI FATFs
    extern void fatfs_test();
    fatfs_test();
    if (HAL_OK == load_font("0:N24.bin", font1_buffer)) {
        extern lv_font_t f_NotoSansCJK_24;
        NotoSansCJK_24 = &f_NotoSansCJK_24;
    } else {
        NotoSansCJK_24 = &lv_font_montserrat_24;
    }

    if (HAL_OK == load_font("0:N36.bin", font2_buffer)) {
        extern lv_font_t f_NotoSansCJK_36;
        NotoSansCJK_36 = &f_NotoSansCJK_36;
    } else {
        NotoSansCJK_36 = &lv_font_montserrat_36;
    }

    // start main gui
    init_gui();
    // enter loading screen
    loading_switch_in();
}
