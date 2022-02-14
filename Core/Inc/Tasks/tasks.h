/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: tasks.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __TASKS_TASKS_H__
#define __TASKS_TASKS_H__

#include <stdint-gcc.h>

void Cmd_ProcessActive(uint8_t *buffer, uint16_t len);

void start_inuse_task(const char *userId, float avail);
void stop_inuse_task();

#endif // __TASKS_TASKS_H__