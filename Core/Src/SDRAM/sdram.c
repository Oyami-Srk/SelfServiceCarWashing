/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: sdram.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#include "fmc.h"
#include "main.h"
#include <stdio.h>
#include "Common/init.h"

#define SDRAM_Size      0x01000000             // 16M字节
#define SDRAM_BANK_ADDR ((uint32_t)0xD0000000) // FMC SDRAM 数据基地址
#define FMC_COMMAND_TARGET_BANK                                                \
    FMC_SDRAM_CMD_TARGET_BANK2           //	SDRAM 的bank选择
#define SDRAM_TIMEOUT ((uint32_t)0x1000) // 超时判断时间

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

// 5ms roughly delay
void SDRAM_delay(__IO uint32_t nCount) {
    __IO uint32_t index;
    for (index = (100000 * nCount); index != 0; index--)
        ;
}

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef      *hsdram,
                                   FMC_SDRAM_CommandTypeDef *Command) {
    __IO uint32_t tmpmrd = 0;

    /* Configure a clock configuration enable command */
    Command->CommandMode   = FMC_SDRAM_CMD_CLK_ENABLE; // 开启SDRAM时钟
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT); // 发送控制指令
    SDRAM_delay(1);                                        // 延时等待

    /* Configure a PALL (precharge all) command */
    Command->CommandMode   = FMC_SDRAM_CMD_PALL;      // 预充电命令
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT); // 发送控制指令

    /* Configure a Auto-Refresh command */
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE; // 使用自动刷新
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    Command->AutoRefreshNumber      = 8;              // 自动刷新次数
    Command->ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT); // 发送控制指令

    /* Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_3 |
             SDRAM_MODEREG_OPERATING_MODE_STANDARD |
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE; // 加载模式寄存器命令
    Command->CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT); // 发送控制指令
    hsdram->Instance->SDRTR |= ((uint32_t)((1386) << 1)); // 设置刷新计数器
}

uint8_t SDRAM_Test(void) {
    uint32_t i;            // 计数变量
    uint32_t ReadData = 0; // 读取到的数据
    uint8_t  ReadData_8b;

    printf("[SYSTEM] Start SDRAM Self-Test.\r\n");
    for (i = 0; i < SDRAM_Size / 4; i++) {
        *(__IO uint32_t *)(SDRAM_BANK_ADDR + 4 * i) = i; // 写入数据
    }

    for (i = 0; i < SDRAM_Size / 4; i++) {
        ReadData =
            *(__IO uint32_t *)(SDRAM_BANK_ADDR + 4 * i); // 从SDRAM读出数据
        if (ReadData != i) //检测数据，若不相等，跳出函数,返回检测失败结果。
        {
            printf("[SYSTEM] SDRAM Self-Test Failed!!\r\n");
            return ERROR; // 返回失败标志
        }
    }

    for (i = 0; i < 255; i++) {
        *(__IO uint8_t *)(SDRAM_BANK_ADDR + i) = i;
    }
    for (i = 0; i < 255; i++) {
        ReadData_8b = *(__IO uint8_t *)(SDRAM_BANK_ADDR + i);
        if (ReadData_8b !=
            (uint8_t)i) //检测数据，若不相等，跳出函数,返回检测失败结果。
        {
            printf("[SYSTEM] SDRAM Self-Test Failed on 8bit width RW!!\r\n");
            printf("[SYSTEM] Please check the connection of NBL0 and "
                   "NBL1\r\n");
            return ERROR; // 返回失败标志
        }
    }
    printf("[SYSTEM] SDRAM Self-Test Passed.\r\n");
    return SUCCESS; // 返回成功标志
}

void SDRAM_INIT_FMC() {
    FMC_SDRAM_CommandTypeDef command;
    SDRAM_Initialization_Sequence(&hsdram1, &command);
    if (SDRAM_Test() != SUCCESS) {
        Error_Handler();
    }
}