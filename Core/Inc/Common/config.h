/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: config.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__

#include "main.h"

/* Static defines */
#define SOFTWARE_VER "0.3.0"

//#define DEBUG_PRINT_USE_RTT
#define DEBUG_PRINT_USE_UART1

//#define NET_UART_USE_1
//#define NET_UART_USE_2
#define NET_UART_USE_3

//#define NET_MODULE_ESP32
#define NET_MODULE_LTE

#define ENABLE_NET_BUFFER_PRINT 1

#define ENABLE_NET_LED     1
#define ENABLE_WORKING_LED 1

#define RTC_COOKIE 0x1234F429
#define CFG_COOKIE 0xCOODF429

#define INUSE_SAMPLE_RATE     100                      // 100 ms sample rate
#define FLOW_SPEED_CALC(freq) (((freq) + 3.0f) / 8.1f) // Freq: Hz

#define ENABLE_HEARTBEAT 1
#define HEARTBEAT_INTV   10000 // in ms

#define NET_MAX_RETRIES 3

#ifdef NET_MODULE_ESP32
#define NET_TCP_KEEPALIVE_TIME "30" // sec
#define NET_TCP_RECONNECT_INTV "10" // 100ms
#endif

#define CONFIG_USE_FATFS
//#define CONFIG_USE_INTERNAL

#ifdef CONFIG_USE_FATFS
#ifdef CONFIG_USE_INTERNAL
#error "You can only chose one location to save config files."
#endif
#define CONFIG_FATFS_FILENAME "0:config.txt"
#endif

#ifdef CONFIG_USE_INTERNAL
#define CONFIG_INTERNAL_PG_SIZE 1024
#endif
/* End of Static defines */

typedef enum {
#ifdef NET_MODULE_ESP32
    CFG_SEL_NET_WIFI_AP_NAME = 1,
    CFG_SEL_NET_WIFI_AP_PSWD = 2,
#endif
#ifdef NET_MODULE_LTE
    CFG_SEL_NET_LTE_APN = 3,
#endif
    CFG_SEL_SERVER_ADDR   = 4,
    CFG_SEL_SERVER_PORT   = 5,
    CFG_SEL_SNTP_SERVER   = 6,
    CFG_SEL_SNTP_UPD_INTV = 7,
    CFG_SEL_FLAGS         = 8,

    CFG_SEL_FOAM_TO_WATER_FRAC = 9
} config_selector;

#define CONFIG_DATA_MAX_BYTES 512

#define CFG_FLAG_FORCE_UPDATE_SNTP  0x0001
#define CFG_FLAG_RESTORE_TO_DEFAULT 0x0002

#define CFG_GUARD_COOKIE 0xA5

typedef struct {
    uint8_t BEGIN_GUARD;
    // Configs
#ifdef NET_MODULE_ESP32
    char CFG_NET_WIFI_AP_NAME[64];
    char CFG_NET_WIFI_AP_PSWD[64];
#endif
#ifdef NET_MODULE_LTE
    char CFG_NET_LTE_APN[64];
#endif
    char     CFG_SERVER_ADDR[64];
    uint16_t CFG_SERVER_PORT;
    char     CFG_SNTP_SERVER[64];
    uint32_t CFG_SNTP_UPD_INTV;
    uint32_t CFG_FOAM_TO_WATER_FRAC;

    uint16_t CFG_FLAGS;
    // End Configs
    uint8_t END_GUARD;
} config_data __attribute__((aligned(16))); // aligned 16 for flash write

#ifdef CONFIG_USE_FATFS
#define CONFIG_DATA_SERIALIZED_MAX_SIZE                                        \
    (64 * 5 + 10 + 10 + 6 + 6 + 8 + 1 + 9) // 370 bytes
#ifdef NET_MODULE_ESP32
#define CONFIG_DATA_SERIALIZED_FORMAT_ESP32_PRINT "%s,%s,"
#define CONFIG_DATA_SERIALIZED_FORMAT_ESP32_SCAN  "%[^,],%[^,],"
#else
#define CONFIG_DATA_SERIALIZED_FORMAT_ESP32_PRINT ""
#define CONFIG_DATA_SERIALIZED_FORMAT_ESP32_SCAN  ""
#endif
#ifdef NET_MODULE_LTE
#define CONFIG_DATA_SERIALIZED_FORMAT_LTE_PRINT "%s,"
#define CONFIG_DATA_SERIALIZED_FORMAT_LTE_SCAN  "%[^,],"
#else
#define CONFIG_DATA_SERIALIZED_FORMAT_LTE_PRINT ""
#define CONFIG_DATA_SERIALIZED_FORMAT_LTE_SCAN  ""
#endif

#define CONFIG_DATA_SERIALIZED_FORMAT_SCAN                                     \
    "%hhu," CONFIG_DATA_SERIALIZED_FORMAT_ESP32_SCAN                           \
        CONFIG_DATA_SERIALIZED_FORMAT_LTE_SCAN                                 \
    "%[^,],%hu,%[^,],%lu,%lu,%hu,%hhu"

#define CONFIG_DATA_SERIALIZED_FORMAT_PRINT                                    \
    "%hhu," CONFIG_DATA_SERIALIZED_FORMAT_ESP32_PRINT                          \
        CONFIG_DATA_SERIALIZED_FORMAT_LTE_PRINT "%s,%hu,%s,%lu,%lu,%hu,%hhu"
#endif

/* Default dynamic configs */
#ifdef NET_MODULE_ESP32
//#define CFG_DEFAULT_NET_WIFI_AP_NAME "WiFi501"
//#define CFG_DEFAULT_NET_WIFI_AP_PSWD "1145141919810"
#define CFG_DEFAULT_NET_WIFI_AP_NAME "Wireless-2.4GHz"
#define CFG_DEFAULT_NET_WIFI_AP_PSWD "83885877"
#define CFG_DEFAULT_SERVER_ADDR      "192.168.0.1"
#else
#define CFG_DEFAULT_NET_LTE_APN "CMNET"
#define CFG_DEFAULT_SERVER_ADDR "srk01.qvq.moe"
#endif

#define CFG_DEFAULT_NET_SNTP_SERVER "114.118.7.163"
#define CFG_DEFAULT_SERVER_PORT     44551

#define CFG_DEFAULT_RTC_MINIUM_UPDATE_INTV (3600 * 4) // 4 hour
#define CFG_DEFAULT_FLAG                   0

#define CFG_DEFAULT_FOAM_TO_WATER_FRACTION 1500

/* End of Dynamic default */

#define FORCE_RESET_CONFIG 1
// #define FORCE_FORMAT_FLASH 1
#define DO_NOT_LOAD_FONT_FROM_FLASH 1
// Type is decided by selector, uint16_t or uint32_t return direct value instead
// of pointer
void  INIT_CONFIG();
void *GET_CONFIG(config_selector sel);
int   SET_CONFIG(config_selector sel, void *data, size_t size);
void  RESET_CONFIG();

#endif // __COMMON_CONFIG_H__
