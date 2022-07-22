/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: dev_cmd.c
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

bool cmd_running = false;
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

char cmd_send_buffer[128] = {0};

static QueueHandle_t     AT_Msg_Queue = NULL;
static AT_Response_Msg_t msg;

#define CREATE_AND_REGISTER()                                                  \
    AT_Msg_Queue = xQueueCreate(8, sizeof(AT_Response_Msg_t));                 \
    if (AT_Msg_Queue == NULL) {                                                \
        LOG("[NET] Queue cannot be create.");                                  \
        Error_Handler();                                                       \
    }                                                                          \
    AT_UART_RegisterResponse(AT_Msg_Queue)

#define UNREGISTER_AND_DELETE()                                                \
    AT_UART_UnregisterResponse(AT_Msg_Queue);                                  \
    vQueueDelete(AT_Msg_Queue)

CMD_RESULT Cmd_RegisterDevice() {
    WAIT_FOR_RUNNING();
    const uint8_t *ip = AT_GetIP();
    sprintf(cmd_send_buffer, DEV_CMD_REGISTER, AT_GetIdent(), ip[0], ip[1],
            ip[2], ip[3], GetRTCTime(), SOFTWARE_VER, AT_GetRadioStrength());
    LOGF("[CMD] Send: %s", cmd_send_buffer);
    CREATE_AND_REGISTER();

    AT_UART_Send((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));
    AT_RESULT at_result =
        AT_UART_Recv(AT_Msg_Queue, &msg, AT_DEFAULT_WAIT_DELAY);
    CMD_RESULT result = CMD_RESULT_UNKNOWN;
    if (at_result == AT_OK)
        result = Cmd_GetRespResult((const char *)msg.Buffer);
    else
        result = CMD_RESULT_ERROR;
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();
    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_UserLogin(const char *username, const char *password,
                         char *userId, char *user_dispname, float *avail) {
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

    sprintf(cmd_send_buffer, DEV_CMD_LOGIN, AT_GetIdent(), username, md5str);

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    CREATE_AND_REGISTER();

    AT_UART_Send((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));
    AT_RESULT at_result =
        AT_UART_Recv(AT_Msg_Queue, &msg, AT_DEFAULT_WAIT_DELAY);
    CMD_RESULT result = CMD_RESULT_UNKNOWN;
    if (at_result == AT_OK) {
        result = Cmd_GetRespResult((const char *)msg.Buffer);
        if (memcmp(msg.Buffer + sizeof("+SERVRESP OK"), AT_GetIdent(),
                   NET_IDENT_SIZE) != 0)
            result = CMD_RESULT_ERROR;

        if (result == CMD_RESULT_OK) {
            sscanf(
                (char *)(msg.Buffer + sizeof("+SERVRESP OK ") + NET_IDENT_SIZE),
                SRV_RESP_LOGIN_OK, userId, user_dispname, avail);
        } else {
            *userId = '\0';
        }
    } else
        result = CMD_RESULT_ERROR;

    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_UserLogOut(const char *userId, float water_used, float foam_used,
                          uint32_t time_used, float *used_money,
                          float *left_money) {
    WAIT_FOR_RUNNING();
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    sprintf(cmd_send_buffer, DEV_CMD_LOGOUT, AT_GetIdent(), userId, water_used,
            foam_used, time_used);

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    CREATE_AND_REGISTER();

    AT_UART_Send((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));
    AT_RESULT at_result =
        AT_UART_Recv(AT_Msg_Queue, &msg, AT_DEFAULT_WAIT_DELAY);
    CMD_RESULT result = CMD_RESULT_UNKNOWN;
    if (at_result == AT_OK) {
        result = Cmd_GetRespResult((const char *)msg.Buffer);

        if (memcmp(msg.Buffer + sizeof("+SERVRESP OK"), AT_GetIdent(),
                   NET_IDENT_SIZE) != 0)
            result = CMD_RESULT_ERROR;

        if (result == CMD_RESULT_OK) {
            sscanf(
                (char *)(msg.Buffer + sizeof("+SERVRESP OK ") + NET_IDENT_SIZE),
                SRV_RESP_LOGOUT_OK, used_money, left_money);
        } else {
            *used_money = 0.0f;
            *left_money = 0.0f;
        }
    } else {
        result = CMD_RESULT_ERROR;
    }
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

static void cb_set_qr_code(lv_timer_t *timer) {
    uint8_t *qr_array =
        timer->user_data + sizeof("+SERVRESP OK ") + NET_IDENT_SIZE;
    redraw_qr_code(qr_array);
    vPortFree(timer->user_data);
    lv_timer_del(timer);
}

CMD_RESULT Cmd_AcquireQRCode() {
    WAIT_FOR_RUNNING();
    AT_WaitForStatus(NET_DEVICE_REGISTERED, portMAX_DELAY);

    sprintf(cmd_send_buffer, DEV_CMD_QRCODE, AT_GetIdent());

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    CREATE_AND_REGISTER();

    AT_UART_Send((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));
    AT_RESULT at_result =
        AT_UART_Recv(AT_Msg_Queue, &msg, AT_DEFAULT_WAIT_DELAY);
    CMD_RESULT result = CMD_RESULT_UNKNOWN;
    if (at_result == AT_OK) {
        result = Cmd_GetRespResult((const char *)msg.Buffer);
        if (memcmp(msg.Buffer + sizeof("+SERVRESP OK"), AT_GetIdent(),
                   NET_IDENT_SIZE) != 0)
            result = CMD_RESULT_ERROR;

        if (result == CMD_RESULT_OK) {
            uint8_t *qr_array =
                msg.Buffer + sizeof("+SERVRESP OK ") + NET_IDENT_SIZE;
            uint16_t size = qr_array[0] | (qr_array[1] << 8);
            //        if ((size = (*((uint16_t *)qr_array))) == 0) {
            if (size == 0) {
                result = CMD_RESULT_ERROR;
            } else {
                LOGF("Received size %dx%d Qr Code", size, size);
                LOGF("Wanted bytes: %d, Received bytes: %d.",
                     (size * size) / 8 + 2 + sizeof("+SERVRESP OK ") +
                         NET_IDENT_SIZE + ((size * size) % 2 ? 1 : 0),
                     msg.Len);
                //            redraw_qr_code(qr_array);
                lv_timer_create(cb_set_qr_code, 1000, msg.Buffer);
                UNREGISTER_AND_DELETE();
                cmd_running = false;
                return result;
            }
        }
    } else {
        result = CMD_RESULT_ERROR;
    }
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}

CMD_RESULT Cmd_HeartBeat() {
    WAIT_FOR_RUNNING();

    const uint8_t *ip = AT_GetIP();
    sprintf(cmd_send_buffer, DEV_CMD_HEARTBEAT, AT_GetIdent(), ip[0], ip[1],
            ip[2], ip[3], GetRTCTime(), SOFTWARE_VER, AT_GetRadioStrength());
    LOGF("[CMD] Send: %s", cmd_send_buffer);
    CREATE_AND_REGISTER();

    AT_UART_Send((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));
    AT_RESULT at_result =
        AT_UART_Recv(AT_Msg_Queue, &msg, AT_DEFAULT_WAIT_DELAY);
    CMD_RESULT result = CMD_RESULT_UNKNOWN;
    if (at_result == AT_OK)
        result = Cmd_GetRespResult((const char *)msg.Buffer);
    else
        result = CMD_RESULT_ERROR;
    AT_FREE_RESP(msg);
    UNREGISTER_AND_DELETE();

    cmd_running = false;
    return result;
}
