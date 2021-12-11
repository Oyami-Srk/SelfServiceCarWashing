/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: vars.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_VARS_H__
#define __NET_VARS_H__

#include "Net/config.h"
#include "cmsis_os2.h"

extern uint8_t             NET_RX_BUFFER[NET_BUFFER_SIZE];
extern uint8_t             NET_STATUS;
extern char                NET_MAC[18];
extern uint8_t             NET_IPV4[4];
extern osMessageQueueId_t *net_queue;

#endif // __NET_VARS_H__
