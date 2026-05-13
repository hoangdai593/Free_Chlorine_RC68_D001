#include "bsp_rs485.h"
#include "usart.h"
#include "gpio.h"
#include "delay.h"

/* =========================================================
   INTERNAL
========================================================= */
static UART_HandleTypeDef* RS485_GetHandle(RS485_PORT port)
{
    switch(port)
    {
        case RS485_PORT1: return &huart1;
        case RS485_PORT3: return &huart3;
        default: return NULL;
    }
}

UART_HandleTypeDef* BSP_RS485_GetHandle(RS485_PORT port)
{
    return RS485_GetHandle(port);
}

/* =========================================================
   INIT
========================================================= */
void BSP_RS485_Init(void)
{
    BSP_RS485_RX_Mode(RS485_PORT1);
    BSP_RS485_RX_Mode(RS485_PORT3);
}

/* =========================================================
   SAFE DIR CONTROL (FIX GLITCH UART1)
========================================================= */
static void rs485_dir_delay(void)
{
    delay_us(3);   // critical for MAX485 / SN65 / generic transceiver
}

/* =========================================================
   MODE CONTROL
========================================================= */
void BSP_RS485_TX_Mode(RS485_PORT port)
{
    if(port == RS485_PORT1)
        HAL_GPIO_WritePin(RS485_1_DIR_PORT, RS485_1_DIR_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(RS485_3_DIR_PORT, RS485_3_DIR_PIN, GPIO_PIN_SET);

    rs485_dir_delay();
}

void BSP_RS485_RX_Mode(RS485_PORT port)
{
    if(port == RS485_PORT1)
        HAL_GPIO_WritePin(RS485_1_DIR_PORT, RS485_1_DIR_PIN, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(RS485_3_DIR_PORT, RS485_3_DIR_PIN, GPIO_PIN_RESET);

    rs485_dir_delay();
}

/* =========================================================
   BLOCKING SEND (SLAVE SAFE)
========================================================= */
HAL_StatusTypeDef BSP_RS485_Send(RS485_PORT port,
                                 uint8_t *buf,
                                 uint16_t len,
                                 uint32_t timeout)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);
    if(!huart) return HAL_ERROR;

    BSP_RS485_TX_Mode(port);

    HAL_StatusTypeDef ret = HAL_UART_Transmit(huart, buf, len, timeout);

    /* WAIT REAL TX COMPLETE (IMPORTANT) */
    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

    BSP_RS485_RX_Mode(port);

    return ret;
}

/* =========================================================
   INTERRUPT SEND (MASTER SENSOR SAFE FIX)
========================================================= */
HAL_StatusTypeDef BSP_RS485_Send_IT(RS485_PORT port,
                                   uint8_t *buf,
                                   uint16_t len)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);
    if(!huart) return HAL_ERROR;

    BSP_RS485_TX_Mode(port);

    /* IMPORTANT:
       KHÔNG bật RX lại ở đây
       RX sẽ bật trong TC callback */
    return HAL_UART_Transmit_IT(huart, buf, len);
}

/* =========================================================
   RECEIVE IT
========================================================= */
HAL_StatusTypeDef BSP_RS485_Receive_IT(RS485_PORT port,
                                      uint8_t *byte)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);
    if(!huart) return HAL_ERROR;

    BSP_RS485_RX_Mode(port);

    return HAL_UART_Receive_IT(huart, byte, 1);
}

/* =========================================================
   BAUDRATE
========================================================= */
HAL_StatusTypeDef BSP_RS485_SetBaudrate(RS485_PORT port,
                                        uint32_t baudrate)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);
    if(!huart) return HAL_ERROR;

    huart->Init.BaudRate = baudrate;

    HAL_UART_DeInit(huart);
    return HAL_UART_Init(huart);
}

/* =========================================================
   WAIT TC (ROBUST)
========================================================= */
void BSP_RS485_WaitTC(RS485_PORT port)
{
    UART_HandleTypeDef *huart = RS485_GetHandle(port);
    if(!huart) return;

    /* thêm check busy tránh fake TC */
    while((__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET) ||
          (huart->gState != HAL_UART_STATE_READY));
}
