/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

//
// Created by shiroko on 2022/2/1.
//

#include "Common/init.h"

void TASKS_INIT_RTOS() {
    LCD_INIT_RTOS();
    NET_UART_INIT_RTOS();
}