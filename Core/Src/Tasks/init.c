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

uint8_t sFlash_WriteBuffer[sFLASH_SPI_PAGESIZE]; //	写数据数组
uint8_t sFlash_ReadBuffer[sFLASH_SPI_PAGESIZE];  //	读数据数组

uint8_t sFlash_Test(void) // Flash读写测试
{
    uint16_t i = 0;
    for (i = 0; i < sFLASH_SPI_PAGESIZE; i++) //将要写入Flash的数据写入数组
    {
        sFlash_WriteBuffer[i] = i;
    }
    sFLASH_EraseSector(0x000000); // 擦除扇区，FLASH写入前要先擦除
    sFLASH_WriteBuffer(sFlash_WriteBuffer, 0x000000,
                       sFLASH_SPI_PAGESIZE); // 从地址0x000000写入数据
    sFLASH_ReadBuffer(sFlash_ReadBuffer, 0x000000,
                      sFLASH_SPI_PAGESIZE); // 从地址0x000000读取数据

    for (i = 0; i < sFLASH_SPI_PAGESIZE; i++) //验证读出的数据是否等于写入的数据
    {
        if (sFlash_WriteBuffer[i] !=
            sFlash_ReadBuffer[i]) //如果数据不相等，则返回0
            return 0;
    }
    return 1; //数据正确，读写测试通过，返回1
}

BYTE FATBuffer[4096] = {0};
BYTE TestData[]      = "Hello world, this is a fatfs test message.\n";
BYTE work[4096];

void fatfs_test() {
    FATFS   fs;
    FIL     file;
    FRESULT f_res;
    f_res = f_mkfs("0:", FM_ANY, 0, work, sizeof(work));
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
    LOG("[FATFS] Mount disk succeed.");

    f_res = f_open(&file, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
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
        DIR   dir;
        char *path;
        UINT  BytesWritten;
        char  string[128];

        path = "0:/"; // where you want to list

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
    f_res = f_stat("0:/test.txt", &info);

    if (f_res != RES_OK) {
        LOG("[FATFS] Stat file failed.");
        info.fsize = written;
    }
    LOGF("[FATFS] Stat succeed, file size: %d bytes.", info.fsize);

    f_res = f_open(&file, "0:/test.txt", FA_READ);
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

void TASKS_INIT_RTOS() {
    LOG_SCR("[INIT] Starting init procedures.");
    LOG_SCR("[INIT] 测试中文显示。");
    set_long_message("……系统启动中……");
    LCD_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished LCD Init procedures.\n");
    NET_UART_INIT_RTOS();
    put_text_on_loading_scr("[Init] Finished Net UART Init procedures.\n");
    // Start Stand-by task
    xTaskCreate(standby_task, "STANDBY", 512, NULL, tskIDLE_PRIORITY, NULL);
    LOGF("[INIT] Testing SPI Flash, Chip ID: 0x%X", sFLASH_ReadID());
    if (sFLASH_ReadID() == sFLASH_ID) {
        if (sFlash_Test() == 1)
            LOG("[INIT] SPI Flash Test OK.");
        else
            LOG("[INIT] SPI Flash Test FAILED.");
    } else {
        LOG("[INIT] SPI Flash chip W25Q64 undetected.");
    }

    fatfs_test();
}