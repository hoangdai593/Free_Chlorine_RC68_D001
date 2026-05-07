/*
 * Task.h
 *
 *  Created on: Apr 7, 2026
 *      Author: PCBOX
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

#include "modbus_rtu.h"
#include "modbus_master.h"
#include "clo_rc68.h"

typedef enum {
	_EVENT_LCD_DISPLAY,
	_EVENT_BUTTON,
	_EVENT_SENSOR,
} EVENT_List;

/* ===== CMD SYSTEM ===== */
typedef enum
{
    CMD_NONE = 0,
    CMD_SET_ID_BAUD,
    CMD_SET_SLOPE,
    CMD_SET_CALIB_ZERO,    // Thêm để calib Zero// Calib Slope
    CMD_SET_OFFSET,
    CMD_SET_WARNING,      // Chỉ lưu biến
    CMD_SET_RANGE,        // Chỉnh Gain
} CMD_TYPE_t;

typedef enum
{
    CMD_IDLE = 0,
    CMD_SEND,
    CMD_WAIT
} CMD_STATE_t;

typedef enum
{
    CMD_RES_NONE = 0,
    CMD_RES_SENDING,
    CMD_RES_RETRY,
    CMD_RES_DONE,
    CMD_RES_FAIL
} CMD_RESULT_t;

// CMD
extern CMD_STATE_t  cmd_state;
extern CMD_RESULT_t cmd_result;

extern CMD_TYPE_t cmd_type;

extern uint8_t  cmd_retry;
extern uint8_t  cmd_busy;
extern uint32_t cmd_tick;


// MODBUS
extern MB_RTU_t mb1;


// SENSOR
extern float clo_value;
extern float mV_value;
extern float rc68_temp;

extern float slope_value;
extern float intercept_value;


void handle_cmd_process(void);
uint8_t _Cb_LCD_Display(uint8_t x);
uint8_t _Cb_Button(uint8_t x);
uint8_t _Cb_Sensor(uint8_t x);
uint8_t _Cb_Debug(uint8_t x);
uint8_t Comm_Task(void);


#endif /* INC_TASK_H_ */
