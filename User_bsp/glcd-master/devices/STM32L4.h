/**
 * @file    STM32L4.h
 * @brief   Functions specific to STM32L4xx devices for Andy Gock GLCD library
 *
 * For STM32CubeIDE + HAL
 */

#ifndef STM32L4_H_
#define STM32L4_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>


#define GLCD_CONTROLLER_ST7565R
/* =========================================================
   SPI HANDLE
   Chỉnh đúng SPI đang dùng trong CubeMX
   ========================================================= */
#define GLCD_SPI_HANDLE                 hspi2

/* =========================================================
   GPIO LCD CONTROL
   Chỉnh đúng chân bạn đang nối thật
   ========================================================= */

/* CS */
#define CONTROLLER_SPI_SS_PORT         GPIOC
#define CONTROLLER_SPI_SS_PIN          GPIO_PIN_6

/* A0 / D-C */
#define CONTROLLER_SPI_DC_PORT         GPIOC
#define CONTROLLER_SPI_DC_PIN          GPIO_PIN_8

/* RESET */
#define CONTROLLER_SPI_RST_PORT        GPIOC
#define CONTROLLER_SPI_RST_PIN         GPIO_PIN_7

/* =========================================================
   CONTROL MACROS
   ========================================================= */

#define GLCD_SELECT() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_SS_PORT, CONTROLLER_SPI_SS_PIN, GPIO_PIN_RESET)

#define GLCD_DESELECT() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_SS_PORT, CONTROLLER_SPI_SS_PIN, GPIO_PIN_SET)

#define GLCD_A0_LOW() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_DC_PORT, CONTROLLER_SPI_DC_PIN, GPIO_PIN_RESET)

#define GLCD_A0_HIGH() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_DC_PORT, CONTROLLER_SPI_DC_PIN, GPIO_PIN_SET)

#define GLCD_RESET_LOW() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_RST_PORT, CONTROLLER_SPI_RST_PIN, GPIO_PIN_RESET)

#define GLCD_RESET_HIGH() \
    HAL_GPIO_WritePin(CONTROLLER_SPI_RST_PORT, CONTROLLER_SPI_RST_PIN, GPIO_PIN_SET)

/* =========================================================
   PUBLIC FUNCTIONS
   ========================================================= */

void glcd_init(void);
void glcd_spi_write(uint8_t c);
void glcd_reset(void);
//void delay_ms(uint32_t ms);

/* Optional */
void glcd_enable_backlight(uint8_t state);
void glcd_change_backlight(uint8_t value);

#ifdef __cplusplus
}
#endif

#else
#error "GLCD_DEVICE_STM32L4XX not defined"
#endif

