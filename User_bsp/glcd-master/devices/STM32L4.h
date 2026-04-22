#ifndef __STM32L4_GLCD_H
#define __STM32L4_GLCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>

/*=========================================================
  LCD CONTROLLER
=========================================================*/
#define GLCD_CONTROLLER_ST7565R
#define GLCD_DEVICE_STM32L4XX
/*=========================================================
  PIN MAPPING (PCB của bạn)
=========================================================*/
#define GLCD_CS_PORT        GPIOC
#define GLCD_CS_PIN         GPIO_PIN_6

#define GLCD_RST_PORT       GPIOC
#define GLCD_RST_PIN        GPIO_PIN_7

#define GLCD_A0_PORT        GPIOC
#define GLCD_A0_PIN         GPIO_PIN_8

/*=========================================================
  CONTROL
=========================================================*/
#define GLCD_SELECT()       HAL_GPIO_WritePin(GLCD_CS_PORT,  GLCD_CS_PIN,  GPIO_PIN_RESET)
#define GLCD_DESELECT()     HAL_GPIO_WritePin(GLCD_CS_PORT,  GLCD_CS_PIN,  GPIO_PIN_SET)

#define GLCD_A0_LOW()       HAL_GPIO_WritePin(GLCD_A0_PORT,  GLCD_A0_PIN,  GPIO_PIN_RESET)
#define GLCD_A0_HIGH()      HAL_GPIO_WritePin(GLCD_A0_PORT,  GLCD_A0_PIN,  GPIO_PIN_SET)

#define GLCD_RESET_LOW()    HAL_GPIO_WritePin(GLCD_RST_PORT, GLCD_RST_PIN, GPIO_PIN_RESET)
#define GLCD_RESET_HIGH()   HAL_GPIO_WritePin(GLCD_RST_PORT, GLCD_RST_PIN, GPIO_PIN_SET)

/*=========================================================
  FUNCTION
=========================================================*/
void glcd_init(void);
void glcd_spi_write(uint8_t data);
void glcd_reset(void);

void glcd_enable_backlight(uint8_t state);
void glcd_change_backlight(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
