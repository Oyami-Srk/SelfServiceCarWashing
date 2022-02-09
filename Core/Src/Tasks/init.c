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

#include "Common/init.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "task.h"

extern void standby_task(); // stanby.c

void TASKS_INIT_RTOS() {
    LOG("[INIT] Starting init procedures.");
    LCD_INIT_RTOS();
    NET_UART_INIT_RTOS();
    // Start Stand-by task
    xTaskCreate(standby_task, "STANDBY", 256, NULL, tskIDLE_PRIORITY, NULL);
}