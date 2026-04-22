/**
 * @file    glcd.h
 * @brief   Graphic LCD Library main header for STM32CubeIDE (STM32L4xx)
 * @author  Andy Gock (original)
 * @author  Reworked for STM32 HAL / CubeIDE
 */

#ifndef __GLCD_H
#define __GLCD_H

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================
   INCLUDE
   ========================================================= */
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <string.h>

/* =========================================================
   DEVICE / CONTROLLER SELECT
   ========================================================= */
#define GLCD_DEVICE_STM32L4XX
#define GLCD_CONTROLLER_ST7565R

/* =========================================================
   CONTROLLER HEADER
   ========================================================= */
#include "st7565r.h"

/* =========================================================
   BASIC MACRO
   ========================================================= */
#define swap(a,b)      \
do{                    \
    uint8_t t = a;     \
    a = b;             \
    b = t;             \
}while(0)

/* =========================================================
   COLOR
   ========================================================= */
#define BLACK   1
#define WHITE   0

/* =========================================================
   LCD SIZE
   ========================================================= */
#ifndef GLCD_LCD_WIDTH
#define GLCD_LCD_WIDTH      128
#endif

#ifndef GLCD_LCD_HEIGHT
#define GLCD_LCD_HEIGHT     64
#endif

/* =========================================================
   IMPORTANT FIX:
   Number of pages = HEIGHT / 8
   ========================================================= */
#define GLCD_NUMBER_OF_BANKS    (GLCD_LCD_HEIGHT / 8)
#define GLCD_NUMBER_OF_COLS     (GLCD_LCD_WIDTH)

#define GLCD_BUFFER_SIZE        (GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8)

/* =========================================================
   RESET TIME
   ========================================================= */
#ifndef GLCD_RESET_TIME
#define GLCD_RESET_TIME     2
#endif

/* =========================================================
   FONT TYPE
   ========================================================= */
typedef enum
{
    STANG = 0,
    MIKRO,
    GLCD_UTILS

} font_table_type_t;

/* =========================================================
   BOUNDING BOX
   ========================================================= */
typedef struct
{
    uint8_t x_min;
    uint8_t y_min;
    uint8_t x_max;
    uint8_t y_max;

} glcd_BoundingBox_t;

/* =========================================================
   FONT CONFIG
   ========================================================= */
typedef struct
{
    const char *font_table;
    uint8_t width;
    uint8_t height;
    char start_char;
    char end_char;
    font_table_type_t table_type;

} glcd_FontConfig_t;

/* =========================================================
   GLOBAL VARIABLES
   ========================================================= */
extern uint8_t glcd_buffer[GLCD_BUFFER_SIZE];
extern glcd_BoundingBox_t glcd_bbox;

extern uint8_t *glcd_buffer_selected;
extern glcd_BoundingBox_t *glcd_bbox_selected;

extern glcd_FontConfig_t font_current;
extern uint8_t glcd_buffer[GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8];
extern glcd_BoundingBox_t glcd_bbox;
/* =========================================================
   LOW LEVEL FUNCTION
   (viết trong bsp hoặc device file)
   ========================================================= */
void glcd_spi_write(uint8_t data);
void delay_ms(uint32_t ms);

/* =========================================================
   BASE FUNCTIONS
   ========================================================= */
void glcd_update_bbox(uint8_t xmin,
                      uint8_t ymin,
                      uint8_t xmax,
                      uint8_t ymax);

void glcd_reset_bbox(void);
void glcd_bbox_reset(void);
void glcd_bbox_refresh(void);

void glcd_clear(void);
void glcd_clear_buffer(void);

void glcd_select_screen(uint8_t *buffer,
                        glcd_BoundingBox_t *bbox);

void glcd_scroll(int8_t x, int8_t y);
void glcd_scroll_line(void);

/* =========================================================
   CONTROLLER FUNCTIONS (ST7565R.c)
   ========================================================= */
void glcd_command(uint8_t c);
void glcd_data(uint8_t c);

void glcd_set_contrast(uint8_t val);

void glcd_power_down(void);
void glcd_power_up(void);

void glcd_set_y_address(uint8_t y);
void glcd_set_x_address(uint8_t x);

void glcd_all_on(void);
void glcd_normal(void);

void glcd_set_column_upper(uint8_t addr);
void glcd_set_column_lower(uint8_t addr);

void glcd_set_start_line(uint8_t addr);

void glcd_clear_now(void);
void glcd_pattern(void);

void glcd_write(void);

void glcd_ST7565R_init(void);

/* =========================================================
   GPIO CONTROL MACRO
   ========================================================= */
#define GLCD_CS_GPIO_Port     GPIOC
#define GLCD_CS_Pin           GPIO_PIN_6

#define GLCD_RST_GPIO_Port    GPIOC
#define GLCD_RST_Pin          GPIO_PIN_7

#define GLCD_A0_GPIO_Port     GPIOC
#define GLCD_A0_Pin           GPIO_PIN_8

#define GLCD_CS_LOW() \
HAL_GPIO_WritePin(GLCD_CS_GPIO_Port, GLCD_CS_Pin, GPIO_PIN_RESET)

#define GLCD_CS_HIGH() \
HAL_GPIO_WritePin(GLCD_CS_GPIO_Port, GLCD_CS_Pin, GPIO_PIN_SET)

#define GLCD_A0_LOW() \
HAL_GPIO_WritePin(GLCD_A0_GPIO_Port, GLCD_A0_Pin, GPIO_PIN_RESET)

#define GLCD_A0_HIGH() \
HAL_GPIO_WritePin(GLCD_A0_GPIO_Port, GLCD_A0_Pin, GPIO_PIN_SET)

#define GLCD_RST_LOW() \
HAL_GPIO_WritePin(GLCD_RST_GPIO_Port, GLCD_RST_Pin, GPIO_PIN_RESET)

#define GLCD_RST_HIGH() \
HAL_GPIO_WritePin(GLCD_RST_GPIO_Port, GLCD_RST_Pin, GPIO_PIN_SET)

#ifdef __cplusplus
}
#endif

#endif /* __GLCD_H */
