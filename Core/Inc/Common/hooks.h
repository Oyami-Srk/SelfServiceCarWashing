/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: hooks.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __COMMON_HOOKS_H__
#define __COMMON_HOOKS_H__

/* DMA2D Hooks */
void DMA2D_CB();
void DMA2D_CB_ERROR();

/* UART3 Hooks */
void NET_UART_IRQ_HANDLER();

/* TIM3 Hooks */
void TIM3_IT_HANDLER();

/* EXTI Hooks */
/*
 * EXTI5 - BTN1
 * EXTI4 - BTN2
 * EXTI2 - BTN3
 * EXTI3 - BTN4
 * EXTI6 - BTN7
 * EXTI7 - BTN8
 */
void EXTI_BTN_IT_HANDLER();

#endif // __COMMON_HOOKS_H__