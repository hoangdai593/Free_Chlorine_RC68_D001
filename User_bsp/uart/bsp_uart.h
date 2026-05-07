/*
 * bsp_uart.h
 *
 *  Created on: Apr 29, 2026
 *      Author: PCBOX
 */

#ifndef UART_BSP_UART_H_
#define UART_BSP_UART_H_


#include "main.h"

/*=========================================================
    UART PORT SELECT
=========================================================*/
typedef enum
{
    BSP_UART1 = 0,
    BSP_UART2,
    BSP_UART3
}BSP_UART_PORT;

/*=========================================================
    API
=========================================================*/
void BSP_UART_Init(void);

UART_HandleTypeDef* BSP_UART_GetHandle(BSP_UART_PORT port);

/* transmit */
HAL_StatusTypeDef BSP_UART_Send(BSP_UART_PORT port,
                                uint8_t *buf,
                                uint16_t len,
                                uint32_t timeout);

HAL_StatusTypeDef BSP_UART_Send_IT(BSP_UART_PORT port,
                                   uint8_t *buf,
                                   uint16_t len);

HAL_StatusTypeDef BSP_UART_Send_DMA(BSP_UART_PORT port,
                                    uint8_t *buf,
                                    uint16_t len);

/* receive */
HAL_StatusTypeDef BSP_UART_Receive_IT(BSP_UART_PORT port,
                                      uint8_t *byte);

HAL_StatusTypeDef BSP_UART_Receive_DMA(BSP_UART_PORT port,
                                       uint8_t *buf,
                                       uint16_t len);

/* config */
HAL_StatusTypeDef BSP_UART_SetBaudrate(BSP_UART_PORT port,
                                       uint32_t baudrate);

/* state */
void BSP_UART_WaitTC(BSP_UART_PORT port);

#endif /* UART_BSP_UART_H_ */
