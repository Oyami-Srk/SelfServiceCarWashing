/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: common_vars.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Net/config.h"
#include "cmsis_os2.h"
#include "main.h"

uint8_t             NET_RX_BUFFER[NET_BUFFER_SIZE] = {0};
uint8_t             NET_STATUS;
char                NET_MAC[18] = {0};
uint8_t             NET_IPV4[4] = {0};
osMessageQueueId_t *net_queue   = NULL;