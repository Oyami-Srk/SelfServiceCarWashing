/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: flash.h
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 * ================================
 */

#ifndef __FLASH_FLASH_H__
#define __FLASH_FLASH_H__

#include "stm32f4xx_hal.h"

#define sFLASH_CMD_WRITE  0x02 /* Write to Memory instruction */
#define sFLASH_CMD_WRSR   0x01 /* Write Status Register instruction */
#define sFLASH_CMD_WREN   0x06 /* Write enable instruction */
#define sFLASH_CMD_READ   0x03 /* Read from Memory instruction */
#define sFLASH_CMD_RDSR   0x05 /* Read Status Register instruction  */
#define sFLASH_CMD_RDID   0x9F /* Read identification */
#define sFLASH_CMD_SE     0x20 /* Sector Erase instruction */
#define sFLASH_CMD_BE     0xC7 /* Bulk Erase instruction */
#define sFLASH_WIP_FLAG   0x01 /* Write In Progress (WIP) flag */
#define sFLASH_DUMMY_BYTE 0xA5

#define sFLASH_SPI_PAGESIZE 256

#define sFLASH_ID 0XEF4017 // W25Q64
//#define  sFLASH_ID                0XEF4018     //W25Q128

/*-------------------------------------------- SPI1配置宏
 * ---------------------------------------*/

#define SPI1_SCK_PIN  GPIO_PIN_3 // SPI1_SCK 引脚
#define SPI1_SCK_PORT GPIOB      // SPI1_SCK 引脚端口
#define GPIO_SPI1_SCK_CLK_ENABLE                                               \
    __HAL_RCC_GPIOB_CLK_ENABLE() // SPI1_SCK	引脚时钟

#define SPI1_MISO_PIN  GPIO_PIN_4 // SPI1_MISO 引脚
#define SPI1_MISO_PORT GPIOB      // SPI1_MISO 引脚端口
#define GPIO_SPI1_MISO_CLK_ENABLE                                              \
    __HAL_RCC_GPIOB_CLK_ENABLE() // SPI1_MISO 引脚时钟

#define SPI1_MOSI_PIN  GPIO_PIN_5 // SPI1_MOSI 引脚
#define SPI1_MOSI_PORT GPIOB      // SPI1_MOSI 引脚端口
#define GPIO_SPI1_MOSI_CLK_ENABLE                                              \
    __HAL_RCC_GPIOB_CLK_ENABLE() // SPI1_MOSI 引脚时钟

#define SPI1_CS_PIN             GPIO_PIN_3 // SPI1_CS 引脚
#define SPI1_CS_PORT            GPIOG      // SPI1_CS 引脚端口
#define GPIO_SPI1_CS_CLK_ENABLE __HAL_RCC_GPIOG_CLK_ENABLE() // SPI1_CS 引脚时钟

#define sFLASH_CS_LOW()                                                        \
    HAL_GPIO_WritePin(SPI1_CS_PORT, SPI1_CS_PIN, GPIO_PIN_RESET) // CS输出低电平
#define sFLASH_CS_HIGH()                                                       \
    HAL_GPIO_WritePin(SPI1_CS_PORT, SPI1_CS_PIN, GPIO_PIN_SET) // CS输出高电平

/*---------------------------------------------- 函数声明
 * ---------------------------------------*/

void     sFLASH_EraseSector(uint32_t SectorAddr);
void     sFLASH_EraseBulk(void);
void     sFLASH_WritePage(uint8_t *pBuffer, uint32_t WriteAddr,
                          uint16_t NumByteToWrite);
void     sFLASH_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr,
                            uint16_t NumByteToWrite);
void     sFLASH_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr,
                           uint16_t NumByteToRead);
uint32_t sFLASH_ReadID(void);
void     sFLASH_StartReadSequence(uint32_t ReadAddr);

uint8_t  sFLASH_ReadByte(void);
uint8_t  sFLASH_SendByte(uint8_t byte);
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
void     sFLASH_WriteEnable(void);
void     sFLASH_WaitForWriteEnd(void);

#endif // __FLASH_FLASH_H__