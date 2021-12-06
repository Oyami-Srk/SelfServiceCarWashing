/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "main.h"

#define LCD_ADDRRESS_LAYER_0 0xD0000000
#define LCD_WIDTH            800
#define LCD_HEIGHT           480

void LCD_GPIO_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}

void LCD_DISPLAY_INIT() {
    HAL_GPIO_WritePin(LCD_BACK_LIGHT_GPIO_Port, LCD_BACK_LIGHT_Pin,
                      GPIO_PIN_SET);
}