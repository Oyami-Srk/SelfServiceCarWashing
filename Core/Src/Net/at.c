/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: at.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

//
// Created by shiroko on 2022/2/3.
//
#include "Net/at.h"
#include "Common/config.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "main.h"
#include "usart.h"

extern QueueHandle_t AT_Resp_Queue;

void AT_SendCommand(uint8_t *buffer, uint16_t len) {
    HAL_UART_Transmit_DMA(&huart3, buffer, len);
}

AT_RESULT AT_RegisterResponse(QueueHandle_t queueHandle) {
    if (AT_Resp_Queue != NULL)
        return AT_BUSY; // Response can only have one registered at one time
    AT_Resp_Queue = queueHandle;
    return AT_OK;
}

AT_RESULT AT_UnregisterResponse(QueueHandle_t queueHandle) {
    if (AT_Resp_Queue != queueHandle)
        return AT_ERROR;
    AT_Resp_Queue = NULL;
    return AT_OK;
}

AT_RESULT AT_GetResult(uint8_t *buffer, uint16_t len) {
    char *p = (char *)(buffer + len - 1);
    if (*p != '\n')
        return AT_PARSE_ERROR; // not ending with \n, an error response
    p--;
    int i = len - 1;
    while (*p != '\n' && (uint32_t)p > (uint32_t)buffer) {
        p--;
        i--;
    }
    char pb[16] = {0};
    memcpy(pb, buffer + i, len - i - 2);
    if (STATIC_STR_CMP(pb, "OK"))
        return AT_OK;
    if (STATIC_STR_CMP(pb, "ERROR"))
        return AT_ERROR;
    // TODO: more comparison
    return AT_OTHER;
}

NET_STATUS AT_Net_Status;
char       AT_Net_MacAddr[18];
uint8_t    AT_Net_IP[4];

void AT_ResetStatus() {
    memset(AT_Net_MacAddr, 0, 18);
    memset(AT_Net_IP, 0, 4);
    AT_Net_Status = NET_NOT_CONNECT;
}

void AT_SetNetStatus(NET_STATUS status) {
    AT_Net_Status = status;
#ifdef ENABLE_NET_LED
    switch (status) {
    case NET_CONNECTED:
        HAL_GPIO_WritePin(GPIO(NET_STATUS_LED), GPIO_PIN_SET);
        break;
    default:
        HAL_GPIO_WritePin(GPIO(NET_STATUS_LED), GPIO_PIN_RESET);
        break;
    }
#endif
}

NET_STATUS AT_GetNetStatus() { return AT_Net_Status; }

void AT_SetMacAddr(const char *mac) {
    memcpy(AT_Net_MacAddr, mac, 18 * sizeof(char));
}

const char *AT_GetMacAddr() { return AT_Net_MacAddr; }

void AT_SetIP(const uint8_t *ip_array) {
    memcpy(AT_Net_IP, ip_array, 4 * sizeof(uint8_t));
}

const uint8_t *AT_GetIP() { return AT_Net_IP; }
