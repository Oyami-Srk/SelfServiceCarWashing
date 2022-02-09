/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: wifi.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Net/wifi.h"
#include "Common/config.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include "Net/at.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#ifdef NET_MODULE_ESP32

void task_wifi_init(void *args) {
    // TODO: Guard for connection lost
    int retry_intv;
retry_start:
    retry_intv = 3000;
    vTaskDelay(pdMS_TO_TICKS(100));
    LOG("[WIFI] Booting WiFi Module.");
    AT_ResetStatus();

    QueueHandle_t AT_Msg_Queue;
    AT_Msg_Queue = xQueueCreate(8, sizeof(AT_Response_Msg_t));

    if (AT_Msg_Queue == NULL) {
        LOG("[NET] Queue cannot be create.");
        Error_Handler();
    }

    AT_Response_Msg_t msg;
    AT_RegisterResponse(AT_Msg_Queue);

    // Reset Module
    AT_SendStaticCommand("+++"); // Exit pass-through for restart.
    vTaskDelay(pdMS_TO_TICKS(100));
    AT_SendStaticCommand("AT+RST\r\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    do {
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        if (AT_GetResult(msg.Buffer, msg.Len) == AT_ERROR)
            LOG("[WIFI] Cannot Reset WiFi Module, leave it alone.");
        AT_FREE_RESP(msg);
    } while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Disable Echo
    AT_SendStaticCommand("ATE0\r\n");
    do {
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    } while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0);

    // Get AT Status
    AT_SendStaticCommand("AT+GMR\r\n");
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    AT_RESULT result = AT_GetResult(msg.Buffer, msg.Len);
    AT_FREE_RESP(msg);
    if (result != AT_OK) {
        goto failed;
    }

#define SEND_WAIT_CHECK(cmd, message, label)                                   \
    AT_SendStaticCommand(cmd "\r\n");                                          \
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);                                       \
    result = AT_GetResult(msg.Buffer, msg.Len);                                \
    if (result != AT_OK) {                                                     \
        PRINTF(message " Retry after %d secs.\r\n", retry_intv / 1000);        \
        AT_FREE_RESP(msg);                                                     \
        retries++;                                                             \
        vTaskDelay(pdMS_TO_TICKS(retry_intv));                                 \
        goto label;                                                            \
    }

    int retries;
    // Connect to AP
    retries = 0;
