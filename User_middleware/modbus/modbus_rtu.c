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
    mb->last_rx_tick = HAL_GetTick();
}

/* ================= START RX ================= */
void MB_RTU_StartReceive(MB_RTU_t *mb)
{
    BSP_RS485_Receive_IT(mb->port, &mb->rx_byte);
}

/* ================= RX BYTE (VERY LIGHT) ================= */
void MB_RTU_RxByteHandler(MB_RTU_t *mb)
{
    if(mb->rx_index < MB_RX_BUF_SIZE)
    {
        mb->rx_buf[mb->rx_index++] = mb->rx_byte;
        mb->last_rx_tick = HAL_GetTick();
    }
    else
    {
        mb->status = MB_RTU_OVERFLOW;
        mb->rx_index = 0;
    }

    MB_RTU_StartReceive(mb);
}

/* ================= SEND (FIX RS485 STABILITY) ================= */
HAL_StatusTypeDef MB_RTU_Send(MB_RTU_t *mb, uint8_t *buf, uint16_t len)
{
    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(mb->port);
    if(!huart) return HAL_ERROR;

    /* IMPORTANT: chờ bus idle */
    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

    BSP_RS485_TX_Mode(mb->port);
    delay_us(5);

    HAL_StatusTypeDef ret = HAL_UART_Transmit(huart, buf, len, 100);

    /* đảm bảo shift register empty */
    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

    delay_us(5);
    BSP_RS485_RX_Mode(mb->port);

    /* restart RX an toàn */
    MB_RTU_StartReceive(mb);

    return ret;
}

/* ================= POLL ================= */
void MB_RTU_Poll(MB_RTU_t *mb)
{
    if(mb->rx_index == 0 || mb->frame_ready)
        return;

    if((HAL_GetTick() - mb->last_rx_tick) < MB_FRAME_TIMEOUT_MS)
        return;

    /* validate CRC */
    if(MB_CRC_Check(mb->rx_buf, mb->rx_index))
    {
        mb->frame_ready = 1;
        mb->status = MB_RTU_OK;
    }
    else
    {
        mb->status = MB_RTU_CRC_ERROR;
    }

    /* reset buffer sau khi quyết định frame */
    mb->rx_index = 0;
}
