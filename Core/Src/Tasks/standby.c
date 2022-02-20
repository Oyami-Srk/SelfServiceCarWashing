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
    xTaskCreate(register_task, "REGISTER", 512, NULL, tskIDLE_PRIORITY, NULL);
#ifdef ENABLE_HEARTBEAT
    xTaskCreate(heartbeat_task, "HEARTBEAT", 512, NULL, tskIDLE_PRIORITY, NULL);
#endif

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Cmd_ProcessActive(uint8_t *buffer, uint16_t len) {
    if (STATIC_STR_CMP(buffer, "+SERVCMD")) {
        LOG("Received Server command.");
        char *ptocomp = (char *)buffer + sizeof("+SERVCMD");
        if (STATIC_STR_CMP(ptocomp, SRV_CMD_GETSTATUS)) {
            Srv_GetStatus(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_LOGIN)) {
            Srv_UserLogin(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_LOGOUT)) {
            Srv_UserLogout(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_RESET)) {
            Srv_Reset(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_SHUTDOWN)) {
            Srv_Shutdown(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_START)) {
            Srv_ServiceStart(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_STOP)) {
            Srv_ServiceStop(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_DISPOFF)) {
            Srv_DispOff(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_DISPON)) {
            Srv_DispOn(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_DISPMSG)) {
            Srv_DispMessage(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_QRCODE)) {
            Srv_DispQRCode(buffer, len);
        } else if (STATIC_STR_CMP(ptocomp, SRV_CMD_SETCONF)) {
            Srv_SetConf(buffer, len);
        } else {
            len -= sizeof("+SERVCMD");
            char *p = ptocomp;
            while (len && *p != ' ') {
                p++;
                len--;
            }
            *p = '\0';

            LOGF("Unknown Command: %s", ptocomp);
            vPortFree(buffer);
        }
    } else {
        vPortFree(buffer);
    }
}
