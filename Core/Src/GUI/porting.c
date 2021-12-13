/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: porting.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

/* Porting function */
#include "Comm/procedure.h"
#include "GUI/gui_lvgl.h"
#include "main.h"
#include "rtc.h"
#include "time.h"
#include <stdio.h>

#define LVGL_DRAW_BUFFER_SIZE LCD_WIDTH * 200
lv_color_t *qr_canvas_buf =
    (lv_color_t *)(LCD_END_ADDR +
                   2 * (LVGL_DRAW_BUFFER_SIZE * sizeof(lv_color_t)));

#define LOGIN_SUCCESS 0x00
#define LOGIN_FAILED  0x01

void get_time_str(char *buffer, uint8_t show_mark) {
    RTC_TimeTypeDef time;
    memset(&time, 0, sizeof(RTC_TimeTypeDef));
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    sprintf(buffer, "%02hhu:%02hhu:%02hhu", time.Hours, time.Minutes,
            time.Seconds);
}

void switch_to_inuse_scr(const char *user);
void switch_to_login_scr();
void switch_to_loading_scr();

char  CURRENT_USER_ID[40]      = {0};
float CURRENT_USER_AVAIL_WATER = 0.0f;
float CURRENT_USER_USED_WATER  = 0.0f;
float CURRENT_USER_USED_FOAM   = 0.0f;
float CURRENT_USER_FLOW_SPEED  = 0.0f;

uint8_t login(const char *username, const char *password) {
    printf("Tring to login with %s and password %s.\n", username, password);
    if (CMD_RESULT_OK == USER_LOGIN(username, password, CURRENT_USER_ID,
                                    &CURRENT_USER_AVAIL_WATER)) {
        switch_to_inuse_scr(username);
        return LOGIN_SUCCESS;
    } else {
        return LOGIN_FAILED;
    }
}

float getCurrentFlow() { return 1; } // in ml/s

float getCurrentUsage() {
    return 10; // in L
}

int getCurrentUsingTime() { return HAL_GetTick(); }

void logout() {
    switch_to_loading_scr();
    switch_to_login_scr();
}

uint16_t get_qr_code(uint8_t **pArray) {
    *pArray = NULL;
    return 0;
}
/* End of Porting function */
