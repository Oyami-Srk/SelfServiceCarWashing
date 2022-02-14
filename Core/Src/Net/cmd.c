/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: cmd.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */
#include "Net/cmd.h"
#include "Common/utils.h"
#include "GUI/gui_lvgl.h"
#include "GUI/handles.h"
#include "MD5/md5.h"
#include "Net/at.h"
#include <stdbool.h>

static bool cmd_running = false;
#define WAIT_FOR_RUNNING()                                                     \
    while (cmd_running)                                                        \
        vTaskDelay(pdMS_TO_TICKS(50));                                         \
    cmd_running = true

CMD_RESULT Cmd_GetRespResult(const char *msg) {
    char c;
    sscanf(msg, SRV_RESP, &c);
    switch (c) {
    case 'O':
        return CMD_RESULT_OK;
    case 'E':
        return CMD_RESULT_ERROR;
    default:
        return CMD_RESULT_UNKNOWN;
    }
}

static char       buffer[64]   = {0};
QueueHandle_t     AT_Msg_Queue = NULL;
AT_Response_Msg_t msg;

#define CREATE_AND_REGISTER()                                                  \
    AT_Msg_Queue = xQueueCreate(8, sizeof(AT_Response_Msg_t));                 \
    if (AT_Msg_Queue == NULL) {                                                \
        LOG("[NET] Queue cannot be create.");                                  \
        Error_Handler();                                                       \
    }                                                                          \
    AT_RegisterResponse(AT_Msg_Queue)

#define UNREGISTER_AND_DELETE()                                                \
    AT_UnregisterResponse(AT_Msg_Queue);                                       \
    vQueueDelete(AT_Msg_Queue)

CMD_RESULT Cmd_RegisterDevice() {
    WAIT_FOR_RUNNING();
    const uint8_t *ip = AT_GetIP();
    sprintf(buffer, DEV_CMD_REGISTER, AT_GetMacAddr(), ip[0], ip[1], ip[2],
            ip[3], GetRTCTime(), "0.2.0", AT_GetRadioStrength());
    LOGF("[CMD] Send: %s", buffer);
    CREATE_AND_REGISTER();

    AT_SendCommand((uint8_t *)buffer, strlen(buffer));
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);

    CMD_RESULT result = Cmd_GetRespResult((const char *)msg.Buffer);
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();
    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_UserLogin(const char *username, const char *password,
                         char *userId, float *avail) {
    WAIT_FOR_RUNNING();
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    static uint8_t MD5Result[16] = {0};

    static MD5_CTX md5ctx;
    uint16_t       size = strlen(password);

    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (unsigned char *)password, size);
    MD5Final(MD5Result, &md5ctx);

    char md5str[33] = {0};

    for (int i = 0; i < 16; i++)
        sprintf(md5str + 2 * i, "%02X", MD5Result[i]);

    sprintf(buffer, DEV_CMD_LOGIN, username, md5str);

    LOGF("[CMD] Send: %s", buffer);
    CREATE_AND_REGISTER();

    AT_SendCommand((uint8_t *)buffer, strlen(buffer));
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);

    CMD_RESULT result = Cmd_GetRespResult((const char *)msg.Buffer);
    if (result == CMD_RESULT_OK) {
        sscanf((char *)msg.Buffer, SRV_RESP_LOGIN_OK, userId, avail);
    } else {
        *userId = '\0';
    }
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_UserLogOut(const char *userId, float water_used, float foam_used,
                          uint32_t time_used) {
    WAIT_FOR_RUNNING();
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    sprintf(buffer, DEV_CMD_LOGOUT, userId, water_used, foam_used, time_used);

    LOGF("[CMD] Send: %s", buffer);
    CREATE_AND_REGISTER();

    AT_SendCommand((uint8_t *)buffer, strlen(buffer));
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);

    CMD_RESULT result = Cmd_GetRespResult((const char *)msg.Buffer);
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

static void cb_set_qr_code(lv_timer_t *timer) {
    uint8_t *qr_array = timer->user_data + sizeof("+SERVRESP OK ") - 1;
    redraw_qr_code(qr_array);
    vPortFree(timer->user_data);
    lv_timer_del(timer);
}

CMD_RESULT Cmd_AcquireQRCode() {
    WAIT_FOR_RUNNING();
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    LOGF("[CMD] Send: %s", DEV_CMD_QRCODE);
    CREATE_AND_REGISTER();

    AT_SendCommand((uint8_t *)DEV_CMD_QRCODE, sizeof(DEV_CMD_QRCODE) - 1);
    //    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    if (xQueueReceive(AT_Msg_Queue, &msg,
                      ((TickType_t)(((TickType_t)(10 * 1000) *
                                     (TickType_t)((TickType_t)1000)) /
                                    (TickType_t)1000))) == ((BaseType_t)0)) {
        f_printf("[UART] Cannot receive response."
                 "\r\n");
        Error_Handler();
    }

    CMD_RESULT result = Cmd_GetRespResult((const char *)msg.Buffer);
    if (result == CMD_RESULT_OK) {
        uint8_t *qr_array = msg.Buffer + sizeof("+SERVRESP OK ") - 1;
        uint16_t size;
        if ((size = (*((uint16_t *)qr_array))) == 0) {
            result = CMD_RESULT_ERROR;
        } else {
            LOGF("Received size %dx%d Qr Code", size, size);
            LOGF("Wanted bytes: %d, Received bytes: %d.",
                 (size * size) / 8 + 2 + sizeof("+SERVRESP OK ") - 1 +
                     ((size * size) % 2 ? 1 : 0),
                 msg.Len);
            //            redraw_qr_code(qr_array);
            lv_timer_create(cb_set_qr_code, 1000, msg.Buffer);
            UNREGISTER_AND_DELETE();
            cmd_running = false;
            return result;
        }
    }
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_HeartBeat() {
    WAIT_FOR_RUNNING();

    const uint8_t *ip = AT_GetIP();
    sprintf(buffer, DEV_CMD_HEARTBEAT, AT_GetMacAddr(), ip[0], ip[1], ip[2],
            ip[3], GetRTCTime(), "0.2.0", AT_GetRadioStrength());
    LOGF("[CMD] Send: %s", buffer);
    CREATE_AND_REGISTER();

    AT_SendCommand((uint8_t *)buffer, strlen(buffer));
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);

    CMD_RESULT result = Cmd_GetRespResult((const char *)msg.Buffer);
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}
