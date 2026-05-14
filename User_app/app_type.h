/*
 * app_type.h
 *
 *  Created on: May 14, 2026
 *      Author: PCBOX
 */

#ifndef APP_TYPE_H_
#define APP_TYPE_H_

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
    CMD_READ_GAIN,

	CMD_SET_SENSOR_ID_BAUD,
	CMD_SET_SLAVE_ID_BAUD,
    CMD_SET_SLOPE,
    CMD_SET_CALIB_ZERO,
    CMD_SET_OFFSET,
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
	CONFIRM_SETUP,
} LCD_INTERFACE;

typedef enum {
	MODBUS_CUR,
	CALIB_CUR,
	OFFSET_CUR,
	WARNING_CUR,
	RANGE_CUR,
	INFO_CUR,
} SETTING_CUR;

#endif /* APP_TYPE_H_ */
