#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "./callbacks.h"
#include "./screens.h"
#include "./vars.h"
#include "GUI/handles.h"
#include "GUI/porting.h"

#if GUI_SIMULATING == 1
/* simulating function */
void simulate_login_switch(__attribute__((unused)) lv_timer_t *timer) {
    //    switch_to_inuse_scr("TestUser");
    switch_to_login_scr();
}

void simulate_loading(__attribute__((unused)) lv_timer_t *timer) {
    //    lv_scr_load(screens[SCR_LOGIN]);
    switch_to_loading_scr();

    lv_timer_t *oneshot = lv_timer_create(simulate_login_switch, 3000, NULL);
    lv_timer_set_repeat_count(oneshot, 1);
}
/* simulating function */
#endif

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

#if GUI_SIMULATING == 1
    // simulate loading
    lv_timer_t *oneshot = lv_timer_create(simulate_loading, 1000, NULL);
    lv_timer_set_repeat_count(oneshot, 1);
#else
    switch_to_loading_scr(); // wait switching on other thread.
#endif
}
