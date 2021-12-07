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
#include "main.h"

// uint8_t NET_RX_BUFFER[NET_BUFFER_SIZE] = {0};
uint8_t *NET_RX_BUFFER = 0xD0000000 + 2 * (800 * 480 * 2);
uint8_t  NET_STATUS;