/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: command.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_AT_COMMAND_H__
#define __NET_AT_COMMAND_H__

#include "Common/status.h"
#include "cmsis_os.h"

#define NET_AT_MSG_REQUEST  0x11
#define NET_AT_MSG_RESPONSE 0x12

typedef struct {
    uint8_t      MSG_TYPE;
    uint16_t     buffer_length;
    uint8_t     *buffer; // save sent command and returned resquest
    osMessageQId qid_response;
} __attribute__((packed)) AT_Command_MSG;

typedef struct {
    uint8_t  MSG_TYPE;
    uint8_t  flag;
    uint16_t len;
    uint32_t padding;
} __attribute__((packed)) NET_AT_RX_MSG;

/*
Status_t NET_AT_CMD_SEND(AT_Command_MSG *msg, uint32_t wait_ticks);
Status_t NET_AT_CMD_WAIT(AT_Command_MSG *msg, uint32_t wait_ticks);
Status_t NET_AT_CMD_SEND_AND_WAIT(uint8_t *buffer, uint16_t buffer_len,
                                  uint32_t wait_ticks);
                                  */

Status_t NET_AT_REGISTER_RECV_BUFFER(uint8_t *buffer, uint16_t buffer_len,
                                     osMessageQId process_queue);
Status_t NET_AT_START_RECV();
Status_t NET_AT_STOP_RECV();
Status_t NET_AT_SEND_CMD(uint8_t *buffer, uint16_t len);
#define NET_AT_SEND_STATIC_CMD(cmd)                                            \
    NET_AT_SEND_CMD((uint8_t *)cmd, sizeof(cmd) - 1)

#endif // __NET_AT_COMMAND_H__