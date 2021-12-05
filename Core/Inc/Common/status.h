/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCashing
 * File: status.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __STATUS_H__
#define __STATUS_H__

typedef enum {
    Failed    = 0,
    QueueFull = 1,
    OK        = 0xFFFF,
} Status_t;

#define NET_STATUS_DEVICE_FAIL      0xFF
#define NET_STATUS_NOT_CONNECTED    0x00
#define NET_STATUS_GOT_IP           0x01
#define NET_STATUS_LINK_ESTABLISHED 0x02

#endif // __STATUS_H__
