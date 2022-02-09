/*
 * Copyright (c) 2021 - 2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: standby.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/utils.h"
#include "FreeRTOS.h"
#include "Net/at.h"

_Noreturn void standby_task() {
    LOG("[STANDBY] Start stand-by task.");
    LOG("[STANDBY] Wait for network ready.");
    AT_WaitForStatus(NET_CONNECTED_TO_SERVER, portMAX_DELAY);

    for (;;)
        ;
}