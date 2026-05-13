#include "modbus_rtu.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"
#include <string.h>

/* ================= INIT ================= */
void MB_RTU_Init(MB_RTU_t *mb, RS485_PORT port, uint8_t slave_id)
{
    mb->port = port;
    mb->slave_id = slave_id;
    MB_RTU_Clear(mb);
    MB_RTU_StartReceive(mb);
}

/* ================= CLEAR ================= */
void MB_RTU_Clear(MB_RTU_t *mb)
{
    mb->rx_index = 0;
    mb->frame_ready = 0;
    mb->status = MB_RTU_OK;
//    mb->last_rx_tick = 0;
//    memset(mb->rx_buf, 0, MB_RX_BUF_SIZE);
}

/* ================= START RX ================= */
void MB_RTU_StartReceive(MB_RTU_t *mb)
{
    BSP_RS485_Receive_IT(mb->port, &mb->rx_byte);
}

/* ================= RX BYTE ================= */
void MB_RTU_RxByteHandler(MB_RTU_t *mb)
{
    if(mb->rx_index >= MB_RX_BUF_SIZE)
    {
        MB_RTU_Clear(mb);
        MB_RTU_StartReceive(mb);
        return;
    }

    mb->rx_buf[mb->rx_index++] = mb->rx_byte;
    mb->last_rx_tick = HAL_GetTick();
    MB_RTU_StartReceive(mb);
}

/* ================= SEND ================= */
HAL_StatusTypeDef MB_RTU_Send(MB_RTU_t *mb, uint8_t *buf, uint16_t len)
{
    MB_RTU_Clear(mb);

    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(mb->port);
    if(huart == NULL) return HAL_ERROR;

//    HAL_UART_AbortReceive_IT(huart);
    uint32_t start = HAL_GetTick();
    while(huart->RxState != HAL_UART_STATE_READY && (HAL_GetTick() - start) < 10);

    BSP_RS485_TX_Mode(mb->port);
    delay_us(10);

    HAL_StatusTypeDef ret = HAL_UART_Transmit(huart, buf, len, 100);

    BSP_RS485_WaitTC(mb->port);
    delay_us(10);

    BSP_RS485_RX_Mode(mb->port);
    MB_RTU_StartReceive(mb);

    return ret;
}

/* ================= POLL ================= */
void MB_RTU_Poll(MB_RTU_t *mb)
{
    if(mb->rx_index == 0 || mb->frame_ready) return;
    if((HAL_GetTick() - mb->last_rx_tick) < MB_FRAME_TIMEOUT_MS) return;

    if(MB_CRC_Check(mb->rx_buf, mb->rx_index))
    {
        mb->status = MB_RTU_OK;
        mb->frame_ready = 1;
    }
    else
    {
        mb->status = MB_RTU_CRC_ERROR;
        MB_RTU_Clear(mb);
    }
}
