#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "./screens.h"
#include "./vars.h"

void draw_scr_loading(lv_obj_t *scr) {
    lv_obj_t *spinner = lv_spinner_create(scr, 3000, 60);
    lv_obj_set_size(spinner, 300, 300);
    lv_obj_center(spinner);
}
