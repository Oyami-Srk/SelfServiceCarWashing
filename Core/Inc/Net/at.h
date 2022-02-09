/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: at.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

//
// Created by shiroko on 2022/2/3.
//

#ifndef __NET_AT_H__
#define __NET_AT_H__

#include "Common/config.h"
#include "FreeRTOS.h"
#include "Net/wifi.h"
#include "queue.h"
#include <stdio.h>

typedef struct {
    uint8_t *Buffer; // free after read
    uint16_t Len;
} AT_Response_Msg_t;

typedef enum {
    AT_OK = 0x00,
    AT_CONNECT,
    AT_RING,
    AT_ERROR,
    AT_BUSY,
    AT_NO_ANSWER,
    AT_PARSE_ERROR,
    AT_OTHER,
} AT_RESULT;

typedef enum {
    NET_NOT_CONNECT,
    NET_NO_IP,
    NET_CONNECTED,
    NET_CONNECTED_TO_SERVER,
    NET_DEVICE_REGISTERED
} NET_STATUS;

#ifdef NET_MODULE_ESP32
// using esp32 as net module
#define NET_MODULE_INIT      NET_WIFI_INIT
#define NET_MODULE_UART_PROC NET_WIFI_UART_PROC
#else
// using 4g module as net module
#define NET_MODULE_INIT      NET_LTE_INIT
#define NET_MODULE_UART_PROC NET_LET_INIT
#endif

#define MAX_DELAY_QUEUE_WAIT 2000

void AT_SendCommand(uint8_t *buffer, uint16_t len);
#define AT_SendStaticCommand(msg)                                              \
    AT_SendCommand((uint8_t *)(msg), sizeof(msg) - 1)

AT_RESULT AT_RegisterResponse(QueueHandle_t queueHandle);
AT_RESULT AT_UnregisterResponse(QueueHandle_t queueHandle);

AT_RESULT AT_GetResult(uint8_t *buffer, uint16_t len);

void           AT_SetNetStatus(NET_STATUS status);
NET_STATUS     AT_GetNetStatus();
void           AT_SetMacAddr(const char *mac);
const char    *AT_GetMacAddr();
void           AT_SetIP(const uint8_t *ip_array);
const uint8_t *AT_GetIP();
void           AT_ResetStatus();

#define AT_WAIT_DELAY pdMS_TO_TICKS(10 * 1000) // wait 10 secs.

#define AT_WAIT_FOR_RESP_WITH_DELAY(queue, msg, delay)                         \
    if (xQueueReceive(queue, &msg, delay) == pdFALSE) {                        \
        LOG("[UART] Cannot receive response.");                                \
        Error_Handler();                                                       \
    }
#define AT_WAIT_FOR_RESP(queue, msg)                                           \
    AT_WAIT_FOR_RESP_WITH_DELAY(queue, msg, AT_WAIT_DELAY)
#define AT_FREE_RESP(msg) vPortFree(msg.Buffer)

// FreeRTOS wait for status
#define AT_WAIT_INTV pdMS_TO_TICKS(100) // 100 ms check intv
BaseType_t AT_WaitForStatus(NET_STATUS status, TickType_t max_delay);

#endif // __NET_AT_H__
