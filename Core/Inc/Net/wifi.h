/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: wifi.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

//
// Created by shiroko on 2022/2/3.
//

#ifndef __NET_WIFI_H__
#define __NET_WIFI_H__

#include "FreeRTOS.h"
#include "Net/at.h"

// Init before UART start receiving
void NET_WIFI_INIT();
// Process active message while No task registered queue
void NET_WIFI_UART_PROC(uint8_t *buffer, uint16_t len);

#endif // __NET_WIFI_H__k