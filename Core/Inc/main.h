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
#define LCD_LAYER0_MEM_ADDR LCD_MEM_ADDR
#define LCD_LAYER0_PIXEL_BYTES 2
#define LCD_WIDTH 800
#define LCD_MEM_ADDR_END 0xD1000000
#define LCD_END_ADDR LCD_MEM_ADDR+LCD_WIDTH*LCD_HEIGHT*LCD_LAYER0_PIXEL_BYTES
#define LCD_HEIGHT 480
#define LCD_MEM_ADDR 0xD0000000
#define BTN3_Pin GPIO_PIN_2
#define BTN3_GPIO_Port GPIOE
#define BTN3_EXTI_IRQn EXTI2_IRQn
#define BTN4_Pin GPIO_PIN_3
#define BTN4_GPIO_Port GPIOE
#define BTN4_EXTI_IRQn EXTI3_IRQn
#define BTN5_Pin GPIO_PIN_4
#define BTN5_GPIO_Port GPIOE
#define BTN6_Pin GPIO_PIN_5
#define BTN6_GPIO_Port GPIOE
#define MDM_NET_Pin GPIO_PIN_6
#define MDM_NET_GPIO_Port GPIOE
#define TOUCH_IIC_SCL_Pin GPIO_PIN_13
#define TOUCH_IIC_SCL_GPIO_Port GPIOC
#define TOUCH_IIC_RST_Pin GPIO_PIN_11
#define TOUCH_IIC_RST_GPIO_Port GPIOI
#define BTN7_Pin GPIO_PIN_6
#define BTN7_GPIO_Port GPIOF
#define BTN7_EXTI_IRQn EXTI9_5_IRQn
#define BTN8_Pin GPIO_PIN_7
#define BTN8_GPIO_Port GPIOF
#define BTN8_EXTI_IRQn EXTI9_5_IRQn
#define RELAY3_Pin GPIO_PIN_8
#define RELAY3_GPIO_Port GPIOF
#define RELAY4_Pin GPIO_PIN_9
#define RELAY4_GPIO_Port GPIOF
#define MDM_PWK_Pin GPIO_PIN_10
#define MDM_PWK_GPIO_Port GPIOF
#define MDM_PEN_Pin GPIO_PIN_1
#define MDM_PEN_GPIO_Port GPIOC
#define RELAY5_Pin GPIO_PIN_2
#define RELAY5_GPIO_Port GPIOH
#define RELAY6_Pin GPIO_PIN_3
#define RELAY6_GPIO_Port GPIOH
#define WORKING_STATUS_LED_Pin GPIO_PIN_4
#define WORKING_STATUS_LED_GPIO_Port GPIOH
#define NET_STATUS_LED_Pin GPIO_PIN_5
#define NET_STATUS_LED_GPIO_Port GPIOH
#define FOAM_LED_Pin GPIO_PIN_4
#define FOAM_LED_GPIO_Port GPIOA
#define WATER_LED_Pin GPIO_PIN_5
#define WATER_LED_GPIO_Port GPIOA
#define BTN2_Pin GPIO_PIN_4
#define BTN2_GPIO_Port GPIOC
#define BTN2_EXTI_IRQn EXTI4_IRQn
#define BTN1_Pin GPIO_PIN_5
#define BTN1_GPIO_Port GPIOC
#define BTN1_EXTI_IRQn EXTI9_5_IRQn
#define TOUCH_IIC_SDA_Pin GPIO_PIN_2
#define TOUCH_IIC_SDA_GPIO_Port GPIOB
#define LCD_BACK_LIGHT_Pin GPIO_PIN_13
#define LCD_BACK_LIGHT_GPIO_Port GPIOD
#define RELAY7_Pin GPIO_PIN_2
#define RELAY7_GPIO_Port GPIOG
#define SPI_FLASH_CS_Pin GPIO_PIN_3
#define SPI_FLASH_CS_GPIO_Port GPIOG
#define RELAY8_Pin GPIO_PIN_6
#define RELAY8_GPIO_Port GPIOG
#define TOUCH_IIC_INT_Pin GPIO_PIN_15
#define TOUCH_IIC_INT_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
