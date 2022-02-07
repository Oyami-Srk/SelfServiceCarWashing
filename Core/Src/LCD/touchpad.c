/*
 * Copyright (c) 2021-2022 Shiroko
 * Project: SelfServiceCarWashing
 * File: touchpad.c
 * Author: Shiroko <hhx.xxm@gmail.com>
 * ================================
 * This Project may contain code from ST under Ultimate Liberty license.
 * For that parts of codes, here is the copy of license: www.st.com/SLA0044
 *
 */

#include "Common/utils.h"
#include "main.h"

#define TP_DEF_GPIO(name)             name##_GPIO_Port, name##_Pin
#define SET_GPIO(status, name) HAL_GPIO_WritePin(TP_DEF_GPIO(name), GPIO_PIN_##status)
#define TOUCH_SCL(status)      SET_GPIO(status, TOUCH_IIC_SCL)
#define TOUCH_SDA(status)      SET_GPIO(status, TOUCH_IIC_SDA)
#define IIC_DELAY              20

#define IIC_ACK_OK  0x00
#define IIC_ACK_ERR 0x88

// Hardware IIC functionalities
static inline void TOUCH_IIC_DELAY(uint32_t time) {
    while (time--)
        for (int i = 0; i < 10; i++)
            ;
}

static void TOUCH_IIC_SET_IIC_OUTPUT() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode             = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull             = GPIO_PULLUP;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin              = TOUCH_IIC_INT_Pin;

    HAL_GPIO_Init(TOUCH_IIC_INT_GPIO_Port, &GPIO_InitStruct);
}

static void TOUCH_IIC_SET_IIC_INPUT() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode             = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin              = TOUCH_IIC_INT_Pin;

    HAL_GPIO_Init(TOUCH_IIC_INT_GPIO_Port, &GPIO_InitStruct);
}

