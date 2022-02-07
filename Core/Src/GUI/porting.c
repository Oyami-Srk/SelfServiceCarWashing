/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: porting.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

/* Porting function */
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

uint8_t login(const char *username, const char *password) {

}

float getCurrentFlow() {

} // in ml/s

float getCurrentUsage() {
}

int getCurrentUsingTime() { }

void logout() {

}

uint16_t get_qr_code(uint8_t **pArray) {

}
/* End of Porting function */
