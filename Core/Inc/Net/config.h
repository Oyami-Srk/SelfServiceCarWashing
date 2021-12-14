/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: config.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_CONFIG_H__
#define __NET_CONFIG_H__

//#define WIFI_AP_NAME "WiFi501"
//#define WIFI_AP_PSWD "1145141919810"
#define WIFI_AP_NAME "QIT-2.4G"
#define WIFI_AP_PSWD ""

#define SNTP_SERVER            "114.118.7.163"
#define TCP_KEEPALIVE_TIME     "30" // sec, max 7200
#define TCP_RECONNECT_INTERVAL "10" // 100ms, max 36000

//#define SERV_ADDR "192.168.3.42"
#define SERV_ADDR "192.168.14.50"
#define SERV_PORT "44551"

#define NET_BUFFER_SIZE 4096

#endif // __NET_CONFIG_H__