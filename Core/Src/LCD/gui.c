/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: gui.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "lvgl.h"
#define SCREEN_COUNT 3
#define SCR_LOGIN    0
#define SCR_USING    1
#define SCR_LOADING  2

/* Porting function */
#include "main.h"
#include "rtc.h"
#include "time.h"
#include <stdio.h>

#define LVGL_DRAW_BUFFER_SIZE LCD_WIDTH * 200
static lv_color_t *cbuf =
    LCD_END_ADDR + 2 * (LVGL_DRAW_BUFFER_SIZE * sizeof(lv_color_t));

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

uint8_t login(const char *username, const char *password) {
    printf("Tring to login with %s and password %s.\n", username, password);
    switch_to_inuse_scr(username);
    return LOGIN_SUCCESS;
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

static uint8_t qr_array[] = {
    0x39, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0xFE, 0x2F, 0x4C, 0x84, 0xBC, 0xBF,
    0x80, 0x41, 0x50, 0x46, 0x28, 0x93, 0xD0, 0x40, 0x2E, 0xB4, 0xA5, 0x6,
    0x1D, 0x6B, 0xA0, 0x17, 0x56, 0x50, 0x7F, 0x16, 0x25, 0xD0, 0xB,  0xAF,
    0xEE, 0x7F, 0xF,  0x82, 0xE8, 0x4,  0x14, 0x1C, 0xF1, 0xE1, 0xD1, 0x4,
    0x3,  0xFA, 0xAA, 0xAA, 0xAA, 0xAA, 0xFE, 0x0,  0x0,  0xEE, 0x64, 0x6F,
    0xA,  0x0,  0x0,  0x27, 0xE7, 0x1B, 0xF3, 0x64, 0x5F, 0x0,  0x50, 0x2F,
    0xC,  0x53, 0x26, 0xAC, 0x40, 0x1B, 0xB5, 0x58, 0x6E, 0x59, 0x77, 0xA0,
    0x1F, 0x3B, 0xE1, 0x92, 0x43, 0xC1, 0x80, 0x7,  0x3C, 0x57, 0xF8, 0xD4,
    0xC0, 0xA0, 0x5,  0x25, 0xA5, 0xC8, 0x76, 0x2A, 0xE8, 0x0,  0xB9, 0x4,
    0x5A, 0x95, 0x77, 0x7A, 0x0,  0xA2, 0x4,  0x4C, 0x99, 0x1C, 0x52, 0x0,
    0xAB, 0xB3, 0xE,  0x25, 0xBD, 0x6D, 0x80, 0x3E, 0x2E, 0xFD, 0xFC, 0x8,
    0x21, 0x40, 0x30, 0x9F, 0x2B, 0x56, 0x6B, 0x97, 0xA0, 0x7,  0xBA, 0x36,
    0x52, 0x8B, 0xAD, 0x80, 0xA,  0xFE, 0xF5, 0x68, 0x8E, 0x96, 0xD0, 0x1,
    0x9,  0x0,  0xF3, 0xBC, 0xEA, 0x10, 0x2,  0x3E, 0x5,  0x1F, 0xA5, 0x13,
    0xEA, 0x1,  0x91, 0x51, 0xDC, 0x64, 0xF1, 0x1A, 0x0,  0xAA, 0xC9, 0xA2,
    0xAE, 0x69, 0xA8, 0x0,  0x54, 0x56, 0xCF, 0x10, 0x6A, 0xC4, 0xC0, 0x17,
    0xFD, 0x56, 0xFD, 0x91, 0x3F, 0x20, 0x10, 0x29, 0x64, 0x29, 0xD1, 0x37,
    0x30, 0x5,  0x74, 0x8C, 0x41, 0xCE, 0xFD, 0x58, 0x2,  0xE0, 0x26, 0x51,
    0x7A, 0x6A, 0xA4, 0x0,  0x8C, 0xA7, 0x48, 0xE2, 0xF6, 0x96, 0x0,  0x2B,
    0xB,  0x6F, 0xFE, 0x27, 0xC1, 0x0,  0xCA, 0xAA, 0x35, 0x13, 0x9B, 0x85,
    0x80, 0x60, 0xD2, 0x49, 0xC4, 0xAA, 0x7A, 0x40, 0x6,  0xA9, 0xCF, 0x70,
    0xF7, 0x6A, 0x20, 0x19, 0x3,  0x9B, 0x39, 0x1E, 0xA4, 0xA0, 0x6,  0xBB,
    0xA9, 0x18, 0x88, 0x92, 0xC8, 0x1,  0x4F, 0xAA, 0x9B, 0x36, 0xA,  0x88,
    0x1,  0x1E, 0x22, 0x86, 0xE5, 0x76, 0xBA, 0x0,  0xE3, 0xC5, 0x34, 0x9F,
    0xDE, 0xF8, 0x0,  0xE2, 0x1C, 0x33, 0xF2, 0xCD, 0xF9, 0x80, 0x0,  0x40,
    0x7B, 0x1D, 0xBF, 0xC6, 0x40, 0x3F, 0xB4, 0x53, 0xAA, 0x7F, 0xAA, 0xA0,
    0x10, 0x50, 0xC8, 0xC6, 0x11, 0xF1, 0x80, 0xB,  0xA0, 0xF8, 0xFE, 0xEE,
    0x8F, 0xC8, 0x5,  0xD3, 0x6F, 0x86, 0x2E, 0x6D, 0x0,  0x2,  0xEA, 0x85,
    0xEB, 0x36, 0x23, 0xA2, 0x1,  0x4,  0x3C, 0xFB, 0x89, 0x93, 0x80, 0x0,
    0xFE, 0x4A, 0x70, 0x1B, 0xAA, 0x5C, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0};

/* End of Porting function */

lv_obj_t   *screens[SCREEN_COUNT];
lv_timer_t *timer_clock;
lv_label_t *clock_label_login;
lv_label_t *clock_label_inuse;
lv_label_t *curr_clock_label;
const char *long_message =
    "This is a very long story from the star we call polaris. And telling this "
    "story would make this message very very very long...";
lv_keyboard_t *kbd;
lv_label_t    *user_label;

lv_label_t *currentAmount;
lv_label_t *currentFlow;
lv_label_t *currentTime;
lv_timer_t *update_timer;
void        timer_update_data(lv_timer_t *timer);

void draw_scr_login(lv_obj_t *scr);
void draw_scr_inuse(lv_obj_t *scr);
void draw_scr_loading(lv_obj_t *scr);
void timer_for_clock(lv_timer_t *timer);

void switch_to_inuse_scr(const char *user) {
    lv_timer_resume(update_timer);
    curr_clock_label = clock_label_inuse;
    lv_timer_resume(timer_clock);
    lv_timer_ready(timer_clock);
    lv_scr_load_anim(screens[SCR_USING], LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000,
                     0);
    if (user_label) {
        lv_label_set_text(user_label, user);
    }
}

typedef struct {
    lv_textarea_t *username, *password;
    lv_group_t    *textgrp;
    lv_obj_t      *login_btn;
} DataInputArea;

lv_btn_t     *btn_login = NULL;
DataInputArea dia;

void switch_to_login_scr() {
    lv_timer_pause(update_timer);
    curr_clock_label = clock_label_login;
    lv_timer_resume(timer_clock);
    lv_timer_ready(timer_clock);
    if (btn_login) {
        lv_obj_clear_state(btn_login, LV_STATE_DISABLED);
    }
    if (dia.username) {
        if (strlen(lv_textarea_get_text(dia.username)) != 0)
            lv_textarea_set_text(dia.username, "");
    }
    if (dia.password) {
        if (strlen(lv_textarea_get_text(dia.password)) != 0)
            lv_textarea_set_text(dia.password, "");
    }

    lv_group_remove_obj(dia.password);
    lv_group_remove_obj(dia.username);
    lv_group_focus_freeze(dia.textgrp, 1);
    lv_group_add_obj(dia.textgrp, dia.username);
    lv_group_add_obj(dia.textgrp, dia.password);
    lv_group_focus_freeze(dia.textgrp, 0);

    lv_scr_load_anim(screens[SCR_LOGIN], LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000,
                     0);
}

void switch_to_loading_scr() {
    lv_timer_pause(update_timer);
    lv_timer_pause(timer_clock);
    lv_scr_load_anim(screens[SCR_LOADING], LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000,
                     0);
}

lv_canvas_t *draw_qr_code(uint16_t width, uint16_t height, uint8_t *qr_array,
                          uint16_t size) {
    uint8_t *parray      = qr_array;
    uint8_t  matrix_size = *parray;
    parray++;
    size--;
    // calculate size of matrix element
    uint16_t     dotsize = (width > height ? height : width) / matrix_size;
    lv_canvas_t *canvas  = lv_canvas_create(lv_scr_act());
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(canvas, matrix_size * dotsize, matrix_size * dotsize);
    lv_canvas_set_buffer(canvas, cbuf, 300, 300, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, lv_color_white(), 0xFF);

    // dot desc
    static lv_draw_rect_dsc_t dot_dsc;
    lv_draw_rect_dsc_init(&dot_dsc);
    dot_dsc.radius   = 0;
    dot_dsc.bg_color = lv_color_black();

    // draw
    int x = 0, y = 0;
    for (int j = 0; j < size * size; j++) {
        uint8_t c = parray[j];
        for (int i = 0; i < 8; i++) {
            if ((c >> (7 - i)) & 0x01 == 1)
                lv_canvas_draw_rect(canvas, x * dotsize, y * dotsize, dotsize,
                                    dotsize, &dot_dsc);
            x++;
            if (x >= matrix_size) {
                x = 0;
                y++;
            }
            if (y >= matrix_size) {
                break;
            }
        }
    }
    return canvas;
}

void simulate_login_switch(lv_timer_t *timer) {
    //    switch_to_inuse_scr("TestUser");
    switch_to_login_scr();
}

void simulate_loading(lv_timer_t *timer) {
    //    lv_scr_load(screens[SCR_LOGIN]);
    switch_to_loading_scr();

    lv_timer_t *oneshot = lv_timer_create(simulate_login_switch, 3000, NULL);
    lv_timer_set_repeat_count(oneshot, 1);
}

void draw_widget() {
    for (int i = 0; i < SCREEN_COUNT; i++)
        screens[i] = lv_obj_create(NULL);
    draw_scr_login(screens[SCR_LOGIN]);
    draw_scr_loading(screens[SCR_LOADING]);
    draw_scr_inuse(screens[SCR_USING]);

    if (timer_clock) {
        lv_timer_del(timer_clock);
        timer_clock = NULL;
    }
    timer_clock = lv_timer_create(timer_for_clock, 1000, NULL);
    lv_timer_pause(timer_clock);

    if (update_timer) {
        lv_timer_del(update_timer);
        update_timer = NULL;
    }
    update_timer =
        lv_timer_create(timer_update_data, 1000, NULL); // sample rate 1s
    lv_timer_pause(update_timer);

    // simulate loading
    lv_timer_t *oneshot = lv_timer_create(simulate_loading, 1000, NULL);
    lv_timer_set_repeat_count(oneshot, 1);
}

void callback_btn_login(lv_event_t *event);
void callback_btn_clear(lv_event_t *event);
void callback_btn_info(lv_event_t *event);
void callback_show_kbd(lv_event_t *event);
void callback_kbd(lv_event_t *event);

void draw_scr_login(lv_obj_t *scr) {
    // Initialize title bar
    lv_obj_t *title_bar = lv_obj_create(scr);
    lv_obj_set_size(title_bar, 800, 40);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    static lv_style_t style_title_bar;
    lv_style_init(&style_title_bar);
    lv_style_set_bg_color(&style_title_bar, lv_color_make(100, 100, 100));
    lv_style_set_radius(&style_title_bar, 0);
    lv_style_set_border_width(&style_title_bar, 0);
    lv_style_set_pad_all(&style_title_bar, 0);
    lv_obj_add_style(title_bar, &style_title_bar, LV_STATE_DEFAULT);

    static lv_style_t style_clock_title;
    lv_style_init(&style_clock_title);
    lv_style_set_text_color(&style_clock_title, lv_color_white());
    lv_style_set_pad_all(&style_clock_title, 0);
    lv_style_set_max_height(&style_clock_title, 40);
    lv_style_set_align(&style_clock_title, LV_ALIGN_CENTER);
    lv_style_set_text_font(&style_clock_title, &lv_font_montserrat_24);
    lv_style_set_text_align(&style_title_bar, LV_TEXT_ALIGN_CENTER);
    lv_style_set_height(&style_title_bar, 40);

    lv_label_t *clock_label = lv_label_create(title_bar);
    lv_obj_add_style(clock_label, &style_clock_title, LV_STATE_DEFAULT);
    lv_label_t *network_status = lv_label_create(title_bar);
    lv_obj_add_style(network_status, &style_clock_title, LV_STATE_DEFAULT);
    lv_obj_set_style_align(network_status, LV_ALIGN_RIGHT_MID,
                           LV_STATE_DEFAULT);
    lv_label_set_text(network_status, LV_SYMBOL_WIFI "  ");
    clock_label_login = clock_label;

    lv_obj_t *bottom_bar = lv_obj_create(scr);
    lv_obj_set_size(bottom_bar, 800, 40);
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(bottom_bar, &style_title_bar, LV_STATE_DEFAULT);
    lv_label_t *long_msg = lv_label_create(bottom_bar);
    lv_label_set_long_mode(long_msg, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(long_msg, long_message);
    lv_obj_add_style(long_msg, &style_clock_title, LV_STATE_DEFAULT);
    lv_obj_set_size(long_msg, 800, 40);
    lv_obj_set_style_align(long_msg, LV_ALIGN_LEFT_MID, LV_STATE_DEFAULT);

    // remain vertical size: 400 px

    // left part, user login text areas
    lv_obj_t *login_area = lv_obj_create(scr);
    lv_obj_set_size(login_area, 300, 300);
    lv_obj_align(login_area, LV_ALIGN_CENTER, -180, 0);
    lv_obj_set_style_pad_all(login_area, 0, LV_STATE_DEFAULT);

    lv_obj_t *text_area = lv_obj_create(login_area);
    lv_obj_set_size(text_area, 280, 120);
    lv_obj_align(text_area, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_pad_all(text_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(text_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(text_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(text_area, 0, LV_STATE_DEFAULT);

    lv_textarea_t *username = lv_textarea_create(text_area);
    lv_textarea_set_placeholder_text(username, "Username");
    lv_textarea_set_one_line(username, 1);
    lv_obj_set_size(username, 250, 50);
    lv_obj_align(username, LV_ALIGN_TOP_MID, 0, 5);

    lv_textarea_t *password = lv_textarea_create(text_area);
    lv_textarea_set_placeholder_text(password, "Password");
    lv_textarea_set_one_line(password, 1);
    lv_obj_set_size(password, 250, 50);
    lv_obj_align(password, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_textarea_set_password_mode(password, 1);
    dia.username        = username;
    dia.password        = password;
    lv_group_t *textgrp = lv_group_create();
    lv_group_add_obj(textgrp, username);
    lv_group_add_obj(textgrp, password);
    dia.textgrp = textgrp;

    lv_obj_t *button_area = lv_obj_create(login_area);
    lv_obj_set_size(button_area, 280, 130);
    lv_obj_align(button_area, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_pad_all(button_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(button_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(button_area, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(button_area, 0, LV_STATE_DEFAULT);

    btn_login = lv_btn_create(button_area);
    lv_obj_align(btn_login, LV_ALIGN_TOP_MID, 0, 10);
    lv_label_t *btn_login_label = lv_label_create(btn_login);
    lv_label_set_text(btn_login_label, "LOGIN");
    lv_obj_set_align(btn_login_label, LV_ALIGN_CENTER);
    lv_obj_set_size(btn_login, 250, 50);
    dia.login_btn = (lv_obj_t *)btn_login;

    lv_btn_t *btn_clear = lv_btn_create(button_area);
    lv_obj_align(btn_clear, LV_ALIGN_BOTTOM_LEFT, 13, -10);
    lv_label_t *btn_clear_label = lv_label_create(btn_clear);
    lv_label_set_text(btn_clear_label, "CLEAR");
    lv_obj_set_align(btn_clear_label, LV_ALIGN_CENTER);
    lv_obj_set_size(btn_clear, 120, 50);

    lv_btn_t *btn_info = lv_btn_create(button_area);
    lv_obj_align(btn_info, LV_ALIGN_BOTTOM_RIGHT, -13, -10);
    lv_label_t *btn_info_label = lv_label_create(btn_info);
    lv_label_set_text(btn_info_label, "INFO");
    lv_obj_set_align(btn_info_label, LV_ALIGN_CENTER);
    lv_obj_set_size(btn_info, 120, 50);
    lv_obj_add_event_cb(btn_login, callback_btn_login, LV_EVENT_ALL, &dia);
    lv_obj_add_event_cb(btn_clear, callback_btn_clear, LV_EVENT_ALL, &dia);
    lv_obj_add_event_cb(btn_info, callback_btn_info, LV_EVENT_ALL, &dia);
    lv_obj_add_event_cb(username, callback_show_kbd, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(password, callback_show_kbd, LV_EVENT_ALL, NULL);

    // right part, login QR Code areas
    lv_obj_t *qrcode_area = lv_obj_create(scr);
    lv_obj_set_size(qrcode_area, 300, 300);
    lv_obj_align(qrcode_area, LV_ALIGN_CENTER, 180, 0);
    lv_obj_set_style_pad_all(qrcode_area, 0, LV_STATE_DEFAULT);

    lv_canvas_t *canvas = draw_qr_code(300, 300, qr_array, 408);
    lv_obj_set_parent(canvas, qrcode_area);

    // init kbd
    static const char               *kbd_map[]      = {"1",
                                    "2",
                                    "3",
                                    LV_SYMBOL_BACKSPACE,
                                    "\n",
                                    "4",
                                    "5",
                                    "6",
                                    LV_SYMBOL_LEFT,
                                    "\n",
                                    "7",
                                    "8",
                                    "9",
                                    LV_SYMBOL_RIGHT,
                                    "\n",
                                    "0",
                                    LV_SYMBOL_OK,
                                    LV_SYMBOL_DOWN,
                                    ""};
    static const lv_btnmatrix_ctrl_t kbd_ctrl_map[] = {
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        0,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        0,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        0,
        LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT,
        0,
        0};
    kbd = lv_keyboard_create(scr);
    lv_keyboard_set_map(kbd, LV_KEYBOARD_MODE_NUMBER, kbd_map, kbd_ctrl_map);
    lv_keyboard_set_mode(kbd, LV_KEYBOARD_MODE_NUMBER);
    lv_btnmatrix_set_btn_width(kbd, 12, 2);
    lv_obj_add_event_cb(kbd, callback_kbd, LV_EVENT_ALL, &dia);
    lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
}

void timer_for_clock(lv_timer_t *timer) {
    if (!curr_clock_label)
        return;
    char buffer[32];
    get_time_str(buffer, 1);
    lv_label_set_text((lv_obj_t *)curr_clock_label, buffer);
}

void callback_btn_login(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    DataInputArea  *dia  = lv_event_get_user_data(event);
    lv_obj_t       *src  = lv_event_get_target(event);

    if (code == LV_EVENT_CLICKED) {
        const char *username = lv_textarea_get_text(dia->username);
        const char *password = lv_textarea_get_text(dia->password);
        if (strlen(username) == 0 || strlen(password) == 0)
            return;
        lv_obj_add_state(src, LV_STATE_DISABLED);
        uint8_t result = login(username, password);
        if (result == LOGIN_SUCCESS) {
            // switch screen
        } else if (result == LOGIN_FAILED) {
            // try again
            lv_obj_clear_state(src, LV_STATE_DISABLED);
        }
    }
}

void callback_btn_clear(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    DataInputArea  *dia  = lv_event_get_user_data(event);
    lv_obj_t       *src  = lv_event_get_target(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_state(src, LV_STATE_DISABLED);
        lv_textarea_set_text(dia->username, "");
        lv_textarea_set_text(dia->password, "");
        lv_obj_clear_state(src, LV_STATE_DISABLED);
    }
}

void callback_info_msgbox(lv_event_t *event) {
    lv_msgbox_t *msgbox      = lv_event_get_user_data(event);
    const char  *btn_clicked = lv_msgbox_get_active_btn_text(msgbox);

    if (strcmp("OK", btn_clicked) == 0) {
        lv_msgbox_close_async(msgbox);
    }
}

void callback_btn_info(lv_event_t *event) {
    lv_event_code_t    code   = lv_event_get_code(event);
    static const char *btns[] = {"OK", ""};
    if (code == LV_EVENT_CLICKED) {
        lv_msgbox_t *msgbox = lv_msgbox_create(
            NULL, "Info", "This is info message box.", btns, 1);
        lv_obj_add_event_cb(msgbox, callback_info_msgbox,
                            LV_EVENT_VALUE_CHANGED, msgbox);
        lv_obj_center(msgbox);
    }
}

void callback_show_kbd(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_FOCUSED) {
        lv_obj_clear_flag(kbd, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kbd, lv_event_get_target(event));
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
    }
}

void callback_kbd(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t       *obj  = lv_event_get_target(event);

    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(event);
        if (dsc->id == 3 || dsc->id == 7 || dsc->id == 11 || dsc->id == 13 ||
            dsc->id == 14) {
            if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
                dsc->rect_dsc->bg_color = lv_palette_darken(LV_PALETTE_BLUE, 2);
            else
                dsc->rect_dsc->bg_color = lv_palette_main(LV_PALETTE_BLUE);
        }
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        lv_textarea_t *textarea = lv_keyboard_get_textarea(obj);
        if (textarea) {
            uint16_t id = *((uint16_t *)lv_event_get_param(event));
            if (id == 3) {
            } else if (id == 7) {
            } else if (id == 11) {
            } else if (id == 13) {
                DataInputArea *dia = lv_event_get_user_data(event);
                if (lv_keyboard_get_textarea(kbd) == dia->username) {
                    //                    lv_keyboard_set_textarea(kbd,
                    //                    dia->password);
                    lv_group_focus_next(dia->textgrp);
                } else {
                    lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
                    lv_event_send(dia->login_btn, LV_EVENT_CLICKED, NULL);
                }
            } else if (id == 14) {
                lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
                lv_textarea_del_char(textarea);
            } else {
            }
        }
    }
}

void timer_update_data(lv_timer_t *timer) {
    char buffer[32];
    if (currentAmount) {
        sprintf(buffer, "%lf", getCurrentUsage());
        lv_label_set_text(currentAmount, buffer);
    }
    if (currentFlow) {
        sprintf(buffer, "%lf", getCurrentFlow());
        lv_label_set_text(currentFlow, buffer);
    }
    if (currentTime) {
        sprintf(buffer, "%d", getCurrentUsingTime());
        lv_label_set_text(currentTime, buffer);
    }
}

void callback_logout(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_CLICKED) {
        logout();
    }
}

void draw_scr_inuse(lv_obj_t *scr) {
    // Initialize title bar
    lv_obj_t *title_bar = lv_obj_create(scr);
    lv_obj_set_size(title_bar, 800, 40);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    static lv_style_t style_title_bar;
    lv_style_init(&style_title_bar);
    lv_style_set_bg_color(&style_title_bar, lv_color_make(100, 100, 100));
    lv_style_set_radius(&style_title_bar, 0);
    lv_style_set_border_width(&style_title_bar, 0);
    lv_style_set_pad_all(&style_title_bar, 0);
    lv_obj_add_style(title_bar, &style_title_bar, LV_STATE_DEFAULT);

    static lv_style_t style_clock_title;
    lv_style_init(&style_clock_title);
    lv_style_set_text_color(&style_clock_title, lv_color_white());
    lv_style_set_pad_all(&style_clock_title, 0);
    lv_style_set_max_height(&style_clock_title, 40);
    lv_style_set_align(&style_clock_title, LV_ALIGN_CENTER);
    lv_style_set_text_font(&style_clock_title, &lv_font_montserrat_24);
    lv_style_set_text_align(&style_title_bar, LV_TEXT_ALIGN_CENTER);
    lv_style_set_height(&style_title_bar, 40);

    lv_label_t *clock_label = lv_label_create(title_bar);
    lv_obj_add_style(clock_label, &style_clock_title, LV_STATE_DEFAULT);
    lv_label_t *network_status = lv_label_create(title_bar);
    lv_obj_add_style(network_status, &style_clock_title, LV_STATE_DEFAULT);
    lv_obj_set_style_align(network_status, LV_ALIGN_RIGHT_MID,
                           LV_STATE_DEFAULT);
    lv_label_set_text(network_status, LV_SYMBOL_WIFI "  ");
    clock_label_inuse = clock_label;

    lv_obj_t *bottom_bar = lv_obj_create(scr);
    lv_obj_set_size(bottom_bar, 800, 40);
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(bottom_bar, &style_title_bar, LV_STATE_DEFAULT);
    lv_label_t *long_msg = lv_label_create(bottom_bar);
    lv_label_set_long_mode(long_msg, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(long_msg, long_message);
    lv_obj_add_style(long_msg, &style_clock_title, LV_STATE_DEFAULT);
    lv_obj_set_size(long_msg, 800, 40);
    lv_obj_set_style_align(long_msg, LV_ALIGN_LEFT_MID, LV_STATE_DEFAULT);

    // remain vertical size: 400 px
    // draw main card
    lv_obj_t *main_card = lv_obj_create(scr);
    lv_obj_set_style_pad_all(main_card, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(main_card, 760, 360);
    lv_obj_center(main_card);
    static lv_style_t main_card_style;
    lv_style_init(&main_card_style);
    lv_style_set_bg_color(&main_card_style,
                          lv_palette_lighten(LV_PALETTE_GREY, 2));
    lv_style_set_border_width(&main_card_style, 2);
    lv_style_set_border_color(&main_card_style,
                              lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_pad_all(&main_card_style, 0);
    lv_style_set_pad_gap(&main_card_style, 0);
    /*
    lv_style_set_shadow_width(&main_card_style, 5);
    lv_style_set_shadow_ofs_x(&main_card_style, 10);
    lv_style_set_shadow_ofs_y(&main_card_style, 10);
    lv_style_set_shadow_color(&main_card_style,
                              lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_spread(&main_card_style, 5);
    lv_style_set_shadow_opa(&main_card_style, 100);
     */
    lv_obj_add_style(main_card, &main_card_style, LV_STATE_DEFAULT);

    lv_obj_t *current_user_panel = lv_obj_create(main_card);
    lv_obj_set_size(current_user_panel, 750, 80);
    lv_obj_set_flex_flow(current_user_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_align(current_user_panel, LV_ALIGN_TOP_LEFT);
    lv_obj_set_style_bg_opa(current_user_panel, 0, LV_STATE_DEFAULT);
    lv_label_t *lbl1 = lv_label_create(current_user_panel);
    lv_label_set_text(lbl1, "Current User: ");

    user_label = lv_label_create(current_user_panel);
    lv_obj_align(user_label, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_text_font(user_label, &lv_font_montserrat_26,
                               LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lbl1, &lv_font_montserrat_26, LV_STATE_DEFAULT);

    // set text
    lv_obj_t *text_card = lv_obj_create(main_card);
    lv_obj_center(text_card);
    lv_obj_set_size(text_card, 700, 250);
    lv_obj_set_style_bg_opa(text_card, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(text_card, 0, LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(text_card, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *current_usage_panel = lv_obj_create(text_card);
    lv_obj_set_size(current_usage_panel, 680, 60);
    lv_obj_set_flex_flow(current_usage_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(current_usage_panel, 0, LV_STATE_DEFAULT);
    //    lv_obj_set_align(current_usage_panel, LV_ALIGN_TOP_MID);
    lv_obj_set_style_bg_opa(current_usage_panel, 0, LV_STATE_DEFAULT);
    lv_label_t *lbl2 = lv_label_create(current_usage_panel);
    lv_label_set_text(lbl2, "Usage(L): ");

    currentAmount = lv_label_create(current_usage_panel);
    lv_obj_set_style_text_font(lbl2, &lv_font_montserrat_36, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(currentAmount, &lv_font_montserrat_36,
                               LV_STATE_DEFAULT);

    lv_obj_t *current_flow_panel = lv_obj_create(text_card);
    lv_obj_set_size(current_flow_panel, 680, 60);
    lv_obj_set_flex_flow(current_flow_panel, LV_FLEX_FLOW_ROW);
    //    lv_obj_set_align(current_flow_panel, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_pad_all(current_flow_panel, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(current_flow_panel, 0, LV_STATE_DEFAULT);
    lv_label_t *lbl3 = lv_label_create(current_flow_panel);
    lv_label_set_text(lbl3, "Flow(mL/s): ");

    currentFlow = lv_label_create(current_flow_panel);
    lv_obj_set_style_text_font(lbl3, &lv_font_montserrat_36, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(currentFlow, &lv_font_montserrat_36,
                               LV_STATE_DEFAULT);

    lv_obj_t *current_time_panel = lv_obj_create(text_card);
    lv_obj_set_size(current_time_panel, 680, 60);
    lv_obj_set_flex_flow(current_time_panel, LV_FLEX_FLOW_ROW);
    //    lv_obj_set_align(current_time_panel, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_pad_all(current_time_panel, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(current_time_panel, 0, LV_STATE_DEFAULT);
    lv_label_t *lbl4 = lv_label_create(current_time_panel);
    lv_label_set_text(lbl4, "Using Time(s): ");

    currentTime = lv_label_create(current_time_panel);
    lv_obj_set_style_text_font(lbl4, &lv_font_montserrat_36, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(currentTime, &lv_font_montserrat_36,
                               LV_STATE_DEFAULT);

    // draw exit button
    lv_btn_t *logout_btn = lv_btn_create(main_card);
    lv_obj_align(logout_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_label_t *logout_lbl = lv_label_create(logout_btn);
    lv_label_set_text(logout_lbl, "LogOut");
    lv_obj_set_style_text_font(logout_lbl, &lv_font_montserrat_26,
                               LV_STATE_DEFAULT);
    lv_obj_add_event_cb(logout_btn, callback_logout, LV_EVENT_ALL, NULL);
}

void draw_scr_loading(lv_obj_t *scr) {
    lv_obj_t *spinner = lv_spinner_create(scr, 3000, 60);
    lv_obj_set_size(spinner, 300, 300);
    lv_obj_center(spinner);
}