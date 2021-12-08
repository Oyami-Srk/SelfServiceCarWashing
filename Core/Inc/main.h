/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_MEM_ADDR 0xD0000000
#define LCD_LAYER0_MEM_ADDR LCD_MEM_ADDR
#define LCD_LAYER0_PIXEL_BYTES 2
#define LCD_LAYER1_MEM_ADDR LCD_MEM_ADDR+LCD_WIDTH*LCD_HEIGHT*LCD_LAYER0_PIXEL_BYTES
#define LCD_END_ADDR LCD_LAYER1_MEM_ADDR+LCD_WIDTH*LCD_HEIGHT*LCD_LAYER1_PIXEL_BYTES
#define LCD_LAYER1_PIXEL_BYTES 4
#define TOUCH_IIC_SCL_Pin GPIO_PIN_13
#define TOUCH_IIC_SCL_GPIO_Port GPIOC
#define TOUCH_IIC_RST_Pin GPIO_PIN_11
#define TOUCH_IIC_RST_GPIO_Port GPIOI
#define WORKING_STATUS_LED_Pin GPIO_PIN_4
#define WORKING_STATUS_LED_GPIO_Port GPIOH
#define NET_STATUS_LED_Pin GPIO_PIN_5
#define NET_STATUS_LED_GPIO_Port GPIOH
#define TOUCH_IIC_SDA_Pin GPIO_PIN_2
#define TOUCH_IIC_SDA_GPIO_Port GPIOB
#define LCD_BACK_LIGHT_Pin GPIO_PIN_13
#define LCD_BACK_LIGHT_GPIO_Port GPIOD
#define TOUCH_IIC_INT_Pin GPIO_PIN_15
#define TOUCH_IIC_INT_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
