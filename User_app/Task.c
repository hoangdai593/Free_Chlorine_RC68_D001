/*
 * Task.c
 *
 *  Created on: Apr 7, 2026
 *      Author: PCBOX
 */
#include "GPIO.h"
#include "event_driven.h"
#include "Task.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "modbus_rtu.h"
#include "modbus_master.h"
#include "modbus_crc.h"
#include "clo_rc68.h"
#include "interface_lcd.h"
#include "button_hanlde.h"

sEvent_struct ALLTASK[] =
{
	{_EVENT_LCD_DISPLAY, 1, 	0,		200, _Cb_LCD_Display},   // chạy mỗi 1s
	{_EVENT_BUTTON, 	1, 		0, 		20, _Cb_Button},
	{_EVENT_SENSOR, 	1, 		0, 		1000, _Cb_Sensor},
};

CMD_STATE_t cmd_state = CMD_IDLE;
CMD_RESULT_t cmd_result = CMD_RES_NONE;

uint8_t cmd_retry = 0;
uint32_t cmd_tick = 0;
uint8_t cmd_busy = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        MB_RTU_RxByteHandler(&mb1);
    }
}

void handle_cmd_process(void)
{
    if(!cmd_busy)
        return;

    switch(cmd_state)
    {
        case CMD_SEND:
            cmd_result = CMD_RES_SENDING;

            switch(cmd_type)
            {
				case CMD_SET_ID_BAUD:
				{
					if(modbus_cursor == 0)
					{
						// chỉnh ID
						RC68_WriteID(&mb1, modbus_id);
					}
					else
					{
						// chỉnh BAUD
						RC68_WriteBaud(&mb1, baud_index+1);
					}
					break;
				}

                case CMD_SET_SLOPE:
                {
                    float slope = slope_digit[0]
                                + slope_digit[1]*0.1f
                                + slope_digit[2]*0.01f;

                    RC68_CalibHigh(&mb1, slope);
                    break;
                }

                case CMD_SET_CALIB_ZERO:
                    RC68_CalibZero(&mb1);
                    break;

                case CMD_SET_OFFSET:
                {
                    float offset = offset_digit[0]*10.0f
                                 + offset_digit[1];

                    RC68_WriteOffset(&mb1, offset);
                    break;
                }

                case CMD_SET_RANGE:
                    RC68_WriteGain(&mb1, range_index);
                    break;

                case CMD_SET_WARNING:
                    cmd_busy = 0;
                    cmd_state = CMD_IDLE;
                    cmd_result = CMD_RES_DONE;
                    return;

                default:
                    cmd_busy = 0;
                    cmd_state = CMD_IDLE;
                    return;
            }

            cmd_state = CMD_WAIT;
            cmd_tick = HAL_GetTick();
            break;

        case CMD_WAIT:
        {
            /* ===== NHẬN RESPONSE ===== */
            if(mb1.frame_ready)
            {
                mb1.frame_ready = 0;
                mb1.rx_index = 0;

                cmd_busy = 0;
                cmd_state = CMD_IDLE;
                cmd_result = CMD_RES_DONE;
                return;
            }

            /* ===== TIMEOUT ===== */
            if(HAL_GetTick() - cmd_tick > 2000)
            {
                cmd_busy = 0;
                cmd_state = CMD_IDLE;
                cmd_result = CMD_RES_FAIL;
                cmd_retry = 0;
            }
            break;
        }

        default:
            cmd_busy = 0;
            cmd_state = CMD_IDLE;
            break;
    }
}

uint8_t _Cb_Sensor(uint8_t x)
{
    static uint32_t slope_tick = 0;
    static uint8_t read_slope_once = 0;

    MB_RTU_Poll(&mb1);

    handle_cmd_process();

    if(mb1.frame_ready)
    {
        if(!cmd_busy)
        {
            if(mb1.rx_buf[1] == 0x03)
            {
                /* ===== response slope/intercept ===== */
                if(read_slope_once)
                {
                    slope_value     = RC68_GetFloat(&mb1, 0);
                    intercept_value = RC68_GetFloat(&mb1, 2);

                    read_slope_once = 0;
                }
                else
                {
                    /* ===== response normal ===== */
                    clo_value = RC68_GetFloat(&mb1, 0);
                    mV_value  = RC68_GetFloat(&mb1, 2);
                    rc68_temp = RC68_GetFloat(&mb1, 4);
                }
            }
        }

        mb1.frame_ready = 0;
        mb1.rx_index = 0;
    }

    if(!cmd_busy)
    {
        /* ===== mỗi 10s đọc slope/intercept 1 lần ===== */
        if(HAL_GetTick() - slope_tick >= 10000)
        {
            slope_tick = HAL_GetTick();

            RC68_ReadSlopeIntercept(&mb1);

            read_slope_once = 1;
        }
        else
        {
            /* ===== bình thường ===== */
            RC68_ReadAll(&mb1);
        }
    }

    return 0;
}

uint8_t _Cb_LCD_Display(uint8_t x)
{
    blink_display();

    static uint32_t display_tick = 0;

    /* ================= COMMAND UI ================= */
    if(cmd_busy || cmd_result != CMD_RES_NONE)
    {
        /* KHÔNG gọi main_display nữa */
        glcd_clear_buffer();

        if(display_tick == 0)
            display_tick = HAL_GetTick();

        if(cmd_busy)
        {
            if(cmd_result == CMD_RES_RETRY)
                draw_string_small(10,20,"RETRY...");
            else
                draw_string_small(10,20,"SENDING...");
        }
        else
        {
            switch(cmd_result)
            {
                case CMD_RES_DONE:
                    draw_string_small(10,20,"DONE");
                    break;

                case CMD_RES_FAIL:
                    draw_string_small(10,20,"FAIL");
                    break;

                default:
                    break;
            }
        }

        /* giữ 1s */
        if(!cmd_busy && (HAL_GetTick() - display_tick > 1000))
        {
            cmd_result = CMD_RES_NONE;
            cmd_retry  = 0;
            display_tick = 0;
            interface = SETTING;
        }

        return 0;
    }

    display_tick = 0;

    /* ================= NORMAL UI ================= */
    switch(interface)
    {
        case MAIN:    main_display(); break;
        case LOGIN:   login_display(); break;
        case SETTING: setting_display(); break;
        case MODBUS:  modbus_display(); break;
        case CALIB:   calib_display(); break;
        case OFFSET:  offset_display(); break;
        case WARNING: warning_display(); break;
        case RANGE:   range_display(); break;
        case INFO:    info_display(); break;
        default: break;
    }

    return 0;
}

uint8_t _Cb_Button(uint8_t x)
{
	if(HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0)
	{
		while(HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0){}
		enter_button_handle();
	}
	else if(HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0)
	{
		while(HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0){}
		down_button_handle();
	}
	else if(HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0)
	{
		while(HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0){}
		up_button_handle();
	}
	else if(HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0)
	{
		while(HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0){}
		exit_button_handle();
	}

	return 0;
}


uint8_t Comm_Task(void)
{
	uint8_t i = 0;

	for (i = 0; i < 3; i++)
	{
		if (ALLTASK[i].e_status == 1)
		{
			if ((ALLTASK[i].e_systick == 0) || ((HAL_GetTick() - ALLTASK[i].e_systick)  >=  ALLTASK[i].e_period))
			{
//				ALLTASK[i].e_status = 0;  //Disable event
				ALLTASK[i].e_systick = HAL_GetTick();
				ALLTASK[i].e_function_handler(i);
			}
		}
	}

	return 0;
}
