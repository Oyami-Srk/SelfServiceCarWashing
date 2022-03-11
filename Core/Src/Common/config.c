/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: config.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/config.h"
#include "Common/utils.h"
#include "FreeRTOS.h"
#include <stdio.h>

#ifdef CONFIG_USE_FATFS
#include "fatfs.h"
#endif

static bool        config_loaded = false;
static config_data configs;

static uint8_t *get_config_buffer(size_t *buffer_size) {
    if (!config_loaded) {
        *buffer_size = 0;
        return NULL;
    }

#ifdef CONFIG_USE_INTERNAL
    *buffer_size = sizeof(config_data);
    return (uint8_t *)configs;
#endif
#ifdef CONFIG_USE_FATFS
    size_t   size   = CONFIG_DATA_SERIALIZED_MAX_SIZE;
    uint8_t *buffer = (uint8_t *)pvPortMalloc(size);
    if (!buffer) {
        LOG("[CFG] Unable to allocate memory.");
        *buffer_size = 0;
        return NULL;
    }
    sprintf(
        (char *)buffer, CONFIG_DATA_SERIALIZED_FORMAT "\n", configs.BEGIN_GUARD,
#ifdef NET_MODULE_ESP32
        configs.CFG_NET_WIFI_AP_NAME, configs.CFG_NET_WIFI_AP_PSWD,
#endif
#ifdef NET_MODULE_LTE
        configs.CFG_NET_LTE_APN,
#endif
        configs.CFG_SERVER_ADDR, configs.CFG_SERVER_PORT,
        configs.CFG_SNTP_SERVER, configs.CFG_SNTP_UPD_INTV,
        configs.CFG_FOAM_TO_WATER_FRAC, configs.CFG_FLAGS, configs.END_GUARD);
    *buffer_size = strlen((char *)buffer);
    return buffer;
#endif
}

static bool load_config_from_fatfs() {
    static FIL     file;
    static FRESULT f_res;

    FILINFO info;
    f_res = f_stat(CONFIG_FATFS_FILENAME, &info);

    if (f_res != RES_OK) {
        return false;
    }
    LOGF("[FATFS] Found Config file " CONFIG_FATFS_FILENAME
         ", file size: %d bytes.",
         info.fsize);

    f_res = f_open(&file, CONFIG_FATFS_FILENAME, FA_READ);
    if (f_res != RES_OK) {
        LOG("[FATFS] Open Config file " CONFIG_FATFS_FILENAME " failed.");
        return false;
    }

    UINT     br;
    uint8_t *buffer = pvPortMalloc(info.fsize);
    if (!buffer) {
        LOG("[CFG] Unable to allocate memory.");
        f_close(&file);
        return false;
    }

    f_res = f_read(&file, buffer, info.fsize, &br);
    if (f_res != RES_OK) {
        LOG("[FATFS] Load Config file " CONFIG_FATFS_FILENAME " failed.");
        return false;
    }
    f_res = f_close(&file);

    sscanf((char *)buffer, CONFIG_DATA_SERIALIZED_FORMAT, &configs.BEGIN_GUARD,
#ifdef NET_MODULE_ESP32
           configs.CFG_NET_WIFI_AP_NAME, configs.CFG_NET_WIFI_AP_PSWD,
#endif
#ifdef NET_MODULE_LTE
           configs.CFG_NET_LTE_APN,
#endif
           configs.CFG_SERVER_ADDR, &configs.CFG_SERVER_PORT,
           configs.CFG_SNTP_SERVER, &configs.CFG_SNTP_UPD_INTV,
           &configs.CFG_FOAM_TO_WATER_FRAC, &configs.CFG_FLAGS,
           &configs.END_GUARD);

    return true;
}

static bool load_config_from_internal_flash() { return false; }

static bool save_config_to_fatfs() {
    static FIL     file;
    static FRESULT f_res;

    f_res = f_open(&file, CONFIG_FATFS_FILENAME, FA_CREATE_ALWAYS | FA_WRITE);
    if (f_res != RES_OK) {
        LOG("[FATFS] Open(create) file failed.");
        return false;
    }

    size_t   buffer_size;
    uint8_t *buffer = get_config_buffer(&buffer_size);
    UINT     written;

    f_res = f_write(&file, buffer, buffer_size, &written);
    if (f_res != RES_OK) {
        LOG("[FATFS] Write to file failed.");
        return false;
    }

    f_res = f_close(&file);
    return true;
}

static bool save_config_to_internal_flash() { return false; }

static bool is_guard_failed() {
    if (!config_loaded)
        return true;
    if (configs.BEGIN_GUARD == configs.END_GUARD &&
        configs.BEGIN_GUARD == CFG_GUARD_COOKIE)
        return false;
    return true;
}

void INIT_CONFIG() {
    if (
#ifdef CONFIG_USE_FATFS
        load_config_from_fatfs()
#else
        load_config_from_internal_flash()
#endif
    ) {
        config_loaded = true;
        return;
    } else {
        RESET_CONFIG();
    }
}

