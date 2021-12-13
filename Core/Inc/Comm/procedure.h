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

#define CMD_RESULT_TIMEOUT  0x00
#define CMD_RESULT_OK       0x01
#define CMD_RESULT_ERROR    0x02
#define CMD_RESULT_OVERFLOW 0x04

uint8_t REGISTER_DEVICE();
uint8_t ACQUIRE_QRCODE();
uint8_t USER_LOGOUT(const char *userid, float water_used, float foam_used,
                    uint32_t time_used);
uint8_t USER_LOGIN(const char *username, const char *password, char *userid,
                   float *avail);

#endif // __COMM_PROCEDURE_H__
