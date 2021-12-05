/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: handles.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __NET_WIFI_HANDLES_H__
#define __NET_WIFI_HANDLES_H__

typedef struct {
    uint8_t  MSG_TYPE;
    uint8_t  OPERATE;
    char    *args;
    uint16_t argv;
} __attribute__((packed)) NET_WIFI_REQUEST_MSG;

// initialize wifi module
// Call it after AT initialize finished.
void NET_WIFI_INIT(void);

#endif // __NET_WIFI_HANDLES_H__
