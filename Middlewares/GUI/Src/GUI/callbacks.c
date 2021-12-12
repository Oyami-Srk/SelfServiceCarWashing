#include "./callbacks.h"
#include "./vars.h"
#include "GUI/porting.h"
#include <stdio.h>

void timer_for_clock(__attribute__((unused)) lv_timer_t *timer) {
    if (!curr_clock_label)
        return;
    char buffer[32];
    get_time_str(buffer, 1);
    lv_label_set_text((lv_obj_t *)curr_clock_label, buffer);
}

void callback_btn_login(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t       *src  = lv_event_get_target(event);

    if (code == LV_EVENT_CLICKED) {
        const char *username = lv_textarea_get_text(textarea_username);
        const char *password = lv_textarea_get_text(textarea_password);
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
    lv_obj_t       *src  = lv_event_get_target(event);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_state(src, LV_STATE_DISABLED);
        lv_textarea_set_text(textarea_username, "");
        lv_textarea_set_text(textarea_password, "");
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
                if (lv_keyboard_get_textarea(kbd) == textarea_username) {
                    lv_group_focus_next(textareas_grp);
                } else {
                    lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
                    lv_event_send(btn_login, LV_EVENT_CLICKED, NULL);
                }
            } else if (id == 14) {
                lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
                lv_textarea_del_char(textarea);
            } else {
            }
        }
    }
}

void timer_update_data(__attribute__((unused)) lv_timer_t *timer) {
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
