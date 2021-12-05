/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: utils.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_AT_UTILS_H__
#define __NET_AT_UTILS_H__

#include "main.h"

#define PRINT_RX_BUFFER(BUFFER, LEN, NEWLINE)                                  \
    printf(NEWLINE);                                                           \
    for (int __PRINT_RX_BUFFER_I = 0; __PRINT_RX_BUFFER_I < LEN;               \
         __PRINT_RX_BUFFER_I++) {                                              \
        if (BUFFER[__PRINT_RX_BUFFER_I] == '\n' &&                             \
            __PRINT_RX_BUFFER_I != LEN - 1)                                    \
            printf("\n" NEWLINE);                                              \
        else                                                                   \
            printf("%c", BUFFER[__PRINT_RX_BUFFER_I]);                         \
    }                                                                          \
    1 == 1

typedef enum {
    AT_OK = 0x00,
    AT_CONNECT,
    AT_RING,
    AT_ERROR,
    AT_BUSY,
    AT_NO_ANSWER,
    AT_PARSER_ERROR
} AT_RESULT;

AT_RESULT AT_GET_RESULT(uint8_t *buffer, uint16_t len);

#endif // __NET_AT_UTILS_H__
