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
#include "modbus_slave.h"
#include "clo_rc68.h"

/* EVENT */
typedef enum
{
    _EVENT_LCD_DISPLAY,
    _EVENT_BUTTON,
    _EVENT_SENSOR,
} EVENT_List;

/* CMD */
typedef enum
{
    CMD_NONE = 0,

    CMD_READ_ALL,
    CMD_READ_SLOPE_INTERCEPT,

    CMD_SET_ID_BAUD,
    CMD_SET_SLOPE,
    CMD_SET_CALIB_ZERO,
    CMD_SET_OFFSET,
    CMD_SET_WARNING,
    CMD_SET_RANGE,

} CMD_TYPE_t;

/* CMD RESULT */
typedef enum
{
    CMD_RES_NONE = 0,
    CMD_RES_SENDING,
    CMD_RES_DONE,
    CMD_RES_FAIL
} CMD_RESULT_t;

/* CMD ITEM */
typedef struct
{
    CMD_TYPE_t type;
} CMD_QUEUE_t;

/* QUEUE */
#define CMD_QUEUE_SIZE     20
#define CMD_TIMEOUT_MS     500

/* CMD */
extern CMD_RESULT_t cmd_result;

extern uint8_t  cmd_running;
extern uint32_t cmd_tick;
extern uint32_t cmd_ui_tick;

extern CMD_QUEUE_t cmd_queue[CMD_QUEUE_SIZE];

extern uint8_t cmd_head;
extern uint8_t cmd_tail;
extern uint8_t cmd_count;

extern CMD_TYPE_t current_cmd;

/* MODBUS */
extern MB_RTU_t mb1;
extern MB_SLAVE_t mb_slave;
/* SENSOR */
extern float clo_value;
extern float mV_value;
extern float rc68_temp;

extern float slope_value;
extern float intercept_value;

/* FUNC */
void CMD_Enqueue(CMD_TYPE_t type);
void process_cmd_queue(void);

uint8_t _Cb_LCD_Display(uint8_t x);
uint8_t _Cb_Button(uint8_t x);
uint8_t _Cb_Sensor(uint8_t x);
uint8_t Comm_Task(void);

#endif
