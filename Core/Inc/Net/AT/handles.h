/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 * This module requires FREERTOS and DMA to be in use.
 * This module provided handler functions definitions for generated code user
 * insertion
 */

#ifndef __NET_AT_HANDLES_H__
#define __NET_AT_HANDLES_H__

#include "main.h"

// at communication interrupt handler for uart devices
void NET_AT_UART_INTERRUPT_HANDLER(void);

// initialize at communication usart
// Call it after MX_USART Init
void NET_AT_UART_INIT(UART_HandleTypeDef *uart);

// initialize at communication tasks for freertos
// Call it in freertos initialize
void NET_AT_FREERTOS_INIT(void);

#endif // __NET_AT_HANDLES_H__