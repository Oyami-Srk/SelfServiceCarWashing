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
#include "Net/cmd.h"
#include "Net/vars.h"
#include "main.h"
#include "queue.h"
#include "rtc.h"
#include "time.h"
#include <stdio.h>
#include <string.h>

#define SERVER_RESP      "+SERVRESP %c"
#define REGISTER_COMMAND "+CTRLCMD REGISTER %s %hhu.%hhu.%hhu.%hhu %lld %s %d\n"

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

uint8_t REGISTER_DEVICE() {
    static char buffer[64] = {0};
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
