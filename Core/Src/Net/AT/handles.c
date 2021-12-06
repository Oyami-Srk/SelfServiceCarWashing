/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "Common/msg_types.h"
#include "Net/AT/command.h"
#include "Net/AT/handles.h"
#include "Net/config.h"
#include "cmsis_os.h"
#include <stdio.h>

uint8_t net_at_uart_initialized = 0;

UART_HandleTypeDef *net_at_uart             = NULL;
uint8_t            *net_at_rx_buffer        = NULL;
uint16_t            net_at_rx_buffer_length = 0;

osThreadId pid_net_at_uart        = 0;
osThreadId pid_net_at_application = 0;

const osThreadAttr_t task_net_at_uart_attributes = {
    .name       = "net-at-uart",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t task_net_at_application_attributes = {
    .name       = "net-at-application",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityNormal,
};

osMessageQId qid_net_at_rx_msg          = NULL;
osMessageQId qid_net_at_application_msg = NULL;

// handle message form interrupt and process, then send back to request task
_Noreturn void task_net_at_uart(void *argument);
// handle message from net work task
_Noreturn void task_net_at_application(void *argument);

NET_AT_RX_MSG __static_alloc_net_at_rx_msg;

void NET_AT_UART_INTERRUPT_HANDLER(void) {
    BaseType_t     xHigherPriorityTaskWoken;
    NET_AT_RX_MSG *msg = &__static_alloc_net_at_rx_msg;
    msg->MSG_TYPE      = NET_AT_MSG_UART_RECV;

    if ((__HAL_UART_GET_FLAG(net_at_uart, UART_FLAG_IDLE) != RESET)) {
        // if not bus reset
        __HAL_UART_CLEAR_IDLEFLAG(net_at_uart);
        // stop dma receive for data correct
        // re-enable it in freertos tasks.
        // HAL_UART_DMAStop(net_at_uart);
        HAL_UART_AbortReceive(net_at_uart);
        uint16_t len = net_at_rx_buffer_length -
                       __HAL_DMA_GET_COUNTER(net_at_uart->hdmarx);
        msg->len  = len;
        msg->flag = 1;
        if (net_at_uart_initialized && qid_net_at_rx_msg) {
            xQueueSendToBackFromISR(qid_net_at_rx_msg, (void *)msg,
                                    &xHigherPriorityTaskWoken);
            msg->flag = 0;
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        HAL_UART_Receive_DMA(net_at_uart, net_at_rx_buffer,
                             net_at_rx_buffer_length);
    }
}

void NET_AT_UART_INIT(UART_HandleTypeDef *uart) {
    printf("[MODULE] NET/AT.UART Initializing.\r\n");
    net_at_uart = uart;

    // Enable IDLE interrupt for receiving signal when transmission finished
    __HAL_UART_ENABLE_IT(uart, UART_IT_IDLE);
    // HAL_UART_Receive_DMA(uart, net_at_rx_buffer, NET_AT_BUFFER_SIZE);

    printf("[MODULE] NET/AT.UART Initialization completed.\r\n");
}

void NET_AT_FREERTOS_INIT(void) {
    printf("[MODULE] NET/AT.FREERTOS Initializing.\r\n");

    //    qid_net_at_rx_msg          = xQueueCreate(2, sizeof(NET_AT_RX_MSG));
    //    qid_net_at_application_msg = xQueueCreate(16, sizeof(AT_Command_MSG));

    /*
    pid_net_at_uart =
        osThreadNew(task_net_at_uart, NULL, &task_net_at_uart_attributes);
        */
    /*
    pid_net_at_application = osThreadNew(task_net_at_application, NULL,
                                         &task_net_at_application_attributes);
                                         */

    net_at_uart_initialized = 1;
    printf("[MODULE] NET/AT.FREERTOS Initialization completed.\r\n");
}

_Noreturn void task_net_at_uart(void *argument) {
    printf("[NET/AT] Entered UART Task.\r\n");
    NET_AT_RX_MSG msg;
    for (;;) {
        xQueueReceive(qid_net_at_rx_msg, (void *)&msg, osWaitForever);
        if (msg.flag) {
            printf("Received: %d bytes.\r\n", msg.len);
            for (int i = 0; i < msg.len; i++)
                printf("%c", net_at_rx_buffer[i]);
            msg.flag = 0;
            HAL_UART_Receive_DMA(net_at_uart, net_at_rx_buffer,
                                 net_at_rx_buffer_length);
        }
    }
}

_Noreturn void task_net_at_application(void *argument) {
    printf("[NET/AT] Entered Application Task.\r\n");
    AT_Command_MSG msg;
    for (;;) {
        xQueueReceive(qid_net_at_application_msg, (void *)&msg, osWaitForever);
        printf("Received an msg.\r\n");
        // send to uart
        // wait uart tasks to finish
        // send back
        msg.MSG_TYPE     = NET_AT_MSG_RESPONSE;
        osMessageQId qid = msg.qid_response;
        if (pdFALSE ==
            xQueueSend(qid, (void *)&msg,
                       pdMS_TO_TICKS(1000))) { // wait 1 sec for queue.
            printf("[NET/AT] Send back response failed.\r\n");
        }
    }
}
