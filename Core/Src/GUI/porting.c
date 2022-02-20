/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: porting.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

/* Porting function */
#include "FreeRTOS.h"
#include "task.h"

#include "GUI/porting.h"
#include "GUI/gui_lvgl.h"

#include "Common/printf-stdarg.h"
#include "Tasks/tasks.h"
#include "Net/cmd.h"
#include "main.h"
#include "rtc.h"

#define LOGIN_SUCCESS 0x00
#define LOGIN_FAILED  0x01

void get_clock_str(char *buffer, bool show_mark) {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    memset(&time, 0, sizeof(RTC_TimeTypeDef));
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    if (show_mark)
        my_sprintf(buffer, "%02d:%02d:%02d", time.Hours, time.Minutes,
                   time.Seconds);
    else
        my_sprintf(buffer, "%02d %02d %02d", time.Hours, time.Minutes,
                   time.Seconds);
}

uint8_t login(const char *username, const char *password, char *dispname) {
    char userId[64];
    memset(userId, 0, 64);

    float avail;
    if (Cmd_UserLogin(username, password, userId, dispname, &avail) ==
        CMD_RESULT_OK) {
        start_inuse_task(userId, avail);
        return LOGIN_SUCCESS;
    }
    return LOGIN_FAILED;
}

void logout() {
    inuse_information_t info;
    get_inuse_information(&info);
    stop_inuse_task();
    while (Cmd_UserLogOut(info.userId, info.current_usage_water,
                          info.current_usage_foam,
                          info.current_used_time) != CMD_RESULT_OK) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* End of Porting function */
