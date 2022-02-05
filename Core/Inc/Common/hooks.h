/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: hooks.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

//
// Created by shiroko on 2022/2/1.
//

#ifndef __COMMON_HOOKS_H__
#define __COMMON_HOOKS_H__

/* DMA2D Hooks */
void DMA2D_CB();
void DMA2D_CB_ERROR();

/* UART3 Hooks */
void UART3_IRQ_HANDLER();

#endif // __COMMON_HOOKS_H__