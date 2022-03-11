/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: lte.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/config.h"
#include "Tasks/tasks.h"

#ifdef NET_MODULE_LTE

#include "Common/utils.h"
#include "FreeRTOS.h"
#include "Net/at.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

static uint8_t radio_strength;
static char    cmd_buffer[512];

void task_lte_init(__attribute__((unused)) void *args) {
    // TODO: Guard for connection lost
    int retry_intv;
retry_start:
    retry_intv = 3000;
    vTaskDelay(pdMS_TO_TICKS(100));
    LOG("[LTE] Booting LTE Module.");
    AT_ResetStatus();

    QueueHandle_t AT_Msg_Queue;
    AT_Msg_Queue = xQueueCreate(8, sizeof(AT_Response_Msg_t));

    if (AT_Msg_Queue == NULL) {
        LOG("[NET] Queue cannot be create.");
        Error_Handler();
    }

    AT_Response_Msg_t msg;
    AT_RegisterResponse(AT_Msg_Queue);

    // Reset the module

    HAL_GPIO_WritePin(GPIO(MDM_PWK), GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(20));
    HAL_GPIO_WritePin(GPIO(MDM_PWK), GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));
    HAL_GPIO_WritePin(GPIO(MDM_PWK), GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(700));
    HAL_GPIO_WritePin(GPIO(MDM_PWK), GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(2050));

    // Clean Queue
    do {
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    } while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0);

    HAL_GPIO_WritePin(GPIO(MDM_PWK), GPIO_PIN_SET);

    LOG("[LTE] Wait for Module powered up");
    do {
        AT_WAIT_FOR_RESP_WITH_DELAY(AT_Msg_Queue, msg, portMAX_DELAY);
        if (AT_GetResult(msg.Buffer, msg.Len) == AT_RDY) {
            AT_FREE_RESP(msg);
            break;
        }
        AT_FREE_RESP(msg);
    } while (true);

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
        AT_UnregisterResponse(AT_Msg_Queue);
        goto failed;
    }

