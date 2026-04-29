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
#include "glcd.h"
#include "ST7565R.h"
#include "glcd_text.h"
#include "glcd_graphics.h"


sEvent_struct ALLTASK[] =
{
	{_EVENT_LCD_DISPLAY, 1, 	0,		 300, _Cb_LCD_Display},   // chạy mỗi 1s
	{_EVENT_BUTTON, 	1, 		0, 			20, _Cb_Button},
};

uint8_t blink = 0;
uint32_t blink_tick = 0;

LCD_INTERFACE interface = MAIN;
SETTING_CUR setting_cursor = MODBUS_CUR;
uint8_t bt_enter=0;

uint8_t password[4] = {0, 0, 0, 0};
uint8_t password_true[4] = {1, 1, 1, 1};
int8_t pass_cur = 0;
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART2)
//    {
//        if (rx_index < RX_BUFFER_SIZE - 1)
//        {
//            // Lưu byte vào buffer
//            rx_buffer[rx_index++] = rx_byte;
//            rx_buffer[rx_index] = '\0';  // luôn giữ string hợp lệ
//
//            //Khi có ít nhất 1 byte, trigger event
//            if (rx_byte=='\n')
//            {
//                fevent_active(ALLTASK, _EVENT_UART_RECEIVE);
//
//            }
//        }
//        else
//        {
//            // Buffer full → reset để tránh tràn
//            rx_index = 0;
//        }
//
//        // Nhận tiếp byte
//        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
//    }
//}

void blink_display(void)
{
	if(HAL_GetTick() - blink_tick >= 300)
	{
	    blink_tick = HAL_GetTick();
	    blink = !blink;
	}
}

void main_display(void)
{
	glcd_clear_buffer();
	glcd_draw_line(0, 10, 128, 10, BLACK);
	draw_string_small(5, 1, "Clo du");
	draw_string_small(85, 35, "mg/L");
	draw_string_big(30, 22, "0.36");
	draw_string_small(5, 55, "Temp  :  26");
	draw_string_small(92, 55, "C");
	glcd_draw_circle(88, 55, 1, BLACK);
}
uint8_t _Cb_LCD_Display(uint8_t x)
{
	blink_display();

	switch(interface)
	{
		case MAIN:
			main_display();

			break;
		case LOGIN:
			glcd_clear_buffer();
			draw_string_small(5, 1, "Loggin");
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(30, 15, "Enter Password");

			char str[5];
			for( int i=0; i<4; i++) {
				str[i] = password[i] + '0';
				if ( str[i] > '9') {
					str[i]= '0';
					password[i] = 0;
				} else if (str[i] < '0'){
					str[i] = '9';
					password[i] = 9;
				}
			}
			str[4] = '\0';
//			draw_string_small(50, 30, str);
			for(int i=0;i<4;i++)
			{
			    if(i == pass_cur && blink == 0)
			        str[i] = ' ';     // ẩn số đang chọn
			}
			draw_string_small(50,30,str);

			break;
		case SETTING:
		    glcd_clear_buffer();
		    glcd_draw_line(0, 8, 128, 8, BLACK);
		    draw_string_small(5, 0, "SETTING");

		    if(setting_cursor != MODBUS_CUR || blink)
		        draw_string_small(5, 10, "1. Modbus RTU");

		    if(setting_cursor != CALIB_CUR || blink)
		        draw_string_small(5, 18, "2. Calib");

		    if(setting_cursor != OFFSET_CUR || blink)
		        draw_string_small(5, 26, "3. Offset");

		    if(setting_cursor != WARNING_CUR || blink)
		        draw_string_small(5, 34, "4. Warning");

		    if(setting_cursor != RANGE_CUR || blink)
		        draw_string_small(5, 42, "5. Range");

		    if(setting_cursor != INFO_CUR || blink)
		        draw_string_small(5, 50, "6. Information");

		    break;
		case MODBUS:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Modbus RTU");
			draw_string_small(5, 20, "Set ID: 1");
			draw_string_small(5, 30, "Set Baudrate: 9600");
			break;
		case CALIB:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Calib");
			draw_string_small(5, 20, "Zero point");
			draw_string_small(5, 35, "Slope point");
			draw_string_small(5, 53, "Note: set PH to 7.5");
			break;
		case OFFSET:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Offset");
			draw_string_small(5, 30, "offset mV: 0 mV");
			break;
		case WARNING:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Warning");
			draw_string_small(5, 20, "Mode: ON");
			draw_string_small(5, 30, "Upper: 1 mg/L");
			draw_string_small(5, 40, "Lower: 0.2 mg/L");
			break;
		case RANGE:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Range");
			draw_string_small(5, 30, "Set range: 0 - 2 mg/L");

			break;
		case INFO:
			glcd_clear_buffer();
			glcd_draw_line(0, 10, 128, 10, BLACK);
			draw_string_small(5, 0, "Information");
			draw_string_small(50, 30, "Sao Viet");
			break;
		default:
			break;
	}
	  return 0;
}

uint8_t _Cb_Button(uint8_t x)
{
	if (HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0) {
		while (HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0) {}
		if (interface == MAIN) {
			interface = LOGIN;

		} else if (interface == LOGIN) {
			if(pass_cur == 3) {
				pass_cur = 0;
				if(password[0]==1 &&
				   password[1]==1 &&
				   password[2]==1 &&
				   password[3]==1) interface = SETTING;
				else interface = MAIN;
				for ( int i=0; i<4 ; i++) password[i]=0;
				pass_cur = 0;
			}
			else pass_cur = pass_cur +1;

		} else if ( interface == SETTING) {
			switch(setting_cursor)
			{
			case MODBUS_CUR:
				interface = MODBUS;
				break;
			case CALIB_CUR:
				interface = CALIB;
				break;
			case OFFSET_CUR:
				interface = OFFSET;
				break;
			case WARNING_CUR:
				interface = WARNING;
				break;
			case RANGE_CUR:
				interface = RANGE;
				break;
			case INFO_CUR:
				interface = INFO;
				break;
			default:
				break;
			}
		}
	} else if (HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0) {
		while (HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0) {}
		if (interface == LOGIN) {
					password[pass_cur] = password[pass_cur] - 1;
				} else if (interface == SETTING) {
					setting_cursor = setting_cursor + 1;
					if (setting_cursor>5) setting_cursor = 0;
				}
	} else if (HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0) {
		while (HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0) {}
		if (interface == LOGIN) {
			password[pass_cur] = password[pass_cur] + 1;
		} else if (interface == SETTING) {
			setting_cursor = setting_cursor - 1;
			if (setting_cursor<0) setting_cursor = 5;
		}
	} else if (HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0) {
		while (HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0) {}
		if( interface != MAIN && interface < MODBUS) {
		interface= interface -1;
		for ( int i=0; i<4 ; i++) password[i]=0;
		pass_cur = 0;
		} else if ( interface >= MODBUS) interface= SETTING;
	}
	return 0;
}
uint8_t Comm_Task(void)
{
	uint8_t i = 0;

	for (i = 0; i < 2; i++)
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
