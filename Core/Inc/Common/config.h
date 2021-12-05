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

#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#define LED_GPIO_GROUP     GPIOH
#define LED_NET_STATUS     GPIOH, WORKING_STATUS_Pin
#define LED_WORKING_STATUS GPIOH, NET_STATUS_LED_Pin

#endif // __COMMON_CONFIG_H__
