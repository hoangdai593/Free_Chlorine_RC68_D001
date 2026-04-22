/*=========================================================
  FILE: st7565r.h
=========================================================*/
#ifndef __ST7565R_H
#define __ST7565R_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>

/*=========================================================
  LCD SIZE
=========================================================*/
#define ST7565_LCD_WIDTH      128
#define ST7565_LCD_HEIGHT     64
#define ST7565_LCD_PAGES      8

/*=========================================================
  COMMANDS
=========================================================*/
#define ST7565_CMD_DISPLAY_OFF      0xAE
#define ST7565_CMD_DISPLAY_ON       0xAF

#define ST7565_CMD_START_LINE       0x40
#define ST7565_CMD_PAGE_ADDR        0xB0

#define ST7565_CMD_COL_UPPER        0x10
#define ST7565_CMD_COL_LOWER        0x00

#define ST7565_CMD_ADC_NORMAL       0xA0
#define ST7565_CMD_ADC_REVERSE      0xA1

#define ST7565_CMD_NORMAL_DISPLAY   0xA4
#define ST7565_CMD_ALL_PIXEL_ON     0xA5

#define ST7565_CMD_BIAS_9           0xA2
#define ST7565_CMD_BIAS_7           0xA3

#define ST7565_CMD_COM_NORMAL       0xC0
#define ST7565_CMD_COM_REVERSE      0xC8

#define ST7565_CMD_INTERNAL_RESET   0xE2

#define ST7565_CMD_POWER_CTRL       0x2F

#define ST7565_CMD_RESISTOR_RATIO   0x20

#define ST7565_CMD_CONTRAST_MODE    0x81

/*=========================================================
  API
=========================================================*/
void glcd_command(uint8_t c);
void glcd_data(uint8_t c);

void glcd_set_contrast(uint8_t val);

void glcd_set_y_address(uint8_t y);
void glcd_set_x_address(uint8_t x);

void glcd_set_column_upper(uint8_t addr);
void glcd_set_column_lower(uint8_t addr);

void glcd_set_start_line(uint8_t addr);

void glcd_all_on(void);
void glcd_normal(void);

void glcd_clear_now(void);
void glcd_pattern(void);
void glcd_write(void);

void glcd_power_down(void);
void glcd_power_up(void);

void glcd_ST7565R_init(void);

#ifdef __cplusplus
}
#endif

#endif
