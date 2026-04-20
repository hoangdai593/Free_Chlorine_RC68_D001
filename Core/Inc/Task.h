/*
 * Task.h
 *
 *  Created on: Apr 7, 2026
 *      Author: PCBOX
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

typedef enum {
	_EVENT_CHECK_EC200U,
	_EVENT_UART_RECEIVE
} EVENT_List;

typedef enum {
    STATE_IDLE,

    STATE_AT,          // kiểm tra module
    STATE_ATE0,        // tắt echo
    STATE_CPIN,        // kiểm tra SIM
    STATE_CSQ,         // kiểm tra sóng
    STATE_CEREG,       // kiểm tra mạng

    STATE_ATTACH,      // attach mạng
    STATE_PDP,         // cấu hình APN
    STATE_ACTIVE,      // kích hoạt PDP (có IP)
    STATE_GETIP,       // lấy IP

    STATE_MQTT_OPEN,   // mở TCP tới broker
    STATE_MQTT_CONN,   // connect MQTT
    STATE_MQTT_PUB,    // publish dữ liệu

    STATE_DONE
} EC200_State_t;

extern EC200_State_t ec_state;
extern uint8_t rx_byte;
void EC200_PowerOn(void);
void EC200_SendCommand(const char* cmd);
void PrintToUSART2(const char* str);
uint8_t Comm_Task(void);
uint8_t _Cb_EC200U_Check (uint8_t event_id);
uint8_t _Cb_UART_Receive(uint8_t event_id);


#endif /* INC_TASK_H_ */
