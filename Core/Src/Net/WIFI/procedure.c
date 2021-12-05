/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: procedure.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Common/config.h"
#include "Common/status.h"
#include "Common/utils.h"
#include "Net/AT/command.h"
#include "Net/AT/utils.h"
#include "Net/config.h"
#include "rtc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern uint8_t NET_STATUS;
extern char    NET_WIFI_MAC[18];
extern uint8_t NET_WIFI_IPV4[0];

static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed",
                                    "Thu", "Fri", "Sat"};
static const char mon_name[][4]  = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

uint8_t WIFI_CONNECT_TO_AP(uint8_t *buffer, uint16_t len, uint8_t step,
                           Status_t *status) {
    *status = Failed;
    switch (step) {
    case 0: {
        // initial step
        HAL_Delay(200);
        NET_AT_SEND_STATIC_CMD("AT+CWMODE=1\r\n"); // enter station mode
        *status = OK;
        break;
    }
    case 1: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Set station mode failed.\n");
            *status = Failed;
            break;
        }
        printf(
            "[NET/WIFI] Wait 1 sec for trying to connect to AP: " WIFI_AP_NAME
            ".\n");
        HAL_Delay(200);
        NET_AT_SEND_STATIC_CMD("AT+CWJAP=\"" WIFI_AP_NAME "\",\"" WIFI_AP_PSWD
                               "\"\r\n");
        *status = OK;
        break;
    }
    case 2: {
        if (STATIC_CHAR_CMP(buffer, "+CWJAP") == 0) {
            // got err
            uint8_t err_code;
            sscanf(buffer, "+CWJAP:%hhu", &err_code);
            printf("[NET/WIFI] Connect to AP Failed: ");
            switch (err_code) {
            case 1:
                printf("Time out.\n");
                break;
            case 2:
                printf("Password Incorrect.\n");
                break;
            case 3:
                printf("Target AP " WIFI_AP_NAME " Not Found.\n");
                break;
            case 4:
                printf("Connected Failed. (code 4)\n");
                break;
            default:
                printf("Unknown Error. (code %d)\n", err_code);
                break;
            }
            *status = Failed;
            break;
        }
        if (STATIC_CHAR_CMP(buffer, "WIFI") != 0) {
            *status = OK;
            return 2;
        }
        if (NET_STATUS == NET_STATUS_NOT_CONNECTED &&
            STATIC_CHAR_CMP(buffer + 5, "DISCONNECT") == 0) {
            *status = OK;
            return 2;
        }
        if (STATIC_CHAR_CMP(buffer + 5, "CONNECTED") == 0) {
            printf("[NET/WIFI] WiFi Connected. Wait for IP.\n");
            *status = OK;
            return 2;
        }
        if (STATIC_CHAR_CMP(buffer + 5, "GOT IP") == 0) {
            HAL_GPIO_TogglePin(LED_NET_STATUS);
            printf("[NET/WIFI] WiFi Got IP.\n");
            NET_STATUS = NET_STATUS_GOT_IP;
            *status    = OK;
            NET_AT_SEND_STATIC_CMD("AT+CIPSTAMAC?\r\n");
        }
        if (STATIC_CHAR_CMP(buffer, "busy") == 0) {
            *status = OK;
            return 2;
        }
        break;
    }
    case 3: {
        // Get mac
        if (AT_GET_RESULT(buffer, len) != AT_OK)
            break;
        *status = OK;

        char *p = buffer;
        p += sizeof("+CIPSTAMAC:") - 1;
        if (*p != '\"') {
            printf("[NET/WIFI] CMD Result broken.\n");
            *status = Failed;
            break;
        }
        memcpy(NET_WIFI_MAC, p + 1, 17);
        NET_WIFI_MAC[17] = 0;
        for (p = NET_WIFI_MAC; p < NET_WIFI_MAC + 17; p++)
            *p = toupper(*p);
        printf("[NET/WIFI] Device MAC: %s\n", NET_WIFI_MAC);
        NET_AT_SEND_STATIC_CMD("AT+CIPSTA?\r\n");
        HAL_Delay(200);
        break;
    }
    case 4: {
        // Get IP
        if (AT_GET_RESULT(buffer, len) != AT_OK)
            break;
        *status = OK;

        char *p = buffer;
        p += sizeof("+CIPSTA:ip:") - 1;
        if (*p != '\"') {
            printf("[NET/WIFI] CMD Result broken.\n");
            *status = Failed;
            break;
        }

        char ipbuffer[4] = {0};
        p++;
        for (int i = 0; i < 4; i++) {
            char *j = ipbuffer;
            while (*p != '.' && *p != '"') {
                *j++ = *p++;
            }
            p++;
            *(j++)           = '\0';
            NET_WIFI_IPV4[i] = atoi(ipbuffer);
        }
        printf("[NET/WIFI] Device IP: %d.%d.%d.%d\n", NET_WIFI_IPV4[0],
               NET_WIFI_IPV4[1], NET_WIFI_IPV4[2], NET_WIFI_IPV4[3]);
        return 0;
    }
    }
    return step + 1;
}

