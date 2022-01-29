#include <sys/cdefs.h>
#include <sys/types.h>
/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: handles.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "FreeRTOS.h"
#include "main.h"
#include "queue.h"

#include "Common/msg_types.h"
#include "Common/utils.h"
#include "Net/AT/command.h"
#include "Net/AT/utils.h"
#include "Net/WIFI/handles.h"
#include "Net/WIFI/procedure.h"
#include "Net/cmd.h"
#include "Net/config.h"
#include "cmsis_os.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

osThreadId pid_net_wifi_task;

const osThreadAttr_t task_net_wifi_attributes = {
    .name       = "net-at-wifi",
    .stack_size = 1024,
    .priority   = (osPriority_t)osPriorityAboveNormal,
};

_Noreturn void task_net_wifi(void *argument);

void NET_WIFI_INIT() {
    printf("[MODULE] NET/WIFI Initializing.\r\n");
    pid_net_wifi_task =
        osThreadNew(task_net_wifi, NULL, &task_net_wifi_attributes);
    printf("[MODULE] NET/WIFI Initialization completed.\r\n");
}

extern uint8_t NET_RX_BUFFER[NET_BUFFER_SIZE]; // common_vars.c
uint8_t        msg_buffer[sizeof(NET_AT_RX_MSG)] = {0};

extern uint8_t NET_STATUS;
extern char    NET_MAC[18];
extern uint8_t NET_IPV4[4];

#define EMPTY_QUEUE(qid, msg_buffer)                                           \
    while (xQueueReceive(qid, msg_buffer, pdMS_TO_TICKS(1000)) == pdTRUE)      \
        ;

osMessageQId qid_wait_cmd = NULL;

