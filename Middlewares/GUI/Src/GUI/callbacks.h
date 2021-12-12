#ifndef __S_GUI_CALLBACKS_H__
#define __S_GUI_CALLBACKS_H__

#include "GUI/lvgl.h"

// timer callback
void timer_update_data(__attribute__((unused)) lv_timer_t *timer);
void timer_for_clock(__attribute__((unused)) lv_timer_t *timer);

// event callback
void callback_btn_login(lv_event_t *event);
void callback_btn_clear(lv_event_t *event);
void callback_btn_info(lv_event_t *event);
void callback_show_kbd(lv_event_t *event);
void callback_kbd(lv_event_t *event);
void callback_logout(lv_event_t *event);

#endif // __S_GUI_CALLBACKS_H__