uint8_t WIFI_UPDATE_TIME(uint8_t *buffer, uint16_t len, uint8_t step,
                         Status_t *status) {
    *status = Failed;
    switch (step) {
    case 0: {
        // initial step
        HAL_Delay(200);
        NET_AT_SEND_STATIC_CMD("AT+CIPSNTPCFG=1,8,\"" SNTP_SERVER
                               "\"\r\n"); // set sntp server
        *status = OK;
        break;
    }
    case 1: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot set sntp server.\n");
            break;
        }
        HAL_Delay(5000); // 5 sec to sync
        NET_AT_SEND_STATIC_CMD("AT+CIPSNTPTIME?\r\n");
        *status = OK;
        break;
    }
    case 2: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot get sntp time.\n");
            break;
        }

        char *p = buffer;
        p += sizeof("+CIPSNTPTIME") - 1;
        if (*p != ':') {
            printf("[NET/WIFI] CMD Result broken.\n");
            *status = Failed;
            break;
        }
        p++;
        char            weekday[4] = {0};
        char            month[4]   = {0};
        RTC_DateTypeDef date;
        RTC_TimeTypeDef time;
        uint16_t        year;
        // this code may fail in year 21xx
        sscanf(p, "%s %s %hhu %hhu:%hhu:%hhu %hu", weekday, month, &date.Date,
               &time.Hours, &time.Minutes, &time.Seconds, &year);

        if (year < 2021) {
            *status = Failed;
            break;
        }
        date.Year = year - 2000;

        for (int i = 0; i < 7; i++) {
            if (strcmp(weekday, wday_name[i]) == 0) {
                date.WeekDay = i;
                break;
            }
        }
        for (int i = 0; i < 12; i++) {
            if (strcmp(month, mon_name[i]) == 0) {
                date.Month = i + 1;
                break;
            }
        }
        HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xF103);
        memset(&time, 0, sizeof(RTC_TimeTypeDef));
        memset(&date, 0, sizeof(RTC_DateTypeDef));
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

        printf("[NET/WIFI] NTP Time Updated: 20%02d/%02d/%02d %02d:%02d:%02d\n",
               date.Year, date.Month, date.WeekDay, time.Hours, time.Minutes,
               time.Seconds);
        *status = OK;
        return 0;
    }
    }
    return step + 1;
}

uint8_t WIFI_CONNECT_TO_SERVER(uint8_t *buffer, uint16_t len, uint8_t step,
                               Status_t *status) {
    *status = Failed;
    switch (step) {
    case 0: {
        // initial step
        NET_AT_SEND_STATIC_CMD("AT+PING=\"" SERV_ADDR "\"\r\n");
        *status = OK;
        break;
    }
    case 1: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot ping to server " SERV_ADDR "\n");
            break;
        }

        char    *p = buffer;
        uint16_t ping_time;
        sscanf(p, "+PING:%hu", &ping_time);
        printf("[NET/WIFI] Ping to server " SERV_ADDR ": %d ms.\n", ping_time);
        *status = OK;
        NET_AT_SEND_STATIC_CMD("AT+CIPRECONNINTV=" TCP_RECONNECT_INTERVAL
                               "\r\n");
        break;
    }
    case 2: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot set TCP reconnect interval.\n");
            break;
        }
        NET_AT_SEND_STATIC_CMD("AT+CIPSTART=\"TCP\",\"" SERV_ADDR
                               "\"," SERV_PORT "," TCP_KEEPALIVE_TIME "\r\n");
        *status = OK;
        break;
    }
    case 3: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot connect to server " SERV_ADDR
                   ":" SERV_PORT ".\n");
            break;
        }
        *status = OK;
        NET_AT_SEND_STATIC_CMD(
            "AT+CIPMODE=1\r\n"); // enter pass-through recv mode
        break;
    }
    case 4: {
        if (AT_GET_RESULT(buffer, len) != AT_OK) {
            printf("[NET/WIFI] Cannot enter pass-through recv mode. Try "
                   "again..\n");
            HAL_Delay(1000);
            NET_AT_SEND_STATIC_CMD("AT+CIPMODE=1\r\n");
            *status = OK;
            return 4;
        }
        *status = OK;
        NET_AT_SEND_STATIC_CMD("AT+CIPSEND\r\n");
        break;
    }
    case 5: {
        if (buffer[len - 1] == '>') {
            printf("[NET/WIFI] Enter full pass-through mode.\n");
            NET_STATUS = NET_STATUS_LINK_ESTABLISHED;
            *status    = OK;
            return 0;
        }
        break;
    }
    }
    return step + 1;
}
