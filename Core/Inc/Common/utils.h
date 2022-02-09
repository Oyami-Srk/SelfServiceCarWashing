/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: utils.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include <string.h>
#include <time.h>

// Debug function
#include "Common/printf-stdarg.h"
int f_putchar(int ch);

#define LOG(msg)       f_printf(msg "\r\n")
#define LOGF(msg, ...) f_printf(msg "\r\n", __VA_ARGS__)
#define PRINTF(...)    f_printf(__VA_ARGS__)

#define GPIO(name) name##_GPIO_Port, name##_Pin

#define COMPILE_TIME_ASSERT(expr) typedef char assert_type[expr ? 1 : -1]

#define STATIC_STR_CMP(dyn, static)                                            \
    ((memcmp((dyn), (static), (sizeof((static)) - 1))) == 0)

#define PRINT_BUFFER(BUFFER, LEN, NEWLINE)                                     \
    PRINTF("\r\n");                                                            \
    PRINTF(NEWLINE);                                                           \
    for (int __PRINT_RX_BUFFER_I = 0; __PRINT_RX_BUFFER_I < LEN;               \
         __PRINT_RX_BUFFER_I++) {                                              \
        if (BUFFER[__PRINT_RX_BUFFER_I] == '\n' &&                             \
            __PRINT_RX_BUFFER_I != LEN - 1)                                    \
            PRINTF("\r\n" NEWLINE);                                            \
        else                                                                   \
            PRINTF("%c", BUFFER[__PRINT_RX_BUFFER_I]);                         \
    }                                                                          \
    PRINTF("\r\n")

uint32_t SetRTCTime(const char *str_time); // return time stamp
uint32_t GetRTCTime();                     // return current time stamp
uint32_t GetRTCLastUpdate();               // return last update tiem stamp
char    *ParseTimeInStr(time_t currTime);  // Free after using

#endif // __UTILS_H__
