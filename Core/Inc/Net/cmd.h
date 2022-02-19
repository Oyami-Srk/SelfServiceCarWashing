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
#define DEV_CMD_REGISTER "+CTRLCMD REGISTER %s %hhu.%hhu.%hhu.%hhu %lu %s %d\n"
#define DEV_CMD_LOGIN    "+CTRLCMD USER_LOGIN %s %s %s\n"
#define DEV_CMD_LOGOUT   "+CTRLCMD USER_LOGOUT %s %s %f %f %ld\n"
#define DEV_CMD_QRCODE   "+CTRLCMD ACQUIRE_QRCODE %s\n"
#define DEV_CMD_HEARTBEAT                                                      \
    "+CTRLCMD HEARTBEAT %s %hhu.%hhu.%hhu.%hhu %ld %s %d\n"
/* TODO: More command
 * DEV_CMD_REQUIRE_FIRMWARE_UPGRADE
 * DEV_CMD_DEVICE_FAIL
 */

// Server response
#define SRV_RESP          "+SERVRESP %c"
#define SRV_RESP_LOGIN_OK "%s %s %f\n"
// Server Command
#define SRV_CMD_HEAD         "+SERVCMD"
#define SRV_CMD_GETSTATUS    "GET_STATUS"
#define SRV_CMD_LOGIN        "USER_LOGIN"
#define SRV_CMD_LOGIN_PARAM  "%s %s %f %s\n"
#define SRV_CMD_LOGOUT       "USER_LOGOUT"
#define SRV_CMD_LOGOUT_PARAM "%s\n"
#define SRV_CMD_RESET        "RESET"
#define SRV_CMD_SHUTDOWN     "SHUTDOWN"
#define SRV_CMD_START        "SERVICE_START"
#define SRV_CMD_STOP         "SERVICE_STOP"
#define SRV_CMD_DISPOFF      "DISPLAY_TURNOFF"
#define SRV_CMD_DISPON       "DISPLAY_TURNON"
#define SRV_CMD_DISPMSG      "DISPLAY_MESSAGE"
#define SRV_CMD_QRCODE       "DISPLAY_QRCODE"
#define SRV_CMD_SETCONF      "SET_CONF"
// Device response
#define DEV_RESP_OK        "+CTRLRESP OK"
#define DEV_RESP_ERROR     "+CTRLRESP ERROR"
#define DEV_RESP_GETSTATUS DEV_RESP_OK " %s %s %hhu.%hhu.%hhu.%hhu %lu %s %d %d"

typedef enum {
    CMD_RESULT_OK,
    CMD_RESULT_ERROR,
    CMD_RESULT_UNKNOWN,
} CMD_RESULT;

CMD_RESULT Cmd_GetRespResult(const char *msg);
CMD_RESULT Cmd_RegisterDevice();
CMD_RESULT Cmd_UserLogin(const char *username, const char *password,
                         char *userId, char *user_dispname, float *avail);
CMD_RESULT Cmd_UserLogOut(const char *userId, float water_used, float foam_used,
                          uint32_t time_used);
CMD_RESULT Cmd_AcquireQRCode();
CMD_RESULT Cmd_HeartBeat();

CMD_RESULT Srv_GetStatus(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_UserLogin(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_UserLogout(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_Reset(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_Shutdown(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_ServiceStop(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_ServiceStart(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_DispOff(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_DispOn(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_DispMessage(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_DispQRCode(uint8_t *recv_buffer, uint16_t len);
CMD_RESULT Srv_SetConf(uint8_t *recv_buffer, uint16_t len);

#endif // __NET_CMD_H__