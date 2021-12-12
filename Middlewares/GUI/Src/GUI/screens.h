#ifndef __S_GUI_SCREENS_H__
#define __S_GUI_SCREENS_H__

#include "GUI/lvgl.h"

#define SCREEN_COUNT 3
#define SCR_LOGIN    0
#define SCR_USING    1
#define SCR_LOADING  2

void draw_scr_login(lv_obj_t *scr);
void draw_scr_inuse(lv_obj_t *scr);
void draw_scr_loading(lv_obj_t *scr);

#endif // __S_GUI_SCREENS_H__