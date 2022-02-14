/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: standby.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/config.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "GUI/handles.h"
#include "Net/at.h"
#include "Net/cmd.h"

_Noreturn void register_task() {
    for (;;) {
        LOG("[STANDBY] Wait for network ready.");
        AT_WaitForStatus(NET_CONNECTED_TO_SERVER, portMAX_DELAY);

        while (CMD_RESULT_OK != Cmd_RegisterDevice()) {
            vTaskDelay(pdMS_TO_TICKS(10 * 1000));
        }
        AT_SetNetStatus(NET_DEVICE_REGISTERED);

        loading_switch_out();
        login_switch_in();

        Cmd_AcquireQRCode();

        while (AT_GetNetStatus() == NET_DEVICE_REGISTERED) {
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        // connection broke
    }

    vTaskDelete(NULL);
}

_Noreturn void heartbeat_task() {
    LOG("[STANDBY] Wait for device registered.");
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    for (;;) {
        if (AT_GetNetStatus() == NET_DEVICE_REGISTERED)
            Cmd_HeartBeat();
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTV));
    }
    vTaskDelete(NULL);
}

_Noreturn void standby_task() {
    LOG("[STANDBY] Start stand-by task.");
    xTaskCreate(register_task, "REGISTER", 128, NULL, tskIDLE_PRIORITY, NULL);
#ifdef ENABLE_HEARTBEAT
    xTaskCreate(heartbeat_task, "HEARTBEAT", 128, NULL, tskIDLE_PRIORITY, NULL);
#endif

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Cmd_ProcessActive(uint8_t *buffer, uint16_t len) { vPortFree(buffer); }
