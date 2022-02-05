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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <string.h>

#define GPIO(name) name##_GPIO_Port, name##_Pin

#define COMPILE_TIME_ASSERT(expr) typedef char assert_type[expr ? 1 : -1]

#define LOG(msg) printf(msg "\r\n")
#define STATIC_STR_CMP(dyn, static)                                            \
    ((memcmp((dyn), (static), (sizeof((static)) - 1))) == 0)

#define PRINT_BUFFER(BUFFER, LEN, NEWLINE)                                     \
    printf("\r\n");                                                            \
    printf(NEWLINE);                                                           \
    for (int __PRINT_RX_BUFFER_I = 0; __PRINT_RX_BUFFER_I < LEN;               \
         __PRINT_RX_BUFFER_I++) {                                              \
        if (BUFFER[__PRINT_RX_BUFFER_I] == '\n' &&                             \
            __PRINT_RX_BUFFER_I != LEN - 1)                                    \
            printf("\r\n" NEWLINE);                                            \
        else                                                                   \
            printf("%c", BUFFER[__PRINT_RX_BUFFER_I]);                         \
    }                                                                          \
    printf("\r\n")

uint32_t SetRTCTime(const char *str_time); // return time stamp
uint32_t GetRTCTime();                     // return current time stamp
uint32_t GetRTCLastUpdate();               // return last update tiem stamp
char    *ParseTimeInStr(time_t currTime);  // Free after using

#endif // __UTILS_H__
