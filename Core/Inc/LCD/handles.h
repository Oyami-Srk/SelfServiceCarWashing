/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __LCD_HANDLES_H__
#define __LCD_HANDLES_H__

// call after msp init of gpio
void LCD_GPIO_INIT();
// call after ltds init
void LCD_DISPLAY_INIT();
// call after dma2d init
void LCD_DMA2D_INIT();

#endif // __LCD_HANDLES_H__