/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: command.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Net/AT/command.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "usart.h"

extern osMessageQId        qid_net_at_application_msg; // handles.h
extern uint8_t             net_at_uart_initialized;
extern UART_HandleTypeDef *net_at_uart;
extern osMessageQId        qid_net_at_rx_msg;
extern uint8_t            *net_at_rx_buffer;
extern uint16_t            net_at_rx_buffer_length;

/*
Status_t NET_AT_CMD_SEND(AT_Command_MSG *msg, uint32_t wait_ticks) {
    if (!net_at_uart_initialized)
        return Failed;
    if (xQueueSend(qid_net_at_application_msg, msg, wait_ticks) != pdTRUE)
        return Failed;
    return OK;
}

Status_t NET_AT_CMD_WAIT(AT_Command_MSG *msg, uint32_t wait_ticks) {
    if (msg->qid_response == NULL)
        return Failed;
    osMessageQId qid = msg->qid_response;
    if (xQueueReceive(qid, msg, wait_ticks) != pdTRUE)
        return Failed;
    return OK;
}

Status_t NET_AT_CMD_SEND_AND_WAIT(uint8_t *buffer, uint16_t buffer_len,
                                  uint32_t wait_ticks) {
    AT_Command_MSG msg;
    Status_t       status = OK;
    osMessageQId   qid    = xQueueCreate(1, sizeof(AT_Command_MSG));
    msg.qid_response      = qid;
    msg.MSG_TYPE          = NET_AT_MSG_REQUEST;
    msg.buffer            = buffer;
    msg.buffer_length     = buffer_len;
    if (NET_AT_CMD_SEND(&msg, wait_ticks) != OK)
        status = Failed;
    else {
        if (NET_AT_CMD_WAIT(&msg, wait_ticks) != OK)
            status = Failed;
        else if (msg.MSG_TYPE == NET_AT_MSG_RESPONSE)
            ; // TODO: process type mismatch which is likely not happened.
    }
    vQueueDelete(qid);
    return status;
}
*/

Status_t NET_AT_REGISTER_RECV_BUFFER(uint8_t *buffer, uint16_t buffer_len,
                                     osMessageQId process_queue) {
    qid_net_at_rx_msg       = process_queue;
    net_at_rx_buffer        = buffer;
    net_at_rx_buffer_length = buffer_len;
    return OK;
}

Status_t NET_AT_START_RECV() {
    if (!net_at_rx_buffer)
        return Failed;
    HAL_UART_Receive_DMA(net_at_uart, net_at_rx_buffer,
                         net_at_rx_buffer_length);
    return OK;
}

Status_t NET_AT_STOP_RECV() {
    HAL_UART_AbortReceive_IT(net_at_uart);
    return OK;
}

Status_t NET_AT_SEND_CMD(uint8_t *buffer, uint16_t len) {
    HAL_UART_Transmit_DMA(net_at_uart, buffer, len);
    return OK;
}
