/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/init.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "GUI/handles.h"
#include "task.h"

extern void standby_task(); // standby.c

void TASKS_INIT_RTOS() {
    LOG_SCR("[INIT] Starting init procedures.");
    set_long_message("...System starting...");
    LCD_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished LCD Init procedures.\n");
    NET_UART_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished Net UART Init procedures.\n");
    // Start Stand-by task
    xTaskCreate(standby_task, "STANDBY", 256, NULL, tskIDLE_PRIORITY, NULL);
}