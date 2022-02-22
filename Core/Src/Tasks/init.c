/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: init.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "Common/init.h"
#include "Common/utils.h"
#include "Flash/flash.h"
#include "FreeRTOS.h"
#include "GUI/handles.h"
#include "task.h"
#include "fatfs.h"

extern void standby_task(); // standby.c

static BYTE FATBuffer[4096] = {0};
static BYTE TestData[]      = "Hello world, this is a fatfs test message.\n";
static BYTE work[4096];

void fatfs_test() {
    static FATFS   fs;
    static FIL     file;
    static FRESULT f_res;

    f_res = f_mount(&fs, "0:", 1);
    if (f_res != RES_OK) {
        f_res = f_mkfs("0:", FM_FAT, 0, work, sizeof(work));
        if (f_res != RES_OK) {
            LOG("[FATFS] Format disk failed.");
            return;
        }
        LOG("[FATFS] Format disk succeed.");
        f_res = f_mount(&fs, "0:", 1);
        if (f_res != RES_OK) {
            LOG("[FATFS] Mount disk failed.");
            return;
        }
    }
    LOG("[FATFS] Mount disk succeed.");

    f_res = f_open(&file, "0:test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (f_res != RES_OK) {
        LOG("[FATFS] Open(create) file failed.");
        return;
    }
    LOG("[FATFS] Open(create) file succeed.");

    UINT written;
    f_res = f_write(&file, TestData, sizeof(TestData), &written);
    if (f_res != RES_OK) {
        LOG("[FATFS] Write to file failed.");
        return;
    }
    LOGF("[FATFS] Write to file succeed, written size: %d.", written);

    f_res = f_close(&file);
    if (f_res != RES_OK) {
        LOG("[FATFS] Close file failed.");
        return;
    }
    LOG("[FATFS] Close file succeed");

    {
        DIR         dir;
        char       *path;
        UINT        BytesWritten;
        static char string[128];

        path = "0:"; // where you want to list

        f_res = f_opendir(&dir, path);

#ifdef DBG
        if (res != FR_OK)
            printf("res = %d f_opendir\n", res);
#endif

        if (f_res == FR_OK) {
            while (1) {
                FILINFO fno;

                f_res = f_readdir(&dir, &fno);

#ifdef DBG
                if (res != FR_OK)
                    printf("res = %d f_readdir\n", res);
#endif

                if ((f_res != FR_OK) || (fno.fname[0] == 0))
                    break;

                sprintf(string, "%c%c%c%c %10d %s/%s",
                        ((fno.fattrib & AM_DIR) ? 'D' : '-'),
                        ((fno.fattrib & AM_RDO) ? 'R' : '-'),
                        ((fno.fattrib & AM_SYS) ? 'S' : '-'),
                        ((fno.fattrib & AM_HID) ? 'H' : '-'), (int)fno.fsize,
                        path, fno.fname);

                PRINTF("%s\r\n", string);
            }
        }
    }

    FILINFO info;
    f_res = f_stat("0:test.txt", &info);

    if (f_res != RES_OK) {
        LOG("[FATFS] Stat file failed.");
        info.fsize = written;
    }
    LOGF("[FATFS] Stat succeed, file size: %d bytes.", info.fsize);

    f_res = f_open(&file, "0:test.txt", FA_READ);
    if (f_res != RES_OK) {
        LOG("[FATFS] Open(create) file failed.");
        return;
    }
    LOG("[FATFS] Open(create) file succeed.");

    f_res = f_read(&file, FATBuffer, info.fsize, &written);
    if (f_res != RES_OK) {
        LOG("[FATFS] Read file failed.");
        return;
    }
    LOGF("[FATFS] Read file succeed, read size: %d.", written);
    LOGF("[FATFS] Read result: %s.", FATBuffer);

    f_res = f_close(&file);
    if (f_res != RES_OK) {
        LOG("[FATFS] Close file failed.");
        return;
    }
    LOG("[FATFS] Close file succeed");
}

void init_task() {
    vTaskDelete(NULL);
}

void TASKS_INIT_RTOS() {
    LOG_SCR("[INIT] Starting init procedures.");
    LOG_SCR("[INIT] 测试中文显示。");
    //    set_long_message("……系统启动中……");
    set_long_message(
        "本洗车机为自助式服务，请用微信扫描屏幕右侧二维码，并按照说明使用。"
        "如有问题请电联1XXXXXXXXXX。感谢您的使用，祝您生活愉快。");
    LCD_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished LCD Init procedures.\n");
    NET_UART_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished Net UART Init procedures.\n");
    // Start Stand-by task
    xTaskCreate(standby_task, "STANDBY", 512, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(init_task, "INIT-TASK", 512, NULL, tskIDLE_PRIORITY, NULL);
    LOGF("[INIT] Testing SPI Flash, Chip ID: 0x%X", W25QXX_ReadID());
}