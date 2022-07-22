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
    AT_RDY,
    AT_UART_TIMEOUT,
} AT_RESULT;

typedef enum {
    NET_NOT_CONNECT,
    NET_NO_IP,
    NET_CONNECTED,
    NET_CONNECTED_TO_SERVER,
    NET_DEVICE_REGISTERED
} NET_STATUS;

/* Must implement this function in each module */
void    NET_MODULE_INIT();
void    NET_MODULE_UART_PROC(uint8_t *buffer, uint16_t len);
uint8_t NET_MODULE_GET_RADIO_STRENGTH();
/* ******************************************* */

#define MAX_DELAY_QUEUE_WAIT 2000

AT_RESULT AT_GetResult(uint8_t *buffer, uint16_t len);

void           AT_SetNetStatus(NET_STATUS status);
NET_STATUS     AT_GetNetStatus();
void           AT_SetIdent(const char *ident);
const char    *AT_GetIdent();
void           AT_SetIP(const uint8_t *ip_array);
const uint8_t *AT_GetIP();
void           AT_ResetStatus();
static inline uint8_t AT_GetRadioStrength() {
    return NET_MODULE_GET_RADIO_STRENGTH();
}


#ifdef NET_MODULE_LTE
#define NET_IDENT_SIZE 15
#endif
#ifdef NET_MODULE_ESP32
#define NET_IDENT_SIZE 17
#endif

// FreeRTOS wait for status
#define AT_WAIT_INTV pdMS_TO_TICKS(100) // 100 ms check intv
BaseType_t AT_WaitForStatus(NET_STATUS status, TickType_t max_delay);

// UART
AT_RESULT AT_UART_RegisterResponse(QueueHandle_t queueHandle);
AT_RESULT AT_UART_UnregisterResponse(QueueHandle_t queueHandle);
void      AT_UART_Send(uint8_t *buffer, uint16_t len);
#define AT_UART_SendStatic(msg) AT_UART_Send((uint8_t *)(msg), sizeof(msg) - 1)
AT_RESULT AT_UART_Recv(QueueHandle_t queue, AT_Response_Msg_t *msg_buffer,
                       TickType_t timeout);

#define AT_DEFAULT_WAIT_DELAY pdMS_TO_TICKS(10 * 1000) // wait 10 secs.

#define AT_FREE_RESP(msg) vPortFree((msg).Buffer)

#endif // __NET_AT_H__
