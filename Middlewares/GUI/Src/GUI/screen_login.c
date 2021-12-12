#include "./callbacks.h"
#include "./screens.h"
#include "./vars.h"
#include "GUI/lvgl.h"
#include "GUI/porting.h"

lv_canvas_t *draw_qr_code(uint16_t width, uint16_t height, uint8_t *qr_array,
                          uint16_t size) {
    if (!qr_array)
        return NULL;
    if (width > QR_CANVAS_SIZE_W || height > QR_CANVAS_SIZE_H) {
        LV_LOG_ERROR("QR Code Canvas size bigger than buffer size.");
        return NULL;
    }
    uint8_t *parray      = qr_array;
    uint8_t  matrix_size = *parray;
    parray++;
    size--;
    // calculate size of matrix element
    uint16_t     dotsize = (width > height ? height : width) / matrix_size;
    lv_canvas_t *canvas  = lv_canvas_create(lv_scr_act());
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(canvas, matrix_size * dotsize, matrix_size * dotsize);
    lv_canvas_set_buffer(canvas, qr_canvas_buf, QR_CANVAS_SIZE_W,
                         QR_CANVAS_SIZE_H, LV_IMG_CF_TRUE_COLOR);
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
            if (((c >> (7 - i)) & 0x01) == 1)
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

    textarea_username = lv_textarea_create(text_area);
    lv_textarea_set_placeholder_text(textarea_username, "Username");
    lv_textarea_set_one_line(textarea_username, 1);
    lv_obj_set_size(textarea_username, 250, 50);
    lv_obj_align(textarea_username, LV_ALIGN_TOP_MID, 0, 5);

    textarea_password = lv_textarea_create(text_area);
    lv_textarea_set_placeholder_text(textarea_password, "Password");
    lv_textarea_set_one_line(textarea_password, 1);
    lv_obj_set_size(textarea_password, 250, 50);
    lv_obj_align(textarea_password, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_textarea_set_password_mode(textarea_password, 1);
    textareas_grp = lv_group_create();
    lv_group_add_obj(textareas_grp, textarea_username);
    lv_group_add_obj(textareas_grp, textarea_password);

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
    lv_obj_add_event_cb(btn_login, callback_btn_login, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn_clear, callback_btn_clear, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(btn_info, callback_btn_info, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(textarea_username, callback_show_kbd, LV_EVENT_ALL,
                        NULL);
    lv_obj_add_event_cb(textarea_password, callback_show_kbd, LV_EVENT_ALL,
                        NULL);

    // right part, login QR Code areas
    lv_obj_t *qrcode_area = lv_obj_create(scr);
    lv_obj_set_size(qrcode_area, 300, 300);
    lv_obj_align(qrcode_area, LV_ALIGN_CENTER, 180, 0);
    lv_obj_set_style_pad_all(qrcode_area, 0, LV_STATE_DEFAULT);

    uint8_t     *qarray;
    uint16_t     qarray_size = get_qr_code(&qarray);
    lv_canvas_t *canvas      = draw_qr_code(300, 300, qarray, qarray_size);
    if (canvas)
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
    lv_obj_add_event_cb(kbd, callback_kbd, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);
}
