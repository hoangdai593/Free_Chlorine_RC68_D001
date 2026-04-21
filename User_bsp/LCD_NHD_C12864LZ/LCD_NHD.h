/*
 * LCD_NHD.h
 *
 *  Created on: Apr 20, 2026
 *      Author: PCBOX
 */
#include "main.h"

#ifndef LCD_NHD_C12864LZ_LCD_NHD_H_
#define LCD_NHD_C12864LZ_LCD_NHD_H_

#define LCD_CS_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_6

#define LCD_RES_Port GPIOC
#define LCD_RES_Pin GPIO_PIN_7

#define LCD_A0_Port GPIOC
#define LCD_A0_Pin GPIO_PIN_8

#define LCD_WR_Port GPIOC
#define LCD_WR_Pin GPIO_PIN_9

#define LCD_RD_Port GPIOA
#define LCD_RD_Pin GPIO_PIN_8

#define LCD_PS_Port GPIOA
#define LCD_PS_Pin GPIO_PIN_10

extern SPI_HandleTypeDef hspi2;
extern unsigned char NHD_Logo[];



void comm_write(uint8_t cmd);
void data_write(uint8_t data);
void  LCD_Init(void);
void LCD_Clear(void);
void LCD_Display(uint8_t *buf);

void LCD_AllPixelOn(void);
void LCD_NormalDisplay(void);
void LCD_SetContrast(uint8_t value);
void LCD_FillBlack(void);
#endif /* LCD_NHD_C12864LZ_LCD_NHD_H_ */
