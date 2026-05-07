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
#include "Task.h"
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

/* LCD + UI */
extern LCD_INTERFACE interface;
extern SETTING_CUR setting_cursor;
extern uint8_t blink;
extern uint32_t blink_tick;

extern uint8_t bt_enter;

/* Password */
extern uint8_t password[4];
extern uint8_t password_true[4];
extern int8_t pass_cur;

/* Modbus */
extern MB_RTU_t mb1;
extern float clo_value;

/* ===== SENSOR DATA ===== */
extern uint16_t rc68_id;
extern uint16_t rc68_baud;
extern float rc68_temp;
extern float rc68_offset;
extern uint16_t rc68_gain;

/* ===== REQUEST FLAG ===== */
extern uint8_t req_read_id;
extern uint8_t req_read_baud;
extern uint8_t req_read_temp;
extern uint8_t req_read_offset;
extern uint8_t req_read_gain;

/* ===== CALIB STATE ===== */
extern uint8_t calib_cursor;
extern uint8_t calib_edit;

extern uint8_t slope_digit[4];
extern int8_t slope_pos;

/* MODBUS */
extern uint8_t modbus_cursor;
extern uint8_t modbus_edit;
extern uint16_t modbus_id;
extern uint8_t baud_index;
extern const uint32_t baud_list[9];

/* ===== OFFSET ===== */
extern uint8_t offset_cursor;
extern uint8_t offset_edit;

extern uint8_t offset_digit[2];
extern uint8_t offset_pos;

/* ===== WARNING ===== */
extern uint8_t warning_cursor;
extern uint8_t warning_edit;

extern uint8_t warning_mode;

extern uint8_t upper_digit[3];
extern uint8_t lower_digit[3];

extern uint8_t warning_pos;

/* ===== RANGE ===== */
extern const uint16_t range_list[];
extern uint8_t range_index;
extern uint8_t range_edit;
extern float mV_value;

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