#define SEND_WAIT_CHECK(cmd, message, label)                                   \
    AT_SendCommand((uint8_t *)(cmd), strlen(cmd));                             \
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
check_network:
    LOG_SCR("[LTE] Trying to connect to the LTE Network.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[LTE] Max retries exceeded while connect to network. Restart all "
            "initialization procedures.");
        goto failed;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        AT_FREE_RESP(msg);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    retry_intv = 3000;
    SEND_WAIT_CHECK("AT+CPIN?\r\n", "[LTE] No SIM Card Installed.",
                    check_network);
    AT_FREE_RESP(msg);

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CSQ\r\n", "[LTE] Cannot check Radio strength.",
                    check_network);
    radio_strength = 0;
    sscanf((char *)msg.Buffer, "\r\n+CSQ: %hhu", &radio_strength);

    retry_intv = 3000;
    if (radio_strength < 18) {
        LOGF("[LTE] Radio Strength is %d, that is too low to maintenance "
             "connection.",
             radio_strength);
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(retry_intv));
        goto check_network;
    }
    LOGF("[LTE] Radio strength is %d.", radio_strength);
    AT_FREE_RESP(msg);

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CREG?\r\n", "[LTE] Cannot check registration state.",
                    check_network);
    uint8_t a = 0, b = 0;
    sscanf((char *)msg.Buffer, "\r\n+CREG: %hhu,%hhu", &a, &b);
    if (b == 0) {
        LOG("[LTE] SIM Card Unregistered.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(retry_intv));
        goto check_network;
    }
    AT_FREE_RESP(msg);

    vTaskDelay(pdMS_TO_TICKS(100));
    SEND_WAIT_CHECK("AT+CGATT?\r\n", "[LTE] Cannot get attachment state.",
                    check_network);
    a = 0;
    sscanf((char *)msg.Buffer, "\r\n+CGATT: %hhu", &a);
    if (a == 0) {
        LOG("[LTE] Cannot attach to the network.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(retry_intv));
        goto check_network;
    }
    AT_FREE_RESP(msg);
    AT_SetNetStatus(NET_NO_IP);

    // Get IMSI
    retries = 0;
get_imsi:
    LOG_SCR("[LTE] Trying to get IMSI.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[LTE] Max retries exceeded while get IMSI. Restart all "
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
    SEND_WAIT_CHECK("AT+CIMI\r\n", "[LTE] Failed to get IMSI.", get_imsi);
    char imsi_buffer[16];
    memset(imsi_buffer, 0, 16);
    memcpy(imsi_buffer, msg.Buffer + 2, 15);
    AT_FREE_RESP(msg);

    PRINTF_SCR("[LTE] SIM Card IMSI: %s\r\n", imsi_buffer);
    AT_SetIdent(imsi_buffer);

    // Active TCP protocol
    retries = 0;
enter_tcp:
    LOG_SCR("[LTE] Trying to active the scene of TCP transmission.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[LTE] Max retries exceeded while get in tcp scene. Restart all "
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
    sprintf((char *)cmd_buffer, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n",
            (char *)GET_CONFIG(CFG_SEL_NET_LTE_APN));
    SEND_WAIT_CHECK(cmd_buffer, "[LTE] Failed to configure TCP scene.",
                    enter_tcp);
    AT_FREE_RESP(msg);

    SEND_WAIT_CHECK("AT+QIDEACT=1\r\n",
                    "[LTE] Failed to deactivate TCP scene...ignore.", tag1);
    AT_FREE_RESP(msg);
tag1:

    SEND_WAIT_CHECK("AT+QIACT=1\r\n", "[LTE] Failed to active TCP scene.",
                    enter_tcp);
    AT_FREE_RESP(msg);

    SEND_WAIT_CHECK("AT+QIACT?\r\n", "[LTE] Failed to get actived TCP scene.",
                    enter_tcp);

    char    *p   = msg.Buffer;
    uint16_t len = msg.Len;
    while (len && *p != '\"')
        len--, p++;
    if (len < 7) {
        LOG("[LTE] Broken Command while getting IP from actived TCP scene.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(retry_intv));
        goto enter_tcp;
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

    PRINTF_SCR("[LTE] IP Address: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    AT_SetIP(ip);
    AT_SetNetStatus(NET_CONNECTED);

    // Update time
    retries = 0;
update_time:;
    time_t lastUpdate = GetRTCLastUpdate();
    time_t currTime   = GetRTCTime();
    if (!(((uint16_t)((uint32_t)GET_CONFIG(CFG_SEL_FLAGS) & 0xFFFF)) &
          CFG_FLAG_FORCE_UPDATE_SNTP) &&
        lastUpdate != 0 &&
        lastUpdate - currTime < (uint32_t)GET_CONFIG(CFG_SEL_SNTP_UPD_INTV)) {
        char *time_buffer = ParseTimeInStr(lastUpdate);
        PRINTF_SCR("[LTE] RTC Time is updated at %s. Skip update.\r\n",
                   time_buffer);
        vPortFree(time_buffer);
        time_buffer = ParseTimeInStr(currTime);
        PRINTF_SCR("[LTE] Now RTC Time is %s.\r\n", time_buffer);
        vPortFree(time_buffer);
        goto update_time_finish;
    }
    LOG_SCR("[LTE] Trying to update time from NTP.");
    if (retries > NET_MAX_RETRIES) {
        LOG("[LTE] Max retries exceeded while update time. Restart all "
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

    sprintf((char *)cmd_buffer, "AT+QNTP=1,\"%s\"\r\n",
            (char *)GET_CONFIG(CFG_SEL_SNTP_SERVER));
    SEND_WAIT_CHECK(cmd_buffer, "[LTE] Failed to update time.", update_time);
    AT_FREE_RESP(msg);
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
    p = (char *)msg.Buffer;
    p += sizeof("\r\n+QNTP:") - 1;
    if (*p != ' ') {
        LOG("[LTE] Command result broken while get NTP time. "
            "Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto update_time;
    }
    p++;
    uint8_t ret = 0xFF;
    sscanf(p, "%hhu", &ret);
    if (ret != 0) {
        LOG("[LTE] Failed to get NTP time. "
            "Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto update_time;
    }
    while (*p != '\"')
        p++;
    p++;
    if (0 == SetRTCTime(p)) {
        LOG("[LTE] Failed to get NTP time. Retry after 3 secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto update_time;
    }
    AT_FREE_RESP(msg);
    currTime          = GetRTCLastUpdate();
    char *time_buffer = ParseTimeInStr(currTime);
    PRINTF("[LTE] Updated time from SNTP, Last Update: %s.\r\n", time_buffer);
    vPortFree(time_buffer);
update_time_finish:

    // Connect to Server
    retries = 0;
connect_to_server:
    PRINTF("[LTE] Trying to connect to server.");
    if (retries != 0) {
        if (retries > 3) {
            retry_intv = 10000;
        } else if (retries > 30) {
            retry_intv = 60000;
        } else if (retries > 50) {
            retry_intv = 60000 * 30;
        } else if (retries > 70) {
            PRINTF("[LTE] Server down......");
            vTaskDelay(pdMS_TO_TICKS(1000 * 60));
            Error_Handler();
        }
        PRINTF(" (retry times: %d).", retries);
    }
    PRINTF("\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    while (uxQueueMessagesWaiting(AT_Msg_Queue) != 0) {
        // clean queue
        AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);
        NET_MODULE_UART_PROC(msg.Buffer, msg.Len); // process active message
    }

    if (AT_GetNetStatus() != NET_CONNECTED) {
        LOG("[LTE] Connection to network lost... Retry after 3 secs.");
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto failed;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    // TODO: ping

    SEND_WAIT_CHECK("AT+QICLOSE=0\r\n",
                    "[LTE] Failed to close socket...ignore.", tag2);
    AT_FREE_RESP(msg);
tag2:

    sprintf((char *)cmd_buffer, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%hu,0,2\r\n",
            (char *)GET_CONFIG(CFG_SEL_SERVER_ADDR),
            (uint16_t)((uint32_t)GET_CONFIG(CFG_SEL_SERVER_PORT) & 0xFFFF));
    AT_SendCommand(cmd_buffer, strlen(cmd_buffer));
    AT_WAIT_FOR_RESP(AT_Msg_Queue, msg);

    if (!STATIC_STR_CMP(msg.Buffer, "\r\nCONNECT")) {
        LOG("[LTE] Failed to connect to server. Retry after 3 "
            "secs.");
        AT_FREE_RESP(msg);
        retries++;
        vTaskDelay(pdMS_TO_TICKS(3000));
        goto connect_to_server;
    }

    AT_FREE_RESP(msg);

    AT_SetNetStatus(NET_CONNECTED_TO_SERVER);

    AT_UnregisterResponse(AT_Msg_Queue);
    vQueueDelete(AT_Msg_Queue);

    LOG_SCR("[LTE] LTE Module booted up.");
    put_text_on_loading_scr("\n\n\n\n\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    vTaskDelete(NULL); // delete self
    return;

failed:
    LOG("[LTE] Cannot boot up LTE module. Retry after 3 secs.");
    AT_UnregisterResponse(AT_Msg_Queue);
    vQueueDelete(AT_Msg_Queue);

    vTaskDelay(pdMS_TO_TICKS(3000));
    goto retry_start;
}

uint8_t NET_MODULE_GET_RADIO_STRENGTH() { return radio_strength; }

void NET_MODULE_INIT() {
    // this Function not inside FreeRTOS Task
    xTaskCreate(task_lte_init, "LTE-INIT", 1024, NULL, tskIDLE_PRIORITY, NULL);
}

// buffer need to be free
void NET_MODULE_UART_PROC(uint8_t *buffer, uint16_t len) {
    LOGF("[NET] Received active message (length %d).", len);
    if (buffer[0] == '+') {
        // server command
        // Free in ProcessActive
        Cmd_ProcessActive(buffer, len);
        return;
    } else {
        if (STATIC_STR_CMP(buffer, "\r\nNO CARRIER")) {
            LOG("[LTE] Connection Broken.");
            AT_SetNetStatus(NET_NOT_CONNECT);

            xTaskCreate(task_lte_init, "LTE-INIT", 512, NULL, tskIDLE_PRIORITY,
                        NULL);
        }
    }
    vPortFree(buffer);
}

#endif // NET_MODULE_LTE
