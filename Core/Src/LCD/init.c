/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "LCD/handles.h"
#include "dma2d.h"
#include "ltdc.h"
#include "main.h"

void LCD_CLEAR(void) {
    HAL_DMA2D_Abort(&hdma2d);
    hdma2d.Init.Mode                  = DMA2D_R2M;
    hdma2d.Init.ColorMode             = DMA2D_OUTPUT_RGB888;
    hdma2d.LayerCfg[0].AlphaMode      = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[0].InputAlpha     = 0xFF;
    hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[0].InputOffset    = 0;

    if (HAL_DMA2D_Init(&hdma2d) == HAL_OK) {
        if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) == HAL_OK) {
            if (HAL_DMA2D_Start(&hdma2d, 0xFF000000, LCD_LAYER0_MEM_ADDR,
                                LCD_WIDTH, LCD_HEIGHT) == HAL_OK) {
                HAL_DMA2D_PollForTransfer(&hdma2d, 100);
            }
        }
    }
}

void LCD_DMA2D_INIT(void) { LCD_CLEAR(); }

void LCD_GPIO_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

void LCD_DISPLAY_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}