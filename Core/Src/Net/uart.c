/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: uart.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

//
// Created by shiroko on 2022/2/1.
//

#include "Common/hooks.h"
#include "Common/init.h"
#include "Common/utils.h"
#include "Common/config.h"
#include "Net/at.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usart.h"
#include <stdio.h>

#define NET_UART_DATA_SIZE 1024

#ifdef NET_UART_USE_1
#define HUART huart1
#endif
#ifdef NET_UART_USE_2
#define HUART huart2
#endif
#ifdef NET_UART_USE_3
#define HUART huart3
#endif

typedef struct {
    uint16_t rxLen;
    uint8_t *rxData;
} UART_RX_Mail_t;

static uint8_t           NET_UART_RxBuffer[NET_UART_DATA_SIZE] = {0};
static uint16_t          NET_UART_RxLen                        = 0;
static SemaphoreHandle_t NET_UART_RxSmph                       = NULL;

QueueHandle_t AT_Resp_Queue = NULL;

_Noreturn void task_net_uart(__attribute__((unused)) void *args) {
    static AT_Response_Msg_t msg;

    for (;;) {
        if (xSemaphoreTake(NET_UART_RxSmph, portMAX_DELAY) == pdTRUE) {
            uint8_t *buffer =
                pvPortMalloc(sizeof(uint8_t) * (NET_UART_RxLen + 1));
            if (!buffer) {
                LOG("[UART] Cannot alloc memory for buffer, OOM!");
            } else {
                memcpy(buffer, NET_UART_RxBuffer, NET_UART_RxLen);
#ifdef ENABLE_NET_BUFFER_PRINT
                PRINT_BUFFER(buffer, NET_UART_RxLen, ">>>>  ");
#endif
                if (AT_Resp_Queue) {
                    msg.Buffer = buffer;
                    msg.Len    = NET_UART_RxLen;
                    if (xQueueSend(AT_Resp_Queue, &msg,
                                   pdMS_TO_TICKS(MAX_DELAY_QUEUE_WAIT)) !=
                        pdTRUE) {
                        // queue not sent, destroy message
                        LOG("[UART] Queue Item cannot be sent.");
                        vPortFree(buffer);
                    }
                } else {
                    NET_MODULE_UART_PROC(buffer, NET_UART_RxLen);
                }
            }
            // restart receive
            HAL_UART_Receive_DMA(&HUART, NET_UART_RxBuffer, NET_UART_DATA_SIZE);
        }
        osDelay(1);
    }
}

void NET_UART_INIT_RTOS() {
    LOG("[UART] Starting NET UART Initialization.");
    NET_UART_RxSmph = xSemaphoreCreateBinary();

    __HAL_UART_ENABLE_IT(&HUART, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&HUART, NET_UART_RxBuffer, NET_UART_DATA_SIZE);
    xTaskCreate(task_net_uart, "NET-UART", 256, NULL, tskIDLE_PRIORITY, NULL);

    NET_MODULE_INIT();
}

void NET_UART_IRQ_HANDLER() {
    BaseType_t xHigherPrioTaskWaken;

    if (__HAL_UART_GET_FLAG(&HUART, UART_FLAG_IDLE) != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&HUART);

        HAL_UART_DMAStop(&HUART);

        NET_UART_RxLen =
            NET_UART_DATA_SIZE - __HAL_DMA_GET_COUNTER(HUART.hdmarx);

        if (NET_UART_RxSmph != NULL) {
            // TODO: Random Hard Fault. Solve it or make it safe to reset.
            xSemaphoreGiveFromISR(NET_UART_RxSmph, &xHigherPrioTaskWaken);
        }
    }

    portYIELD_FROM_ISR(xHigherPrioTaskWaken);
}
