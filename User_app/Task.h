/*
 * Task.h
 *
 *  Created on: Apr 7, 2026
 *      Author: PCBOX
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

typedef enum {
	_EVENT_LCD_DISPLAY,
	_EVENT_BUTTON
} EVENT_List;

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

#define BT_ENTER_PORT   GPIOC
#define BT_ENTER_PIN    GPIO_PIN_5

#define BT_EXIT_PORT    GPIOB
#define BT_EXIT_PIN     GPIO_PIN_0

#define BT_DOWN_PORT    GPIOB
#define BT_DOWN_PIN     GPIO_PIN_1

#define BT_UP_PORT      GPIOB
#define BT_UP_PIN     GPIO_PIN_2

uint8_t _Cb_LCD_Display(uint8_t x);
uint8_t _Cb_Button(uint8_t x);
uint8_t Comm_Task(void);


#endif /* INC_TASK_H_ */
