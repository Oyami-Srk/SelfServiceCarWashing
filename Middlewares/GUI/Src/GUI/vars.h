#ifndef __S_GUI_VARS_H__
#define __S_GUI_VARS_H__

#include "./screens.h"
#include "GUI/config.h"
#include "GUI/lvgl.h"

extern lv_obj_t      *screens[SCREEN_COUNT];
extern lv_timer_t    *timer_clock;
extern lv_label_t    *clock_label_login;
extern lv_label_t    *clock_label_inuse;
extern lv_label_t    *curr_clock_label;
extern const char    *long_message;
extern lv_keyboard_t *kbd;
extern lv_label_t    *user_label;
extern lv_label_t    *currentAmount;
extern lv_label_t    *currentFlow;
extern lv_label_t    *currentTime;
extern lv_timer_t    *update_timer;

extern lv_textarea_t *textarea_username, *textarea_password;
extern lv_group_t    *textareas_grp;
extern lv_obj_t      *btn_login;

#ifdef QR_CANVAS_USE_STATIC_ALLOC
extern lv_color_t qr_canvas_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(300, 300)];
#else
extern lv_color_t *qr_canvas_buf;
#endif

#endif // __S_GUI_VARS_H__