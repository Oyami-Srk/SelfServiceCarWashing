/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

/* LCD */
// call after msp init of gpio
void LCD_INIT_GPIO();
// call after ltds init
void LCD_INIT_DISP();
// call after dma2d init
void LCD_INIT_DMA2D();
// call in RTOS init
void LCD_INIT_RTOS();

/* SDRAM */
void SDRAM_INIT_FMC();

/* Net UART3 */
void NET_UART_INIT_RTOS();

/* Tasks */
void TASKS_INIT_RTOS();

#endif // __COMMON_INIT_H__