void *GET_CONFIG(config_selector sel) {
    if (!config_loaded) {
        INIT_CONFIG();
    }
    if (!config_loaded) {
        return NULL;
    }

    if (is_guard_failed()) {
        LOG("[CFG] Config memory corruption.");
        return NULL;
    }

    switch (sel) {
#ifdef NET_MODULE_ESP32
    case CFG_SEL_NET_WIFI_AP_NAME:
        return configs.CFG_NET_WIFI_AP_NAME;
    case CFG_SEL_NET_WIFI_AP_PSWD:
        return configs.CFG_NET_WIFI_AP_PSWD;
#endif
#ifdef NET_MODULE_LTE
    case CFG_SEL_NET_LTE_APN:
        return configs.CFG_NET_LTE_APN;
#endif
    case CFG_SEL_SERVER_ADDR:
        return configs.CFG_SERVER_ADDR;
    case CFG_SEL_SERVER_PORT:
        return (void *)(configs.CFG_SERVER_PORT + 0xFFFF0000);
    case CFG_SEL_SNTP_SERVER:
        return configs.CFG_SNTP_SERVER;
    case CFG_SEL_SNTP_UPD_INTV:
        return (void *)(configs.CFG_SNTP_UPD_INTV);
    case CFG_SEL_FLAGS:
        return (void *)(configs.CFG_FLAGS + 0xFFFF0000);
    case CFG_SEL_FOAM_TO_WATER_FRAC:
        return (void *)(configs.CFG_FOAM_TO_WATER_FRAC);
    default:
        return NULL;
    }
}

int SET_CONFIG(config_selector sel, void *data, size_t size) {
    if (is_guard_failed()) {
        LOG("[CFG] Config memory corruption.");
        return 1;
    }

    if (size > 64) {
        LOG("[CFG] Config too long to save.");
        return 2;
    }

    switch (sel) {
#ifdef NET_MODULE_ESP32
    case CFG_SEL_NET_WIFI_AP_NAME:
        memcpy(configs.CFG_NET_WIFI_AP_NAME, data, size);
        break;
    case CFG_SEL_NET_WIFI_AP_PSWD:
        memcpy(configs.CFG_NET_WIFI_AP_PSWD, data, size);
        break;
#endif
#ifdef NET_MODULE_LTE
    case CFG_SEL_NET_LTE_APN:
        memcpy(configs.CFG_NET_LTE_APN, data, size);
        break;
#endif
    case CFG_SEL_SERVER_ADDR:
        memcpy(configs.CFG_SERVER_ADDR, data, size);
        break;
    case CFG_SEL_SERVER_PORT:
        configs.CFG_SERVER_PORT = ((uint32_t)data) & 0xFFFF;
        break;
    case CFG_SEL_SNTP_SERVER:
        memcpy(configs.CFG_SNTP_SERVER, data, size);
        break;
    case CFG_SEL_SNTP_UPD_INTV:
        configs.CFG_SNTP_UPD_INTV = (uint32_t)data;
        break;
    case CFG_SEL_FLAGS:
        configs.CFG_FLAGS = ((uint32_t)data) & 0xFFFF;
        break;
    case CFG_SEL_FOAM_TO_WATER_FRAC:
        configs.CFG_FOAM_TO_WATER_FRAC = (uint32_t)data;
        break;
    default:
        return 3;
    }
    if (!
#ifdef CONFIG_USE_FATFS
        save_config_to_fatfs()
#else
        save_config_to_internal_flash()
#endif
    ) {
        LOG("[CFG] Failed to save configs.");
        return 4;
    }
    return 0;
}

void RESET_CONFIG() {
    configs.BEGIN_GUARD = configs.END_GUARD = CFG_GUARD_COOKIE;
#ifdef NET_MODULE_ESP32
    strcpy(configs.CFG_NET_WIFI_AP_NAME, CFG_DEFAULT_NET_WIFI_AP_NAME);
    strcpy(configs.CFG_NET_WIFI_AP_PSWD, CFG_DEFAULT_NET_WIFI_AP_PSWD);
#else
    strcpy(configs.CFG_NET_LTE_APN, CFG_DEFAULT_NET_LTE_APN);
#endif
    strcpy(configs.CFG_SERVER_ADDR, CFG_DEFAULT_SERVER_ADDR);
    configs.CFG_SERVER_PORT = CFG_DEFAULT_SERVER_PORT;
    strcpy(configs.CFG_SNTP_SERVER, CFG_DEFAULT_NET_SNTP_SERVER);
    configs.CFG_SNTP_UPD_INTV      = CFG_DEFAULT_RTC_MINIUM_UPDATE_INTV;
    configs.CFG_FOAM_TO_WATER_FRAC = CFG_DEFAULT_FOAM_TO_WATER_FRACTION;
    configs.CFG_FLAGS              = CFG_FLAG_FORCE_UPDATE_SNTP;
    config_loaded                  = true;
    if (!
#ifdef CONFIG_USE_FATFS
        save_config_to_fatfs()
#else
        save_config_to_internal_flash()
#endif
    ) {
        LOG("[CFG] Failed to save configs.");
    }
}
