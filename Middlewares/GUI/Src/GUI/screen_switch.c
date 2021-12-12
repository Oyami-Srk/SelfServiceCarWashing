#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "./screens.h"
#include "./vars.h"

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

void switch_to_login_scr() {
    lv_timer_pause(update_timer);
    curr_clock_label = clock_label_login;
    lv_timer_resume(timer_clock);
    lv_timer_ready(timer_clock);
    if (btn_login) {
        lv_obj_clear_state(btn_login, LV_STATE_DISABLED);
    }
    if (textarea_username) {
        if (strlen(lv_textarea_get_text(textarea_username)) != 0)
            lv_textarea_set_text(textarea_username, "");
    }
    if (textarea_password) {
        if (strlen(lv_textarea_get_text(textarea_password)) != 0)
            lv_textarea_set_text(textarea_password, "");
    }

    lv_group_remove_obj(textarea_password);
    lv_group_remove_obj(textarea_username);
    lv_group_focus_freeze(textareas_grp, 1);
    lv_group_add_obj(textareas_grp, textarea_username);
    lv_group_add_obj(textareas_grp, textarea_password);
    lv_group_focus_freeze(textareas_grp, 0);

    lv_scr_load_anim(screens[SCR_LOGIN], LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000,
                     0);
}

void switch_to_loading_scr() {
    lv_timer_pause(update_timer);
    lv_timer_pause(timer_clock);
    lv_scr_load_anim(screens[SCR_LOADING], LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000,
                     0);
}
