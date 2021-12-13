/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: procedure.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Comm/procedure.h"
#include "FreeRTOS.h"
#include "GUI/gui_lvgl.h"
#include "GUI/handles.h"
#include "MD5/md5.h"
#include "Net/cmd.h"
#include "Net/vars.h"
#include "main.h"
#include "queue.h"
#include "rtc.h"
#include "semphr.h"
#include "time.h"
#include <stdio.h>
#include <string.h>

#define SERVER_RESP            "+SERVRESP %c"
#define REGISTER_COMMAND       "+CTRLCMD REGISTER %s %hhu.%hhu.%hhu.%hhu %lld %s %d\n"
#define USER_LOGIN_COMMAND     "+CTRLCMD USER_LOGIN %s %s\n"
#define USER_LOGIN_OK_RESP     "+SERVRESP OK %s %f\n"
#define USER_LOGOUT_COMMAND    "+CTRLCMD USER_LOGOUT %s %f %f %d\n"
#define ACQUIRE_QRCODE_COMMAND "+CTRLCMD ACQUIRE_QRCODE\n"

uint8_t QR_ARRAY[2048];
uint8_t QR_ARRQY_MODIFIED = 0;

void callback_set_qr_code(lv_timer_t *timer) {
    draw_qr_code(QR_ARRAY);
    lv_timer_del(timer);
}

time_t mktimestamp() {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    struct tm currTime = {.tm_year = date.Year + 100,
                          .tm_mon  = date.Month - 1,
                          .tm_mday = date.Date,
                          .tm_hour = time.Hours,
                          .tm_min  = time.Minutes,
                          .tm_sec  = time.Seconds};

    return mktime(&currTime);
}

uint8_t GET_SERVER_RESP_STATUS(const char *msg) {
    char buffer;
    sscanf(msg, SERVER_RESP, &buffer);
    switch (buffer) {
    case 'O':
        return CMD_RESULT_OK;
    case 'E':
        return CMD_RESULT_ERROR;
    default:
        return CMD_RESULT_ERROR;
    }
}

static char buffer[64] = {0};

uint8_t REGISTER_DEVICE() {
    sprintf(buffer, REGISTER_COMMAND, NET_MAC, NET_IPV4[0], NET_IPV4[1],
            NET_IPV4[2], NET_IPV4[3], mktimestamp(), "0.1.0", 99);
    printf("[COMM] Send command: %s\r\n", buffer);
    osMessageQueueId_t qid = xQueueCreate(1, sizeof(NET_MSG));
    NET_MSG            msg;
    if (SEND_MSG(buffer, strlen(buffer), qid) != SEND_MSG_STATUS_OK) {
        printf("[COMM] Cannot send.\r\n");
        return CMD_RESULT_ERROR;
    }
    if (pdFALSE == xQueueReceive(qid, &msg, pdMS_TO_TICKS(1000 * 10))) {
        vQueueDelete(qid);
        return CMD_RESULT_TIMEOUT;
    }
    RECIVED_MSG(); // clear wait queue
    return GET_SERVER_RESP_STATUS(msg.recv);
}

// userid: 32 size len, avail: available water amount.
uint8_t USER_LOGIN(const char *username, const char *password, char *userid,
                   float *avail) {
    static uint8_t MD5Result[16] = {0};
    static MD5_CTX md5ctx;
    uint16_t       size = strlen(password);

    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (unsigned char *)password, size);
    MD5Final(MD5Result, &md5ctx);
    char md5str[33] = {0};
    for (int i = 0; i < 16; i++)
        sprintf(md5str + 2 * i, "%02X", MD5Result[i]);
    sprintf(buffer, USER_LOGIN_COMMAND, username, md5str);

    printf("[COMM] Send command: %s\r\n", buffer);
    osMessageQueueId_t qid = xQueueCreate(1, sizeof(NET_MSG));
    NET_MSG            msg;
    if (SEND_MSG(buffer, strlen(buffer), qid) != SEND_MSG_STATUS_OK) {
        printf("[COMM] Cannot send.\r\n");
        return CMD_RESULT_ERROR;
    }
    if (pdFALSE == xQueueReceive(qid, &msg, pdMS_TO_TICKS(1000 * 10))) {
        vQueueDelete(qid);
        return CMD_RESULT_TIMEOUT;
    }
    RECIVED_MSG(); // clear wait queue
    uint8_t status = GET_SERVER_RESP_STATUS(msg.recv);
    if (status == CMD_RESULT_OK) {
        sscanf(msg.recv, USER_LOGIN_OK_RESP, userid, avail);
    } else {
        *userid = '\0';
    }
    return status;
}

uint8_t USER_LOGOUT(const char *userid, float water_used, float foam_used,
                    uint32_t time_used) {
    sprintf(buffer, USER_LOGOUT_COMMAND, userid, water_used, foam_used,
            time_used);

    printf("[COMM] Send command: %s\r\n", buffer);
    osMessageQueueId_t qid = xQueueCreate(1, sizeof(NET_MSG));
    NET_MSG            msg;
    if (SEND_MSG(buffer, strlen(buffer), qid) != SEND_MSG_STATUS_OK) {
        printf("[COMM] Cannot send.\r\n");
        return CMD_RESULT_ERROR;
    }
    if (pdFALSE == xQueueReceive(qid, &msg, pdMS_TO_TICKS(1000 * 10))) {
        vQueueDelete(qid);
        return CMD_RESULT_TIMEOUT;
    }
    RECIVED_MSG(); // clear wait queue
    uint8_t status = GET_SERVER_RESP_STATUS(msg.recv);
    return status;
}

// direct draw
uint8_t ACQUIRE_QRCODE() {
    printf("[COMM] Send command: %s\r\n", ACQUIRE_QRCODE_COMMAND);
    osMessageQueueId_t qid = xQueueCreate(1, sizeof(NET_MSG));
    NET_MSG            msg;
    if (SEND_MSG(ACQUIRE_QRCODE_COMMAND, sizeof(ACQUIRE_QRCODE_COMMAND) - 1,
                 qid) != SEND_MSG_STATUS_OK) {
        printf("[COMM] Cannot send.\r\n");
        return CMD_RESULT_ERROR;
    }
    if (pdFALSE == xQueueReceive(qid, &msg, pdMS_TO_TICKS(1000 * 10))) {
        vQueueDelete(qid);
        return CMD_RESULT_TIMEOUT;
    }
    RECIVED_MSG(); // clear wait queue
    uint8_t status = GET_SERVER_RESP_STATUS(msg.recv);
    if (status == CMD_RESULT_OK) {
        uint8_t *recv = msg.recv + sizeof("+SERVRESP OK ") - 1;
        uint8_t  size = *recv;
        if (size == 0) {
        } else {
            printf("Received size %dx%d qrcode.\r\n", size, size);
            uint16_t size2 = size * size;
            memcpy(QR_ARRAY, recv, size2 / 8 + (size2 % 8 ? 1 : 0));
            QR_ARRQY_MODIFIED = 1;
            lv_timer_t *handle =
                lv_timer_create(callback_set_qr_code, 1000, NULL);
        }
    } else {
    }
    return status;
}