_Noreturn void task_net_wifi(void *argument) {
    COMPILE_TIME_ASSERT(sizeof(NET_AT_RX_MSG) == sizeof(NET_WIFI_REQUEST_MSG));

    printf("[NET/WIFI] Enter WIFI Task.\r\n");
    NET_STATUS            = NET_STATUS_NOT_CONNECTED;
    osMessageQId qid_wifi = xQueueCreate(16, sizeof(NET_AT_RX_MSG));
    NET_AT_REGISTER_RECV_BUFFER(NET_RX_BUFFER, NET_BUFFER_SIZE, qid_wifi);

    vTaskDelay(pdMS_TO_TICKS(100));
    NET_AT_SEND_STATIC_CMD("+++"); // Exit pass-through for restart.
    vTaskDelay(pdMS_TO_TICKS(100));
    NET_AT_SEND_STATIC_CMD("AT+RST\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    NET_AT_START_RECV();
    NET_AT_SEND_STATIC_CMD("ATE0\r\n");
    EMPTY_QUEUE(qid_wifi, msg_buffer);

    NET_AT_SEND_STATIC_CMD("AT+GMR\r\n");
    NET_AT_RX_MSG *msg = (NET_AT_RX_MSG *)msg_buffer;

    for (;;) {
        xQueueReceive(qid_wifi, msg_buffer, osWaitForever);
        if (msg_buffer[0] == NET_AT_MSG_UART_RECV) {
            if (msg->len == 11 && memcmp(NET_RX_BUFFER, "busy", 4) == 0)
                continue;
            if (AT_GET_RESULT(NET_RX_BUFFER, msg->len) != AT_OK) {
                printf("[NET/WIFI] WiFi Module not ready. Wait 30 sec\r\n");
                NET_STATUS = NET_STATUS_DEVICE_FAIL;
                vTaskDelay(pdMS_TO_TICKS(1000 * 30));
                EMPTY_QUEUE(qid_wifi, msg_buffer);
                NET_AT_SEND_STATIC_CMD("AT+GMR\r\n");
                continue;
            } else {
                printf("[NET/WIFI] WiFi Module ready.\r\n");
                PRINT_RX_BUFFER(NET_RX_BUFFER, msg->len, ">>>    ");
            }
            break;
        }
    }

    // connect to ap
connect_to_ap:;
    EMPTY_QUEUE(qid_wifi, msg_buffer);
    uint8_t  p = 0;
    Status_t status;
    while ((p = WIFI_CONNECT_TO_AP(NET_RX_BUFFER, msg->len, p, &status)) != 0) {
        if (status != OK) {
            printf("[NET/WIFI] Failed to run connect to ap at step %d. Halt "
                   "for 30sec and retry.\r\n",
                   p - 1);
            EMPTY_QUEUE(qid_wifi, msg_buffer);
            NET_AT_SEND_STATIC_CMD("AT+RST\r\n"); // reset module
            vTaskDelay(pdMS_TO_TICKS(1000 * 30));
            goto connect_to_ap;
        }
        for (;;) {
            xQueueReceive(qid_wifi, msg_buffer, osWaitForever);
            if (msg->len == 11 && memcmp(NET_RX_BUFFER, "busy", 4) == 0)
                continue;
            if (msg_buffer[0] == NET_AT_MSG_UART_RECV) {
                break;
            }
        }
    }

    printf("[NET/WIFI] WiFi Connected to Network. Trying to update time.\r\n");
update_time:
    EMPTY_QUEUE(qid_wifi, msg_buffer);
    p = 0;
    while ((p = WIFI_UPDATE_TIME(NET_RX_BUFFER, msg->len, p, &status)) != 0) {
        if (status != OK) {
            if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0xF103) {
                printf("[NET/WIFI] NTP Time update failed, but we got "
                       "backup.\r\n");
                printf("[NET/WIFI] If time is incorrect, please restart the "
                       "device.\r\n");
                break;
            }
            printf("[NET/WIFI] Failed to run update time at step %d. Halt "
                   "for 30sec and retry.\r\n",
                   p - 1);
            EMPTY_QUEUE(qid_wifi, msg_buffer);
            vTaskDelay(pdMS_TO_TICKS(1000 * 30));
            goto update_time;
        }
        for (;;) {
            xQueueReceive(qid_wifi, msg_buffer, osWaitForever);
            if (msg->len == 11 && memcmp(NET_RX_BUFFER, "busy", 4) == 0)
                continue;
            if (msg_buffer[0] == NET_AT_MSG_UART_RECV) {
                break;
            }
        }
    }

    printf("[NET/WIFI] RTC Clock updated. Trying to Connect to server.\r\n");
update_time_end:
connect_to_server:
    EMPTY_QUEUE(qid_wifi, msg_buffer);
    p = 0;
    while ((p = WIFI_CONNECT_TO_SERVER(NET_RX_BUFFER, msg->len, p, &status)) !=
           0) {
        if (status != OK) {
            printf(
                "[NET/WIFI] Failed to run connect to server at step %d. Halt "
                "for 30sec and retry.\n",
                p - 1);
            EMPTY_QUEUE(qid_wifi, msg_buffer);
            vTaskDelay(pdMS_TO_TICKS(1000 * 30));
            goto connect_to_server;
        }
        for (;;) {
            xQueueReceive(qid_wifi, msg_buffer, osWaitForever);
            if (msg->len == 11 && memcmp(NET_RX_BUFFER, "busy", 4) == 0)
                continue;
            if (msg_buffer[0] == NET_AT_MSG_UART_RECV) {
                break;
            }
        }
    }

    // Enable pass-through mode after setup. We do not care about TCP

    for (;;) {
        xQueueReceive(qid_wifi, msg_buffer, osWaitForever);
        switch (msg_buffer[0]) {
        case NET_AT_MSG_UART_RECV: {
            printf("Received Bytes: %d\r\n", msg->len);
            PRINT_RX_BUFFER(NET_RX_BUFFER, msg->len, ">>>    ");
            if (qid_wait_cmd) {
                NET_MSG msg1;
                msg1.recv = NET_RX_BUFFER;
                msg1.len  = msg->len;
                xQueueSend(qid_wait_cmd, &msg1, pdMS_TO_TICKS(3000));
            }
            break;
        }
        default:
            printf("[NET/WIFI] Received Unknown type of message.\r\n");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}