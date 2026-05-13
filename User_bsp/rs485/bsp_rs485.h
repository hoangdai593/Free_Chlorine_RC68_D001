#ifndef __BSP_RS485_H
#define __BSP_RS485_H

#include "main.h"
#include <stdint.h>

/* ================= PORT ================= */
typedef enum
{
    RS485_PORT1 = 0,    // USART1 - Datalogger (Slave)
    RS485_PORT3         // USART3 - Sensor (Master)
} RS485_PORT;

/* ================= DIR CONTROL ================= */
#define RS485_1_DIR_PORT    GPIOB
#define RS485_1_DIR_PIN     GPIO_PIN_5

#define RS485_3_DIR_PORT    GPIOC
#define RS485_3_DIR_PIN     GPIO_PIN_12

/* ================= API ================= */
void BSP_RS485_Init(void);

void BSP_RS485_TX_Mode(RS485_PORT port);
void BSP_RS485_RX_Mode(RS485_PORT port);

HAL_StatusTypeDef BSP_RS485_Send(RS485_PORT port,
                                 uint8_t *buf,
                                 uint16_t len,
                                 uint32_t timeout);

HAL_StatusTypeDef BSP_RS485_Send_IT(RS485_PORT port,
                                   uint8_t *buf,
                                   uint16_t len);

HAL_StatusTypeDef BSP_RS485_Receive_IT(RS485_PORT port,
                                      uint8_t *byte);

HAL_StatusTypeDef BSP_RS485_SetBaudrate(RS485_PORT port,
                                        uint32_t baudrate);

void BSP_RS485_WaitTC(RS485_PORT port);

UART_HandleTypeDef* BSP_RS485_GetHandle(RS485_PORT port);

#endif
