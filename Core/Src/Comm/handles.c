/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: handles.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Net/AT/command.h"
#include "Net/config.h"
#include "Net/vars.h"
#include "cmsis_os2.h"
#include "main.h"
#include <stdio.h>

const osThreadAttr_t task_comm_attributes = {
    .name       = "comm",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityAboveNormal,
};

_Noreturn void task_comm(void *argument);

osThreadId_t pid_comm;

void COMM_INIT_FREERTOS() {
    printf("[MODULE] Comm Initializing.\r\n");
    pid_comm = osThreadNew(task_comm, NULL, &task_comm_attributes);
    printf("[MODULE] Comm Initialization completed.\r\n");
}

_Noreturn void task_comm(void *argument) {
    while (NET_STATUS != NET_STATUS_LINK_ESTABLISHED) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // wait 1 sec for link established
    }

    for (;;) {
    }
}
