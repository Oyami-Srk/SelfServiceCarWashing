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

#include "main.h"

#define LCD_ADDRRESS_LAYER_0 0xD0000000
#define LCD_WIDTH            800
#define LCD_HEIGHT           480

void LCD_Clear(void) {
    DMA2D->CR &= ~(DMA2D_CR_START);           //	停止DMA2D
    DMA2D->CR     = DMA2D_R2M;                //	寄存器到SDRAM
    DMA2D->OPFCCR = LTDC_PIXEL_FORMAT_RGB888; //	设置颜色格式
    DMA2D->OOR    = 0;                        //	设置行偏移
    DMA2D->OMAR   = 0xD0000000;               // 地址
    DMA2D->NLR    = (800 << 16) | (480);      //	设定长度和宽度
    DMA2D->OCOLR  = 0x0000FF;                 //	颜色

    // 等待 垂直数据使能显示状态 ，即LTDC即将刷完一整屏数据的时候
    // 因为在屏幕没有刷完一帧时进行刷屏，会有撕裂的现象
    // 用户也可以使用 寄存器重载中断
    // 进行判断，不过为了保证例程的简洁以及移植的方便性，这里直接使用判断寄存器的方法
    //
    //
    while (LTDC->CDSR != 0X00000001)
        ; // 判断 显示状态寄存器LTDC_CDSR 的第0位 VDES：垂直数据使能显示状态
    DMA2D->CR |= DMA2D_CR_START; //	启动DMA2D
    while (DMA2D->CR & DMA2D_CR_START)
        ; //	等待传输完成
}

void LCD_GPIO_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

void LCD_DISPLAY_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
    LCD_Clear();
    uint8_t *buffer = 0xD0000000;
    for (int i = 0; i < 480; i++) {
        for (int j = 0; j < 800; j++) {
            uint8_t *p = buffer + (i * 800 * 3) + j * 3;
            if (i <= 200) {
                p[0] = 0x00;
                p[1] = 0x00;
                p[2] = 0xFF;
            } else if (i <= 400) {
                p[0] = 0x00;
                p[1] = 0xFF;
                p[2] = 0x00;
            } else {
                p[0] = 0xFF;
                p[1] = 0x00;
                p[2] = 0x00;
            }
        }
    }
}