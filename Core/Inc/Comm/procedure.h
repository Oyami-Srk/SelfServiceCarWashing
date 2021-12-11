/*
 * Copyright (c) 2021 Shiroko
 * Project: SelfServiceCarWashing
 * File: procedure.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#ifndef __COMM_PROCEDURE_H__
#define __COMM_PROCEDURE_H__

#include <stdint.h>

#define CMD_RESULT_OK      0x01
#define CMD_RESULT_ERROR   0x02
#define CMD_RESULT_TIMEOUT 0x00

uint8_t REGISTER_DEVICE();

#endif // __COMM_PROCEDURE_H__
