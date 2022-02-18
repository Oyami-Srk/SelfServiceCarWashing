/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: serv_cmd.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */
#include "Common/utils.h"
#include "GUI/gui_lvgl.h"
#include "GUI/handles.h"
#include "GUI/porting.h"
#include "Net/at.h"
#include "Net/cmd.h"
#include "Tasks/tasks.h"

#include <stdbool.h>

extern bool cmd_running;
#define WAIT_FOR_RUNNING()                                                     \
    while (cmd_running)                                                        \
        vTaskDelay(pdMS_TO_TICKS(50));                                         \
    cmd_running = true

extern char cmd_send_buffer[];

static QueueHandle_t     AT_Msg_Queue = NULL;
static AT_Response_Msg_t msg;

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

extern bool inuse;

CMD_RESULT Srv_GetStatus(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();
    const uint8_t *ip = AT_GetIP();
    const char    *status;
    if (inuse)
        status = "INUSE";
    else
        status = "OK";
    sprintf(cmd_send_buffer, DEV_RESP_GETSTATUS, status, AT_GetIdent(), ip[0],
            ip[1], ip[2], ip[3], GetRTCTime(), SOFTWARE_VER,
            AT_GetRadioStrength(), FOAM_TO_WATER_FRACTION);

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    cmd_running = false;
    vPortFree(recv_buffer);
    return CMD_RESULT_OK;
}

static char  login_call_userId[64];
static char  login_call_dispName[64];
static float avail = 0.0f;

static void cb_login_call(lv_timer_t *timer) {
    if (avail == 0) {
        return;
    }

    start_inuse_task(login_call_userId, avail);

    login_switch_out();
    inuse_switch_in(login_call_dispName);

    lv_timer_del(timer);
}

CMD_RESULT Srv_UserLogin(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();
    if (inuse) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());

        LOGF("[CMD] Send: %s", cmd_send_buffer);
        AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

        cmd_running = false;
        vPortFree(recv_buffer);
        return CMD_RESULT_ERROR;
    }
    memset(login_call_dispName, 0, 64);
    memset(login_call_userId, 0, 64);
    avail = 0.0f;

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer,
           SRV_CMD_HEAD " " SRV_CMD_LOGIN " " SRV_CMD_LOGIN_PARAM, ident,
           login_call_userId, &avail, login_call_dispName);

    CMD_RESULT result = CMD_RESULT_OK;
    bool       match  = (strcmp(AT_GetIdent(), ident) == 0);

    if (avail != 0.0f && match) {
        lv_timer_t *timer = lv_timer_create(cb_login_call, 1000, NULL);
        lv_timer_set_repeat_count(timer, 1);
        sprintf(cmd_send_buffer, DEV_RESP_OK "%s\n", AT_GetIdent());
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR "%s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    cmd_running = false;
    vPortFree(recv_buffer);
    return result;
}

static void cb_logout_call(lv_timer_t *timer) {
    logout();
    inuse_switch_out();
    login_switch_in();

    lv_timer_del(timer);
}

CMD_RESULT Srv_UserLogout(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_LOGOUT " %s",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    if (!inuse || !match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());

        LOGF("[CMD] Send: %s", cmd_send_buffer);
        AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

        cmd_running = false;
        vPortFree(recv_buffer);
        return CMD_RESULT_ERROR;
    }

    sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());

    lv_timer_t *timer = lv_timer_create(cb_logout_call, 1000, NULL);
    lv_timer_set_repeat_count(timer, 1);

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    cmd_running = false;
    vPortFree(recv_buffer);
    return CMD_RESULT_OK;
}

CMD_RESULT Srv_Reset(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_RESET " %s",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (inuse || !match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    if (!inuse && match) {
        HAL_NVIC_SystemReset();
    }

    cmd_running = false;
    vPortFree(recv_buffer);
    return result;
}

CMD_RESULT Srv_Shutdown(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_SHUTDOWN " %s",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (inuse || !match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    if (!inuse && match) {
        HAL_PWR_EnterSTANDBYMode();
    }

    cmd_running = false;
    vPortFree(recv_buffer);
    return result;
}

CMD_RESULT Srv_ServiceStop(uint8_t *recv_buffer, uint16_t len) {
    vPortFree(recv_buffer);
    return CMD_RESULT_OK;
}

CMD_RESULT Srv_ServiceStart(uint8_t *recv_buffer, uint16_t len) {
    vPortFree(recv_buffer);
    return CMD_RESULT_OK;
}

CMD_RESULT Srv_DispOff(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_DISPOFF " %s",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (inuse || !match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    if (!inuse && match) {
        HAL_GPIO_WritePin(GPIO(LCD_BACK_LIGHT), GPIO_PIN_RESET);
    }

    cmd_running = false;
    vPortFree(recv_buffer);
    return result;
}

CMD_RESULT Srv_DispOn(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_DISPON " %s",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (inuse || !match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    if (!inuse && match) {
        HAL_GPIO_WritePin(GPIO(LCD_BACK_LIGHT), GPIO_PIN_SET);
    }

    cmd_running = false;
    vPortFree(recv_buffer);
    return result;
}

static void cb_long_msg_set(lv_timer_t *timer) {
    set_long_message(timer->user_data + sizeof("+SERVCMD DISPLAY_MESSAGE") +
                     NET_IDENT_SIZE);
    vPortFree(timer->user_data);
    lv_timer_del(timer);
}

CMD_RESULT Srv_DispMessage(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_DISPMSG " %s ",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (!match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    lv_timer_t *timer = lv_timer_create(cb_long_msg_set, 1000, recv_buffer);
    lv_timer_set_repeat_count(timer, 1);

    cmd_running = false;
    return result;
}

static void cb_qrcode(lv_timer_t *timer) {
    uint8_t *qr_array =
        timer->user_data + sizeof("+SERVCMD DISPLAY_QRCODE ") + NET_IDENT_SIZE;
    redraw_qr_code(qr_array);
    vPortFree(timer->user_data);
    lv_timer_del(timer);
}

CMD_RESULT Srv_DispQRCode(uint8_t *recv_buffer, uint16_t len) {
    WAIT_FOR_RUNNING();

    char ident[18];
    memset(ident, 0, 18);

    sscanf((const char *)recv_buffer, SRV_CMD_HEAD " " SRV_CMD_QRCODE " %s ",
           ident);

    bool match = (strcmp(AT_GetIdent(), ident) == 0);

    CMD_RESULT result = CMD_RESULT_OK;
    if (!match) {
        sprintf(cmd_send_buffer, DEV_RESP_ERROR " %s\n", AT_GetIdent());
        result = CMD_RESULT_ERROR;
    } else {
        sprintf(cmd_send_buffer, DEV_RESP_OK " %s\n", AT_GetIdent());
    }

    LOGF("[CMD] Send: %s", cmd_send_buffer);
    AT_SendCommand((uint8_t *)cmd_send_buffer, strlen(cmd_send_buffer));

    lv_timer_t *timer = lv_timer_create(cb_qrcode, 1000, recv_buffer);
    lv_timer_set_repeat_count(timer, 1);

    cmd_running = false;
    return result;
}

CMD_RESULT Srv_SetConf(uint8_t *recv_buffer, uint16_t len) {
    vPortFree(recv_buffer);
    return CMD_RESULT_OK;
}
