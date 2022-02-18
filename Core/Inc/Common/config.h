/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: config.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#include "main.h"

#define SOFTWARE_VER "0.3.0"

//#define DEBUG_PRINT_USE_RTT
#define DEBUG_PRINT_USE_UART1

//#define NET_MODULE_ESP32
#define NET_MODULE_LTE

#define ENABLE_NET_BUFFER_PRINT

#define ENABLE_NET_LED
#define ENABLE_WORKING_LED

//#define ENABLE_HEARTBEAT
#define HEARTBEAT_INTV 5000 // in ms

#define NET_MAX_RETRIES 3
#ifdef NET_MODULE_ESP32
#define NET_WIFI_AP_NAME "Wireless-2.4GHz"
#define NET_WIFI_AP_PSWD "83885877"
#define SERVER_ADDR      "192.168.0.3"
#else
#define NET_LTE_APN "CMNET"
#define SERVER_ADDR "srk00.qvq.moe"
#endif
#define NET_TCP_KEEPALIVE_TIME "30" // sec
#define NET_TCP_RECONNECT_INTV "10" // 100ms

#define NET_SNTP_SERVER "114.118.7.163"
#define SERVER_PORT     "44551"

#define RTC_COOKIE             0x1234F429
#define RTC_MINIUM_UPDATE_INTV (3600 * 4) // 4 hour

#define FOAM_TO_WATER_FRACTION 1500

//#define NET_UART_USE_1
//#define NET_UART_USE_2
#define NET_UART_USE_3

#endif // __COMMON_CONFIG_H__
