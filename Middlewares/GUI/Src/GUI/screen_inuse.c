#include "./callbacks.h"
#include "./screens.h"
#include "./vars.h"
#include "GUI/lvgl.h"

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