static void TOUCH_IIC_START(void) {
    TOUCH_SDA(SET);
    TOUCH_SCL(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);

    TOUCH_SDA(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SCL(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
}

static void TOUCH_IIC_STOP(void) {
    TOUCH_SCL(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SDA(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);

    TOUCH_SCL(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SDA(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);
}

static void TOUCH_IIC_ACK(void) {
    TOUCH_SCL(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SDA(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SCL(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);

    TOUCH_SCL(RESET);
    TOUCH_SDA(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);
}

static void TOUCH_IIC_NOACK(void) {
    TOUCH_SCL(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SDA(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SCL(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);

    TOUCH_SCL(RESET);
    TOUCH_IIC_DELAY(IIC_DELAY);
}

static uint8_t TOUCH_IIC_WAIT_ACK(void) {
    TOUCH_SDA(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);
    TOUCH_SCL(SET);
    TOUCH_IIC_DELAY(IIC_DELAY);

    if (HAL_GPIO_ReadPin(TP_DEF_GPIO(TOUCH_IIC_SDA)) != 0) {
        TOUCH_SCL(RESET);
        TOUCH_IIC_DELAY(IIC_DELAY);
        return IIC_ACK_ERR;
    } else {
        TOUCH_SCL(RESET);
        TOUCH_IIC_DELAY(IIC_DELAY);
        return IIC_ACK_OK;
    }
}

static uint8_t TOUCH_IIC_WRITE_BYTE(uint8_t data) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            TOUCH_SDA(SET);
        else
            TOUCH_SDA(RESET);

        TOUCH_IIC_DELAY(IIC_DELAY);
        TOUCH_SCL(SET);
        TOUCH_IIC_DELAY(IIC_DELAY);
        TOUCH_SCL(RESET);
        if (i == 7) {
            TOUCH_SDA(SET);
        }
        data <<= 1;
    }
    return TOUCH_IIC_WAIT_ACK();
}

static uint8_t TOUCH_IIC_READ_BYTE(uint8_t ACK_Mode) {
    uint8_t data = 0;
    uint8_t i    = 0;

    for (i = 0; i < 8; i++) {
        data <<= 1;
        TOUCH_SCL(SET);
        TOUCH_IIC_DELAY(IIC_DELAY);
        data |= (HAL_GPIO_ReadPin(TP_DEF_GPIO(TOUCH_IIC_SDA)) & 0x01);
        TOUCH_SCL(RESET);
        TOUCH_IIC_DELAY(IIC_DELAY);
    }

    if (ACK_Mode == 1)
        TOUCH_IIC_ACK();
    else
        TOUCH_IIC_NOACK();

    return data;
}

// Touch chip functionalities

#define TOUCH_POINT_MAX 5
#define GT9XX_IIC_USE_0x28
#ifdef GT9XX_IIC_USE_0x28
#define GT9XX_IIC_RADDR 0x29
#define GT9XX_IIC_WADDR 0x28
#else
#define GT9XX_IIC_RADDR 0xBB
#define GT9XX_IIC_WADDR 0xBA
#endif
#define GT9XX_READ_ADDR   0x814E
#define GT9XX_ID_ADDR     0x8140
#define GT9XX_CFG_ADDR    0x8047 // 固件配置信息寄存器和配置起始地址
#define GT9XX_Chksum_ADDR 0X80FF // GT911 校验和寄存器

typedef struct {
    uint8_t  flag; // 1 means on touch
    uint8_t  points;
    uint16_t x[TOUCH_POINT_MAX];
    uint16_t y[TOUCH_POINT_MAX];
} TouchInfo;

static TouchInfo    touchInfo;
__IO static uint8_t Modify_Flag = 0;

/*****************************************************************************************
 *	函 数 名:	GT9XX_Reset
 *	入口参数:	无
 *	返 回 值:	无
 *	函数功能:	复位GT911
 *	说    明:	复位GT911，并将芯片的IIC地址配置为0xBA/0xBB
 ******************************************************************************************/

#define Touch_IIC_SCL_CLK_ENABLE __HAL_RCC_GPIOC_CLK_ENABLE() // SCL 引脚时钟
#define Touch_IIC_SCL_PORT       GPIOC       // SCL 引脚端口
#define Touch_IIC_SCL_PIN        GPIO_PIN_13 // SCL 引脚

#define Touch_IIC_SDA_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE() // SDA 引脚时钟
#define Touch_IIC_SDA_PORT       GPIOB      // SDA 引脚端口
#define Touch_IIC_SDA_PIN        GPIO_PIN_2 // SDA 引脚

#define Touch_INT_CLK_ENABLE __HAL_RCC_GPIOA_CLK_ENABLE() // INT 引脚时钟
#define Touch_INT_PORT       GPIOA                        // INT 引脚端口
#define Touch_INT_PIN        GPIO_PIN_15                  // INT 引脚

#define Touch_RST_CLK_ENABLE __HAL_RCC_GPIOI_CLK_ENABLE() // RST 引脚时钟
#define Touch_RST_PORT       GPIOI                        // RST 引脚端口
#define Touch_RST_PIN        GPIO_PIN_11                  // RST 引脚
void Touch_IIC_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    Touch_IIC_SCL_CLK_ENABLE; //初始化IO口时钟
    Touch_IIC_SDA_CLK_ENABLE;
    Touch_INT_CLK_ENABLE;
    Touch_RST_CLK_ENABLE;

    GPIO_InitStruct.Pin   = Touch_IIC_SCL_PIN;   // SCL引脚
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD; // 开漏输出
    GPIO_InitStruct.Pull  = GPIO_NOPULL;         // 不带上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 速度等级
    HAL_GPIO_Init(Touch_IIC_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Touch_IIC_SDA_PIN; // SDA引脚
    HAL_GPIO_Init(Touch_IIC_SDA_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;         // 上拉

    GPIO_InitStruct.Pin = Touch_INT_PIN; //	INT
    HAL_GPIO_Init(Touch_INT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Touch_RST_PIN; //	RST
    HAL_GPIO_Init(Touch_RST_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(Touch_IIC_SCL_PORT, Touch_IIC_SCL_PIN,
                      GPIO_PIN_SET); // SCL输出高电平
    HAL_GPIO_WritePin(Touch_IIC_SDA_PORT, Touch_IIC_SDA_PIN,
                      GPIO_PIN_SET); // SDA输出高电平
    HAL_GPIO_WritePin(Touch_INT_PORT, Touch_INT_PIN,
                      GPIO_PIN_RESET); // INT输出低电平
    HAL_GPIO_WritePin(Touch_RST_PORT, Touch_RST_PIN,
                      GPIO_PIN_SET); // RST输出高	电平
}

void GT9XX_Reset(void) {
    Touch_IIC_GPIO_Config();
    SET_GPIO(SET, TOUCH_IIC_SCL);
    SET_GPIO(SET, TOUCH_IIC_SDA);
    SET_GPIO(RESET, TOUCH_IIC_INT);
    SET_GPIO(SET, TOUCH_IIC_RST);

    // 初始化引脚状态
    SET_GPIO(RESET, TOUCH_IIC_INT);
    SET_GPIO(SET, TOUCH_IIC_RST);
    TOUCH_IIC_DELAY(10000);

    // 开始执行复位
#ifdef GT9XX_IIC_USE_0x28
    SET_GPIO(RESET, TOUCH_IIC_RST);
    TOUCH_IIC_DELAY(250000);      // 延时
    SET_GPIO(SET, TOUCH_IIC_INT); // 拉高INT引脚
    TOUCH_IIC_DELAY(250000);      // 延时
    SET_GPIO(SET, TOUCH_IIC_RST); // 拉高复位引脚，复位结束
    TOUCH_IIC_DELAY(450000);      // 延时
    TOUCH_IIC_SET_IIC_INPUT();    // INT引脚转为浮空输入
#else
    //	INT引脚保持低电平不变，将器件地址设置为0XBA/0XBB
    SET_GPIO(RESET, TOUCH_IIC_RST); // 拉低复位引脚，此时芯片执行复位
    TOUCH_IIC_DELAY(250000);        // 延时
    SET_GPIO(SET, TOUCH_IIC_RST); // 拉高复位引脚，复位结束
    TOUCH_IIC_DELAY(450000);      // 延时
    TOUCH_IIC_SET_IIC_INPUT();    // INT引脚转为浮空输入
    TOUCH_IIC_DELAY(350000);      // 延时
#endif
}

/*****************************************************************************************
 *	函 数 名:	GT9XX_WriteHandle
 *	入口参数:	addr - 要操作的寄存器
 *	返 回 值:	SUCCESS - 操作成功
 *					ERROR	  - 操作失败
 *	函数功能:	GT9XX 写操作
 *	说    明:	对指定的寄存器执行写操作
 ******************************************************************************************/
uint8_t GT9XX_WriteHandle(uint16_t addr) {
    uint8_t status; // 状态标志位

    TOUCH_IIC_START();                                       // 启动IIC通信
    if (TOUCH_IIC_WRITE_BYTE(GT9XX_IIC_WADDR) == IIC_ACK_OK) //写数据指令
    {
        if (TOUCH_IIC_WRITE_BYTE((uint8_t)(addr >> 8)) ==
            IIC_ACK_OK) //写入16位地址
        {
            if (TOUCH_IIC_WRITE_BYTE((uint8_t)(addr)) != IIC_ACK_OK) {
                status = ERROR; // 操作失败
            }
        }
    }
    status = SUCCESS; // 操作成功
    return status;
}

/*****************************************************************************************
 *	函 数 名:	GT9XX_WriteData
 *	入口参数:	addr - 要写入的寄存器
 *					value - 要写入的数据
 *	返 回 值:	SUCCESS - 操作成功
 *					ERROR	  - 操作失败
 *	函数功能:	GT9XX 写一字节数据
 *	说    明:	对指定的寄存器写入一字节数据
 ******************************************************************************************/
uint8_t GT9XX_WriteData(uint16_t addr, uint8_t value) {
    uint8_t status;

    TOUCH_IIC_START(); //启动IIC通讯

    if (GT9XX_WriteHandle(addr) == SUCCESS) //写入要操作的寄存器
    {
        if (TOUCH_IIC_WRITE_BYTE(value) != IIC_ACK_OK) //写数据
        {
            status = ERROR;
        }
    }
    TOUCH_IIC_STOP(); // 停止通讯

    status = SUCCESS; // 写入成功
    return status;
}

/*****************************************************************************************
 *	函 数 名:	GT9XX_WriteReg
 *	入口参数:	addr - 要写入的寄存器区域首地址
 *					cnt  - 数据长度
 *					value - 要写入的数据区
 *	返 回 值:	SUCCESS - 操作成功
 *					ERROR	  - 操作失败
 *	函数功能:	GT9XX 写寄存器
 *	说    明:	往芯片的寄存器区写入指定长度的数据
 ******************************************************************************************/
uint8_t GT9XX_WriteReg(uint16_t addr, uint8_t cnt, uint8_t *value) {
    uint8_t status;
    uint8_t i;

    TOUCH_IIC_START();

    if (GT9XX_WriteHandle(addr) == SUCCESS) // 写入要操作的寄存器
    {
        for (i = 0; i < cnt; i++) // 计数
        {
            TOUCH_IIC_WRITE_BYTE(value[i]); // 写入数据
        }
        TOUCH_IIC_STOP(); // 停止IIC通信
        status = SUCCESS; // 写入成功
    } else {
        TOUCH_IIC_STOP(); // 停止IIC通信
        status = ERROR;   // 写入失败
    }
    return status;
}

/*****************************************************************************************
 *	函 数 名:	GT9XX_ReadReg
 *	入口参数:	addr - 要读取的寄存器区域首地址
 *					cnt  - 数据长度
 *					value - 要读取的数据区
 *	返 回 值:	SUCCESS - 操作成功
 *					ERROR	  - 操作失败
 *	函数功能:	GT9XX 读寄存器
 *	说    明:	从芯片的寄存器区读取指定长度的数据
 ******************************************************************************************/
uint8_t GT9XX_ReadReg(uint16_t addr, uint8_t cnt, uint8_t *value) {
    uint8_t status;
    uint8_t i;

    status = ERROR;
    TOUCH_IIC_START(); // 启动IIC通信

    if (GT9XX_WriteHandle(addr) == SUCCESS) //写入要操作的寄存器
    {
        TOUCH_IIC_START(); //重新启动IIC通讯

        if (TOUCH_IIC_WRITE_BYTE(GT9XX_IIC_RADDR) == IIC_ACK_OK) // 发送读命令
        {
            for (i = 0; i < cnt; i++) // 计数
            {
                if (i == (cnt - 1)) {
                    value[i] = TOUCH_IIC_READ_BYTE(
                        0); // 读到最后一个数据时发送 非应答信号
                } else {
                    value[i] = TOUCH_IIC_READ_BYTE(1); // 发送应答信号
                }
            }
            TOUCH_IIC_STOP(); // 停止IIC通信
            status = SUCCESS;
        }
    }
    TOUCH_IIC_STOP(); // 停止IIC通信
    return (status);
}

/*****************************************************************************************
 *	函 数 名: Touch_Init
 *	入口参数: 无
 *	返 回 值: SUCCESS  - 初始化成功
 *            ERROR 	 - 错误，未检测到触摸屏
 *	函数功能: 触摸IC初始化，并读取相应信息发送到串口
 *	说    明: 初始化触摸面板
 ******************************************************************************************/
uint8_t Touch_Init(void) {
    uint8_t GT9XX_Info[11]; // 触摸屏IC信息
    uint8_t cfgVersion = 0; // 触摸配置版本

    GT9XX_Reset(); // 复位IC

    GT9XX_ReadReg(GT9XX_ID_ADDR, 11, GT9XX_Info);  // 读触摸屏IC信息
    GT9XX_ReadReg(GT9XX_CFG_ADDR, 1, &cfgVersion); // 读触摸配置版本

    if (GT9XX_Info[0] == '9') //	判断第一个字符是否为 ‘9’
    {
        PRINTF("[SYSTEM] Touchpad ID: GT%.4s \r\n",
               GT9XX_Info); // 打印触摸芯片的ID
        PRINTF("[SYSTEM] Touchpad Firmware Version: 0x%.4x\r\n",
               (GT9XX_Info[5] << 8) + GT9XX_Info[4]); // 芯片固件版本
        PRINTF("[SYSTEM] Touchpad Resolution: %d * %d\r\n",
               (GT9XX_Info[7] << 8) + GT9XX_Info[6],
               (GT9XX_Info[9] << 8) + GT9XX_Info[8]); // 当前触摸分辨率
        PRINTF("[SYSTEM] Touchpad Conf Version: 0x%.2x \r\n",
               cfgVersion); // 触摸配置版本

        return SUCCESS;
    } else {
        PRINTF("[SYSTEM] Touchpad undetected\r\n"); //错误，未检测到触摸屏
        return ERROR;
    }
}

/*****************************************************************************************
 *	函 数 名: Touch_Scan
 *	入口参数: 无
 *	返 回 值: 无
 *	函数功能: 触摸扫描
 *	说    明: 在程序里周期性的调用该函数，用以检测触摸操作，触摸信息存储在
 *touchInfo 结构体
 ******************************************************************************************/

void Touch_Scan(void) {
    uint8_t touchData[2 + 8 * TOUCH_POINT_MAX]; // 用于存储触摸数据
    uint8_t i = 0;

    GT9XX_ReadReg(GT9XX_READ_ADDR, 2 + 8 * TOUCH_POINT_MAX,
                  touchData);            // 读数据
    GT9XX_WriteData(GT9XX_READ_ADDR, 0); //	清除触摸芯片的寄存器标志位
    touchInfo.points = touchData[0] & 0x0f; // 取当前的触摸点数

    if ((touchInfo.points >= 1) &&
        (touchInfo.points <= 5)) // 当触摸数在 1-5 之间时
    {
        for (i = 0; i < touchInfo.points; i++) // 取相应的触摸坐标
        {
            touchInfo.y[i] =
                (touchData[5 + 8 * i] << 8) | touchData[4 + 8 * i]; // 获取Y坐标
            touchInfo.x[i] = (touchData[3 + 8 * i] << 8) |
                             touchData[2 + 8 * i]; //	获取X坐标
        }
        touchInfo.flag = 1; // 触摸标志位置1，代表有触摸动作发生
    } else {
        touchInfo.flag = 0; // 触摸标志位置0，无触摸动作
    }
}

uint8_t touchpad_get_xy(uint16_t *x, uint16_t *y) {
    Touch_Scan();
    if (touchInfo.flag == 0)
        return 0x00;
    if (touchInfo.x[0] > LCD_WIDTH || touchInfo.y[0] > LCD_HEIGHT) {
        *x = 0;
        *y = 0;
        return 0x00;
    }
    // Only get first touch
    *x = touchInfo.x[0];
    *y = touchInfo.y[0];

    return 0xFF;
}
