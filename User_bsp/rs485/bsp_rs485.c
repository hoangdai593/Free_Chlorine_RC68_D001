/*=========================================================
 * File: bsp_rs485.c
 * Desc: RS485 BSP using STM32 HAL
 *=========================================================*/
#include "bsp_rs485.h"
#include "usart.h"

/*=========================================================
    PRIVATE
=========================================================*/
static UART_HandleTypeDef* RS485_GetHandle(RS485_PORT port)
{
    switch(port)
    {
        case RS485_PORT1: return &huart1;
        case RS485_PORT3: return &huart3;
        default: return NULL;
    }
}

/*=========================================================
    INIT
=========================================================*/
void BSP_RS485_Init(void)
{
    BSP_RS485_RX_Mode(RS485_PORT1);
    BSP_RS485_RX_Mode(RS485_PORT3);
}

/*=========================================================
    MODE CONTROL
=========================================================*/
void BSP_RS485_TX_Mode(RS485_PORT port)
{
    switch(port)
    {
        case RS485_PORT1:
            HAL_GPIO_WritePin(RS485_1_DIR_PORT,
                              RS485_1_DIR_PIN,
                              GPIO_PIN_SET);
            break;

        case RS485_PORT3:
            HAL_GPIO_WritePin(RS485_3_DIR_PORT,
                              RS485_3_DIR_PIN,
                              GPIO_PIN_SET);
            break;
    }
}

void BSP_RS485_RX_Mode(RS485_PORT port)
{
    switch(port)
    {
        case RS485_PORT1:
            HAL_GPIO_WritePin(RS485_1_DIR_PORT,
                              RS485_1_DIR_PIN,
                              GPIO_PIN_RESET);
            break;

        case RS485_PORT3:
            HAL_GPIO_WritePin(RS485_3_DIR_PORT,
                              RS485_3_DIR_PIN,
                              GPIO_PIN_RESET);
            break;
    }
}

/*=========================================================
    SEND BLOCKING
=========================================================*/
HAL_StatusTypeDef BSP_RS485_Send(RS485_PORT port,
                                 uint8_t *buf,
                                 uint16_t len,
                                 uint32_t timeout)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    BSP_RS485_TX_Mode(port);

    HAL_StatusTypeDef ret =
        HAL_UART_Transmit(huart, buf, len, timeout);

    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

    BSP_RS485_RX_Mode(port);

    return ret;
}

/*=========================================================
    SEND IT
=========================================================*/
HAL_StatusTypeDef BSP_RS485_Send_IT(RS485_PORT port,
                                    uint8_t *buf,
                                    uint16_t len)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    BSP_RS485_TX_Mode(port);

    return HAL_UART_Transmit_IT(huart, buf, len);
}

/*=========================================================
    SEND DMA
=========================================================*/
HAL_StatusTypeDef BSP_RS485_Send_DMA(RS485_PORT port,
                                     uint8_t *buf,
                                     uint16_t len)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    BSP_RS485_TX_Mode(port);

    return HAL_UART_Transmit_DMA(huart, buf, len);
}

/*=========================================================
    RECEIVE IT (1 BYTE)
=========================================================*/
HAL_StatusTypeDef BSP_RS485_Receive_IT(RS485_PORT port,
                                       uint8_t *byte)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    BSP_RS485_RX_Mode(port);

    return HAL_UART_Receive_IT(huart, byte, 1);
}

/*=========================================================
    RECEIVE DMA
=========================================================*/
HAL_StatusTypeDef BSP_RS485_Receive_DMA(RS485_PORT port,
                                        uint8_t *buf,
                                        uint16_t len)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    BSP_RS485_RX_Mode(port);

    return HAL_UART_Receive_DMA(huart, buf, len);
}

/*=========================================================
    SET BAUDRATE
=========================================================*/
HAL_StatusTypeDef BSP_RS485_SetBaudrate(RS485_PORT port,
                                        uint32_t baudrate)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return HAL_ERROR;

    huart->Init.BaudRate = baudrate;

    HAL_UART_DeInit(huart);

    return HAL_UART_Init(huart);
}

/*=========================================================
    WAIT TX COMPLETE
=========================================================*/
void BSP_RS485_WaitTC(RS485_PORT port)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);

    if(huart == NULL)
        return;

    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);
}