connect_to_ap:
    LOG("[WIFI] Trying to connect to AP " NET_WIFI_AP_NAME);
    if (retries > NET_MAX_RETRIES) {
        LOG("[WIFI] Max retries exceeded while connect to ap. Restart all "
            "initialization procedures.");
        goto failed;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CWMODE=1", "[WIFI] Cannot set ESP32 to Station Mode.",
                    connect_to_ap);
    AT_FREE_RESP(msg);

    vTaskDelay(pdMS_TO_TICKS(100));
    AT_SendStaticCommand("AT+CWJAP=\"" NET_WIFI_AP_NAME "\",\"" NET_WIFI_AP_PSWD
                         "\"\r\n");
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    result = AT_GetResult(msg.Buffer, msg.Len);
    if (result == AT_ERROR) {
        if (STATIC_STR_CMP(msg.Buffer, "+CWJAP")) {
            uint8_t err_code;
            sscanf(msg.Buffer, "+CWJAP:%hhu", &err_code);
            PRINTF("[WIFI] Connect to AP Failed with Error code: %d(",
                   err_code);
            switch (err_code) {
            case 1:
                PRINTF("Time out");
                break;
            case 2:
                PRINTF("Password Incorrect");
                break;
            case 3:
                PRINTF("Target AP " NET_WIFI_AP_NAME " cannot be found");
                break;
            case 4:
                PRINTF("Connection failed.");
                break;
            default:
                PRINTF("Unknown error.");
                break;
            }
            PRINTF(").\r\n");
        } else {
            LOG("[WIFI] Unknown Error when connecting to AP, maybe a command "
                "mismatch. Retry after 3 secs.");
        }
        retries++;
        AT_FREE_RESP(msg);
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto connect_to_ap;
    }
    if (STATIC_STR_CMP(msg.Buffer, "busy")) {
        // busy
        LOG("[WIFI] Module busy. Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto connect_to_ap;
    }

    char *p;

    while (AT_GetNetStatus() != NET_CONNECTED) {
        while (!STATIC_STR_CMP(msg.Buffer, "WIFI")) {
            LOG("[WIFI] Waiting...");
            AT_FREE_RESP(msg);
            if (xQueueReceive(AT_Msg_Queue, &msg, pdMS_TO_TICKS(10 * 1000)) ==
                pdFALSE) {
                LOG("[UART] Timeout for waiting. Retry after 3 secs.");
                retries++;
                vTaskDelay(pdMS_TO_TICKS(3000));
                goto connect_to_ap;
            }
        }
        p          = msg.Buffer;
        int remain = msg.Len;
        int size;

        while (remain > 5) {
            if (STATIC_STR_CMP(p + 5, "DISCONNECT")) {
                LOG("[WIFI] Disconnected, wait for connected.");
                AT_SetNetStatus(NET_NOT_CONNECT);
                size = sizeof("WIFI DISCONNECT\r\n");
            } else if (STATIC_STR_CMP(p + 5, "CONNECTED")) {
                LOG("[WIFI] WiFi Connected, wait for ip.");
                AT_SetNetStatus(NET_NO_IP);
                size = sizeof("WIFI CONNECTED\r\n");
            } else if (STATIC_STR_CMP(p + 5, "GOT IP")) {
                LOG("[WIFI] WiFi Got IP.");
                AT_SetNetStatus(NET_CONNECTED);
                size = sizeof("WIFI GOT IP\r\n");
            }
            size--;
            remain -= size;
            p += size;
        }
        AT_FREE_RESP(msg);
        if (AT_GetNetStatus() != NET_CONNECTED)
            AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    }

    // Get mac address
    retries = 0;
get_mac_addr:
    LOG("[WIFI] Trying to get device's MAC address.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[WIFI] Max retries exceeded while get mac address. Restart all "
            "initialization procedures.");
        goto failed;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CIPSTAMAC?",
                    "[WIFI] Failed to get device's MAC address.", get_mac_addr);
    p = msg.Buffer;
    p += sizeof("+CIPSTAMAC:") - 1;
    if (*p != '\"') {
        LOG("[WIFI] Command result broken while get device's MAC address. "
            "Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto get_mac_addr;
    }

    char mac_buffer[17];
    memcpy(mac_buffer, p + 1, 17);
    AT_FREE_RESP(msg);
    mac_buffer[17] = '\0';
    for (p = mac_buffer; p < mac_buffer + 17; p++)
        *p = toupper(*p);
    PRINTF("[WIFI] Device MAC: %s\r\n", mac_buffer);
    AT_SetMacAddr(mac_buffer);

    // Get IP
    retries = 0;
get_ip_addr:
    LOG("[WIFI] Trying to get device's IP address.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[WIFI] Max retries exceeded while get ip address. Restart all "
            "initialization procedures.");
        goto failed;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CIPSTA?", "[WIFI] Failed to get device's IP address.",
                    get_ip_addr);
    p = (char *)msg.Buffer;
    p += sizeof("+CIPSTA:ip:") - 1;
    if (*p != '\"') {
        LOG("[WIFI] Command result broken while get device's IP address. "
            "Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto get_ip_addr;
    }
    char    ip_buffer[4] = {0};
    uint8_t ip[4]        = {0};

    p++;
    for (int i = 0; i < 4; i++) {
        char *j = ip_buffer;
        while (*p != '.' && *p != '\"')
            *j++ = *p++;
        p++;
        *(j++) = '\0';
        ip[i]  = atoi(ip_buffer);
    }
    AT_FREE_RESP(msg);
    PRINTF("[WIFI] Device IP Address: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2],
           ip[3]);
    AT_SetIP(ip);

    // Update time
    retries = 0;
update_time:;
    time_t lastUpdate = GetRTCLastUpdate();
    time_t currTime   = GetRTCTime();
    if (lastUpdate != 0 && lastUpdate - currTime < RTC_MINIUM_UPDATE_INTV) {
        char *time_buffer = ParseTimeInStr(lastUpdate);
        PRINTF("[WIFI] RTC Time is updated at %s. Skip update.\r\n",
               time_buffer);
        vPortFree(time_buffer);
        time_buffer = ParseTimeInStr(currTime);
        PRINTF("[WIFI] Now RTC Time is %s.\r\n", time_buffer);
        vPortFree(time_buffer);
        goto update_time_finish;
    }
    LOG("[WIFI] Trying to update time from SNTP.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[WIFI] Max retries exceeded while update time. Restart all "
            "initialization procedures.");
        goto failed;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CIPSNTPCFG=1,8,\"" NET_SNTP_SERVER "\"",
                    "[WIFI] Failed to configure SNTP Server.", update_time);
    AT_FREE_RESP(msg);
    SEND_WAIT_CHECK("AT+CIPSNTPTIME?", "[WIFI] Failed to update SNTP time.",
                    update_time);
    p = (char *)msg.Buffer;
    p += sizeof("+CIPSNTPTIME") - 1;
    if (*p != ':') {
        LOG("[WIFI] Command result broken while get SNTP time. "
            "Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto update_time;
    }
    p++;
    SetRTCTime(p);
    AT_FREE_RESP(msg);
    currTime          = GetRTCLastUpdate();
    char *time_buffer = ParseTimeInStr(currTime);
    PRINTF("[WIFI] Updated time from SNTP, Last Update: %s.\r\n", time_buffer);
    vPortFree(time_buffer);
update_time_finish:

    // Connect to Server
    retries = 0;
connect_to_server:
    PRINTF("[WIFI] Trying to connect to server.");
    if (retries != 0) {
        retry_intv = 100;
        PRINTF(" (retry times: %d).", retries);
    }
    PRINTF("\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        NET_WIFI_UART_PROC(msg.Buffer, msg.Len); // process active message
    }
    if (AT_GetNetStatus() != NET_CONNECTED) {
        LOG("[WIFI] Connection to network lost... Retry after 3 secs.");
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto failed;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+PING=\"" SERVER_ADDR "\"",
                    "[WIFI] Failed to ping to server.", connect_to_server);
    uint16_t ping;
    sscanf(msg.Buffer, "+PING:%hu", &ping);
    PRINTF("[WIFI] Ping to server " SERVER_ADDR " : %d ms.\r\n", ping);
    AT_FREE_RESP(msg);
    SEND_WAIT_CHECK("AT+CIPRECONNINTV=" NET_TCP_RECONNECT_INTV,
                    "[WIFI] Failed to set TCP reconnect interval.",
                    connect_to_server);
    AT_FREE_RESP(msg);
    SEND_WAIT_CHECK("AT+CIPSTART=\"TCP\",\"" SERVER_ADDR "\"," SERVER_PORT
                    "," NET_TCP_KEEPALIVE_TIME,
                    "[WIFI] Failed to connect to server " SERVER_ADDR
                    ":" SERVER_PORT,
                    connect_to_server);
    AT_FREE_RESP(msg);
    SEND_WAIT_CHECK("AT+CIPMODE=1",
                    "[WIFI] Failed to enter pass-through recv mode.",
                    connect_to_server);
    AT_FREE_RESP(msg);

    AT_SendStaticCommand("AT+CIPSEND\r\n");
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    if (msg.Buffer[msg.Len - 1] != '>') {
        LOG("[WIFI] Failed to enter full pass-through mode. Retry after 3 "
            "secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto connect_to_server;
    }
    AT_SetNetStatus(NET_CONNECTED_TO_SERVER);

    AT_UnregisterResponse(AT_Msg_Queue);
    vQueueDelete(AT_Msg_Queue);

    LOG("[WIFI] WiFi Module booted up.");
    vTaskDelay(pdMS_TO_TICKS(2000));
    vTaskDelete(NULL); // delete self
    return;

failed:
    LOG("[WIFI] Cannot boot up WiFi module. Retry after 3 secs.");
    AT_UnregisterResponse(AT_Msg_Queue);
    vQueueDelete(AT_Msg_Queue);

    vTaskDelay(pdMS_TO_TICKS(3000));
    goto retry_start;
}

void NET_WIFI_INIT() {
    // this Function not inside FreeRTOS Task
    xTaskCreate(task_wifi_init, "WIFI-INIT", 256, NULL, tskIDLE_PRIORITY, NULL);
}

// buffer need to be free
void NET_WIFI_UART_PROC(uint8_t *buffer, uint16_t len) {
    LOG("[NET] Received active message.");
    vPortFree(buffer);
}

#endif // NET_MODULE_ESP32