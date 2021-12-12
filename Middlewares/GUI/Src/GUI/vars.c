#include "./vars.h"
#include "./screens.h"
#include "GUI/config.h"

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

lv_textarea_t *textarea_username, *textarea_password;
lv_group_t    *textareas_grp;
lv_obj_t      *btn_login;

#ifdef QR_CANVAS_USE_STATIC_ALLOC
lv_color_t qr_canvas_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(QR_CANVAS_SIZE_W,
                                                       QR_CANVAS_SIZE_H)];
#endif
