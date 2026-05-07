/*
 * bsp_uart.c
 *
 *  Created on: Apr 29, 2026
 *      Author: PCBOX
 */
#include "bsp_uart.h"
#include "usart.h"

/*=========================================================
    PRIVATE UART HANDLE TABLE
=========================================================*/
static UART_HandleTypeDef* uart_table[] =
{
    &huart1,
    &huart3
};

/*=========================================================
    INIT
=========================================================*/
void BSP_UART_Init(void)
{
    /* nếu cần init thêm thì để đây */
}

/*=========================================================
    GET HANDLE
=========================================================*/
UART_HandleTypeDef* BSP_UART_GetHandle(BSP_UART_PORT port)
{
    if(port > BSP_UART3)
        return NULL;

    return uart_table[port];
}

/*=========================================================
    BLOCKING SEND
=========================================================*/
HAL_StatusTypeDef BSP_UART_Send(BSP_UART_PORT port,
                                uint8_t *buf,
                                uint16_t len,
                                uint32_t timeout)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Transmit(huart, buf, len, timeout);
}

/*=========================================================
    TX INTERRUPT
=========================================================*/
HAL_StatusTypeDef BSP_UART_Send_IT(BSP_UART_PORT port,
                                   uint8_t *buf,
                                   uint16_t len)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Transmit_IT(huart, buf, len);
}

/*=========================================================
    TX DMA
=========================================================*/
HAL_StatusTypeDef BSP_UART_Send_DMA(BSP_UART_PORT port,
                                    uint8_t *buf,
                                    uint16_t len)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Transmit_DMA(huart, buf, len);
}

/*=========================================================
    RX INTERRUPT 1 BYTE
=========================================================*/
HAL_StatusTypeDef BSP_UART_Receive_IT(BSP_UART_PORT port,
                                      uint8_t *byte)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Receive_IT(huart, byte, 1);
}

/*=========================================================
    RX DMA
=========================================================*/
HAL_StatusTypeDef BSP_UART_Receive_DMA(BSP_UART_PORT port,
                                       uint8_t *buf,
                                       uint16_t len)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    return HAL_UART_Receive_DMA(huart, buf, len);
}

/*=========================================================
    CHANGE BAUDRATE
=========================================================*/
HAL_StatusTypeDef BSP_UART_SetBaudrate(BSP_UART_PORT port,
                                       uint32_t baudrate)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    huart->Init.BaudRate = baudrate;

    HAL_UART_DeInit(huart);

    return HAL_UART_Init(huart);
}

/*=========================================================
    WAIT TX COMPLETE
=========================================================*/
void BSP_UART_WaitTC(BSP_UART_PORT port)
{
    UART_HandleTypeDef *huart = BSP_UART_GetHandle(port);

    if(huart == NULL)
        return;

    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);
}

