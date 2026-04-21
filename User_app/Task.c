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

EC200_State_t ec_state = STATE_AT;


sEvent_struct ALLTASK[] =
{
	{_EVENT_CHECK_EC200U, 1, 0, 2000, _Cb_EC200U_Check},   // chạy mỗi 1s
	{_EVENT_UART_RECEIVE, 0, 0, 0, _Cb_UART_Receive},   // chỉ trigger bằng interrupt
};

#define RX_BUFFER_SIZE  1024     // Buffer nhận từ EC200U (USART3)
#define PWRKEY_GPIO_Port GPIOA   //  PWRKEY
#define PWRKEY_Pin       GPIO_PIN_4

uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_byte;
volatile uint16_t rx_index = 0;
uint32_t state_tick = 0;
uint8_t hello=0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        if (rx_index < RX_BUFFER_SIZE - 1)
        {
            // Lưu byte vào buffer
            rx_buffer[rx_index++] = rx_byte;
            rx_buffer[rx_index] = '\0';  // luôn giữ string hợp lệ

            //Khi có ít nhất 1 byte, trigger event
            if (rx_byte=='\n')
            {
                fevent_active(ALLTASK, _EVENT_UART_RECEIVE);

            }
        }
        else
        {
            // Buffer full → reset để tránh tràn
            rx_index = 0;
        }

        // Nhận tiếp byte
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}


void EC200_PowerOn(void)
{
    PrintToUSART2("Power ON. Waiting for startup...\r\n");
    HAL_GPIO_WritePin(PWRKEY_GPIO_Port, PWRKEY_Pin, GPIO_PIN_SET);
    HAL_Delay(2500);                          // >= 2.5s, mình tăng lên 3s an toàn
    HAL_GPIO_WritePin(PWRKEY_GPIO_Port, PWRKEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(6000);                         // chờ 10 giây (an toàn hơn)
}


void PrintToUSART2(const char* str)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 200);
}


uint8_t _Cb_EC200U_Check(uint8_t event_id)
{
	fevent_disable(ALLTASK, _EVENT_CHECK_EC200U);
    switch (ec_state)
    {
        case STATE_AT:
            EC200_SendCommand("AT");
            HAL_Delay(300);
            ec_state = STATE_ATE0;
            break;

        case STATE_ATE0:
            EC200_SendCommand("ATE0");
            HAL_Delay(300);
            ec_state = STATE_CPIN;
            break;

        case STATE_CPIN:
            EC200_SendCommand("AT+CPIN?");
            HAL_Delay(300);
            ec_state = STATE_CSQ;
            break;

        case STATE_CSQ:
            EC200_SendCommand("AT+CSQ");
            HAL_Delay(300);
            ec_state = STATE_CEREG;
            break;

        case STATE_CEREG:
            EC200_SendCommand("AT+CEREG?");
            HAL_Delay(500);
            EC200_SendCommand("AT+COPS?");
			HAL_Delay(500);
            ec_state = STATE_ATTACH;
            break;

        case STATE_ATTACH:
            EC200_SendCommand("AT+CGATT=1");
            HAL_Delay(1000);
            ec_state = STATE_PDP;
            break;

        case STATE_PDP:
            EC200_SendCommand("AT+CGDCONT=1,\"IP\",\"internet\"");
            HAL_Delay(300);
            ec_state = STATE_ACTIVE;
            break;

        case STATE_ACTIVE:
            EC200_SendCommand("AT+CGACT=1,1");
            HAL_Delay(2000);   // chờ cấp IP
            ec_state = STATE_GETIP;
            break;

        case STATE_GETIP:
            EC200_SendCommand("AT+CGPADDR=1");
            HAL_Delay(1000);
            ec_state = STATE_MQTT_OPEN;
            break;

        case STATE_MQTT_OPEN:
            EC200_SendCommand("AT+QMTOPEN=0,\"broker.hivemq.com\",1883");
            // mở TCP tới HiveMQ broker
            HAL_Delay(3000);   // chờ +QMTOPEN: 0,0
            ec_state = STATE_MQTT_CONN;
            break;

        case STATE_MQTT_CONN:
            EC200_SendCommand("AT+QMTCONN=0,\"stm32_client\"");
            // connect MQTT
            HAL_Delay(2000);   // chờ +QMTCONN: 0,0,0
            ec_state = STATE_MQTT_PUB;
            break;

        case STATE_MQTT_PUB:
            EC200_SendCommand("AT+QMTPUB=0,0,0,0,\"test/topic5555\"");
            HAL_Delay(300); // đợi dấu >

            char buf[20];
            int len = sprintf(buf, "Hello %d times", hello);  // trả về số ký tự, không tính '\0'
            sprintf(buf, "Hello %d times", hello);
            // gửi payload KHÔNG kèm \r\n
            HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);
            hello++;

            uint8_t ctrlz = 0x1A;
            HAL_UART_Transmit(&huart3, &ctrlz, 1, 1000);

            HAL_Delay(1000);

            ec_state = STATE_MQTT_PUB;
            break;

        case STATE_DONE:
            break;

        default:
            break;
    }

    return 0;
}

uint8_t _Cb_UART_Receive(uint8_t event_id)
{
    PrintToUSART2("<<< EC200U: ");
    PrintToUSART2((char*)rx_buffer);
    PrintToUSART2("\r\n");

    rx_index = 0;
    fevent_disable(ALLTASK, _EVENT_UART_RECEIVE);
    fevent_enable(ALLTASK, _EVENT_CHECK_EC200U);
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
				ALLTASK[i].e_status = 0;  //Disable event
				ALLTASK[i].e_systick = HAL_GetTick();
				ALLTASK[i].e_function_handler(i);
			}
		}
	}

	return 0;
}
