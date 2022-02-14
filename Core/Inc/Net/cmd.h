/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: cmd.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __NET_CMD_H__
#define __NET_CMD_H__

#include <stdint-gcc.h>

// Device Command
#define DEV_CMD_REGISTER "+CTRLCMD REGISTER %s %hhu.%hhu.%hhu.%hhu %ld %s %d\n"
#define DEV_CMD_LOGIN    "+CTRLCMD USER_LOGIN %s %s\n"
#define DEV_CMD_LOGOUT   "+CTRLCMD USER_LOGOUT %s %f %f %ld\n"
#define DEV_CMD_QRCODE   "+CTRLCMD ACQUIRE_QRCODE\n"
#define DEV_CMD_HEARTBEAT                                                      \
    "+CTRLCMD HEARTBEAT %s %hhu.%hhu.%hhu.%hhu %ld %s %d\n"
/* TODO: More command
 * DEV_CMD_REQUIRE_FIRMWARE_UPGRADE
 * DEV_CMD_DEVICE_FAIL
 */

// Server response
#define SRV_RESP          "+SERVRESP %c"
#define SRV_RESP_LOGIN_OK "+SERVRESP OK %s %f\n"
// Server Command
// Device response

typedef enum {
    CMD_RESULT_OK,
    CMD_RESULT_ERROR,
    CMD_RESULT_UNKNOWN,
} CMD_RESULT;

CMD_RESULT Cmd_GetRespResult(const char *msg);
CMD_RESULT Cmd_RegisterDevice();
CMD_RESULT Cmd_UserLogin(const char *username, const char *password,
                         char *userId, float *avail);
CMD_RESULT Cmd_UserLogOut(const char *userId, float water_used, float foam_used,
                          uint32_t time_used);
CMD_RESULT Cmd_AcquireQRCode();
CMD_RESULT Cmd_HeartBeat();

#endif // __NET_CMD_H__