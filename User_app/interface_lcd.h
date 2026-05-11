/*
 * interface_lcd.h
 *
 *  Created on: May 4, 2026
 *      Author: PCBOX
 */

#ifndef INTERFACE_LCD_H_
#define INTERFACE_LCD_H_

#include "GPIO.h"
#include "event_driven.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "glcd.h"
#include "ST7565R.h"
#include "glcd_text.h"
#include "glcd_graphics.h"
#include "modbus_rtu.h"
#include "modbus_master.h"
#include "modbus_crc.h"
#include "clo_rc68.h"


typedef enum {
	MAIN,
	LOGIN,
	SETTING,
	MODBUS,
	CALIB,
	OFFSET,
	WARNING,
	RANGE,
	INFO,
} LCD_INTERFACE;

typedef enum {
	MODBUS_CUR,
	CALIB_CUR,
	OFFSET_CUR,
	WARNING_CUR,
	RANGE_CUR,
	INFO_CUR,
} SETTING_CUR;

// UI
extern LCD_INTERFACE interface;
extern SETTING_CUR   setting_cursor;
extern uint8_t  blink;
extern uint32_t blink_tick;
// PASSWORD
extern uint8_t password[4];
extern uint8_t password_true[4];
extern int8_t pass_cur;
// MODBUS SETTING
extern uint8_t  modbus_cursor;
extern uint8_t  modbus_edit;
extern uint16_t modbus_id;
#define BAUD_LIST_SIZE   9
extern const uint32_t baud_list[BAUD_LIST_SIZE];
extern uint8_t baud_index;
// CALIB
extern uint8_t calib_cursor;
extern uint8_t calib_edit;
extern uint8_t slope_digit[4];
extern int8_t slope_pos;
extern uint8_t calib_zero_confirm;
// OFFSET
extern uint8_t offset_cursor;
extern uint8_t offset_edit;
extern uint8_t offset_digit[4];
extern uint8_t offset_pos;
// WARNING
extern uint8_t warning_cursor;
extern uint8_t warning_edit;
extern uint8_t warning_mode;
extern uint8_t upper_digit[3];
extern uint8_t lower_digit[3];
extern uint8_t warning_mode_saved;
extern uint8_t upper_digit_saved[3];
extern uint8_t lower_digit_saved[3];

extern uint8_t warning_pos;
// RANGE
extern const uint16_t range_list[];
extern uint8_t range_index;
extern uint8_t range_edit;
extern uint8_t gain_current;

void cmd_wait_display(void);
void blink_display(void);
void main_display(void);
void login_display(void);
void setting_display(void);
void modbus_display(void);
void calib_display(void);
void offset_display(void);
void warning_display(void);
void range_display(void);
void info_display(void);


#endif /* INTERFACE_LCD_H_ */
