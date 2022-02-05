/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: uart.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

//
// Created by shiroko on 2022/2/1.
//

#include "Common/hooks.h"
#include "Common/init.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "Net/at.h"
#include "cmsis_os.h"
#include "usart.h"
#include <stdio.h>

#define UART3_DATA_SIZE 256

typedef struct {
    uint16_t rxLen;
    uint8_t *rxData;
} UART_RX_Mail_t;

static uint8_t           UART3_RxBuffer[UART3_DATA_SIZE] = {0};
static uint16_t          UART3_RxLen                     = 0;
static SemaphoreHandle_t UART3_RxSmph                    = NULL;

QueueHandle_t AT_Resp_Queue = NULL;

_Noreturn void task_uart3(void *args) {
    static AT_Response_Msg_t msg;

    for (;;) {
        if (xSemaphoreTake(UART3_RxSmph, portMAX_DELAY) == pdTRUE) {
            uint8_t *buffer = pvPortMalloc(sizeof(uint8_t) * (UART3_RxLen + 1));
            if (!buffer) {
                LOG("[UART] Cannot alloc memory for buffer, OOM!");
            } else {
                memcpy(buffer, UART3_RxBuffer, UART3_RxLen);
#ifdef ENABLE_NET_BUFFER_PRINT
                PRINT_BUFFER(buffer, UART3_RxLen, ">>>>  ");
#endif
                if (AT_Resp_Queue) {
                    msg.Buffer = buffer;
                    msg.Len    = UART3_RxLen;
                    if (xQueueSend(AT_Resp_Queue, &msg,
                                   pdMS_TO_TICKS(MAX_DELAY_QUEUE_WAIT)) !=
                        pdTRUE) {
                        // queue not sent, destroy message
                        LOG("[UART] Queue Item cannot be sent.");
                        vPortFree(buffer);
                    }
                } else {
                    NET_MODULE_UART_PROC(buffer, UART3_RxLen);
                }
            }
            // restart recive
            HAL_UART_Receive_DMA(&huart3, UART3_RxBuffer, UART3_DATA_SIZE);
        }
        osDelay(1);
    }
}

void NET_UART_INIT_RTOS() {
    LOG("[UART] Starting NET UART Initialization.");
    UART3_RxSmph = xSemaphoreCreateBinary();

    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart3, UART3_RxBuffer, UART3_DATA_SIZE);
    xTaskCreate(task_uart3, "UART3", 256, NULL, tskIDLE_PRIORITY, NULL);

    NET_MODULE_INIT();
}

void UART3_IRQ_HANDLER() {
    BaseType_t xHigherPrioTaskWaken;

    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);

        HAL_UART_DMAStop(&huart3);

        UART3_RxLen = UART3_DATA_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);

        if (UART3_RxSmph != NULL) {
            // TODO: Random Hard Fault. Solve it or make it safe to reset.
            xSemaphoreGiveFromISR(UART3_RxSmph, &xHigherPrioTaskWaken);
        }
    }

    portYIELD_FROM_ISR(xHigherPrioTaskWaken);
}
