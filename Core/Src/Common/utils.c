/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: utils.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/utils.h"
#include "Common/config.h"
#include "FreeRTOS.h"
#include "rtc.h"
#include <stdio.h>
#include <time.h>

#ifdef NET_MODULE_ESP32
/*
static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed",
                                    "Thu", "Fri", "Sat"};*/
static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif

uint32_t SetRTCTime(const char *str_time) {
    // NOTICE: This Time System will be broken after 2106-02-07
    char            weekday[4] = {0};
    char            month[4]   = {0};
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
#ifdef NET_MODULE_ESP32
    // Process ESP32 time response
    // Eg: Sat Feb  5 01:29:31 2022
    uint16_t year;
    sscanf(str_time, "%s %s %hhu %hhu:%hhu:%hhu %hu", weekday, month,
           &date.Date, &time.Hours, &time.Minutes, &time.Seconds, &year);
    if (year < 2022) {
        return 0;
    }
    date.Year = year - 2000; // RTC year between 0-99
    for (int i = 0; i < 12; i++) {
        if (strcmp(month, mon_name[i]) == 0) {
            date.Month = i + 1;
            break;
        }
    }
#else
    // Process LTE time response
    // Eg: 2021/07/19,09:22:04+32
#endif
    HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_COOKIE);

    struct tm curr = {.tm_year = date.Year + 100, // tm_year start from 1900
                      .tm_mon  = date.Month - 1,  // tm_mon start from 0
                      .tm_mday = date.Date,
                      .tm_hour = time.Hours,
                      .tm_min  = time.Minutes,
                      .tm_sec  = time.Seconds};
    uint32_t  timestamp = mktime(&curr) & 0xFFFFFFFF;
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, timestamp);
    // Disable time stamp for touch screen
    if (HAL_RTCEx_DeactivateTimeStamp(&hrtc) != HAL_OK) {
        LOG("[RTC] Failed to deactivate time stamp for touch screen.");
        Error_Handler();
    }
    return timestamp;
}

uint32_t GetRTCTime() {
    if (GetRTCLastUpdate() == 0)
        return 0;
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    struct tm curr = {.tm_year = date.Year + 100, // tm_year start from 1900
                      .tm_mon  = date.Month - 1,  // tm_mon start from 0
                      .tm_mday = date.Date,
                      .tm_hour = time.Hours,
                      .tm_min  = time.Minutes,
                      .tm_sec  = time.Seconds};
    return mktime(&curr) &
           0xFFFFFFFF; // only take bottom half, available before year 2106
}

uint32_t GetRTCLastUpdate() {
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != RTC_COOKIE)
        return 0;
    return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
}

char *ParseTimeInStr(time_t currTime) {
    char *time_buffer = (char *)pvPortMalloc(sizeof(char) * 100);
    strftime(time_buffer, 100, "%Y/%m/%d %X", localtime(&currTime));
    return time_buffer;
}

/* Debug output */

#ifdef DEBUG_PRINT_USE_RTT
#include "SEGGER_RTT.h"
#endif
#ifdef DEBUG_PRINT_USE_UART1
#include "usart.h"
#endif

int f_putchar(int ch) {
#ifdef DEBUG_PRINT_USE_RTT
    SEGGER_RTT_PutChar(0, ch);
#endif
#ifdef DEBUG_PRINT_USE_UART1
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
#endif
    return ch;
}