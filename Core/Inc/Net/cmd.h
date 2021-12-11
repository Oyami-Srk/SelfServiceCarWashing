/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: cmd.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_CMD_H__
#define __NET_CMD_H__

#include "cmsis_os2.h"
#include <stdint.h>

typedef struct {
    uint8_t *recv;
    uint16_t len;
} NET_MSG;

#define SEND_MSG_STATUS_OK     0x00
#define SEND_MSG_STATUS_FAILED 0x01

uint8_t SEND_MSG(uint8_t *msg, uint16_t len, osMessageQueueId_t qid_recv_msg);
void    RECIVED_MSG();

#endif
