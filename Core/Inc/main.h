/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32l4xx_hal.h"

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
extern SPI_HandleTypeDef hspi2;

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SIM_TX_Pin GPIO_PIN_2
#define SIM_TX_GPIO_Port GPIOA
#define SIM_RX_Pin GPIO_PIN_3
#define SIM_RX_GPIO_Port GPIOA
#define I_O_2_Pin GPIO_PIN_4
#define I_O_2_GPIO_Port GPIOA
#define RX_DEBUG_Pin GPIO_PIN_10
#define RX_DEBUG_GPIO_Port GPIOB
#define TX_DEBUG_Pin GPIO_PIN_11
#define TX_DEBUG_GPIO_Port GPIOB
#define LCD_CLK_Pin GPIO_PIN_13
#define LCD_CLK_GPIO_Port GPIOB
#define LCD_MOSI_Pin GPIO_PIN_15
#define LCD_MOSI_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_6
#define LCD_CS_GPIO_Port GPIOC
#define LCD_RES_Pin GPIO_PIN_7
#define LCD_RES_GPIO_Port GPIOC
#define LCD_A0_Pin GPIO_PIN_8
#define LCD_A0_GPIO_Port GPIOC
#define LCD_WR_Pin GPIO_PIN_9
#define LCD_WR_GPIO_Port GPIOC
#define LCD_RD_E_Pin GPIO_PIN_8
#define LCD_RD_E_GPIO_Port GPIOA
#define LCD_C86_Pin GPIO_PIN_9
#define LCD_C86_GPIO_Port GPIOA
#define LCD_P_S_Pin GPIO_PIN_10
#define LCD_P_S_GPIO_Port GPIOA
#define MCU_RL1_Pin GPIO_PIN_11
#define MCU_RL1_GPIO_Port GPIOA
#define MCU_RL2_Pin GPIO_PIN_12
#define MCU_RL2_GPIO_Port GPIOA
#define TX_485_2_Pin GPIO_PIN_10
#define TX_485_2_GPIO_Port GPIOC
#define RX_485_2_Pin GPIO_PIN_11
#define RX_485_2_GPIO_Port GPIOC
#define control_485_2_Pin GPIO_PIN_12
#define control_485_2_GPIO_Port GPIOC
#define control_485_1_Pin GPIO_PIN_5
#define control_485_1_GPIO_Port GPIOB
#define TX_485_1_Pin GPIO_PIN_6
#define TX_485_1_GPIO_Port GPIOB
#define RX_485_1_Pin GPIO_PIN_7
#define RX_485_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
