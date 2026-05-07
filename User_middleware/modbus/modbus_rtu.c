#include "modbus_rtu.h"
#include <string.h>

/* ================= INIT ================= */
void MB_RTU_Init(MB_RTU_t *mb, RS485_PORT port, uint8_t slave_id)
{
    mb->port = port;
    mb->slave_id = slave_id;

    mb->rx_index = 0;
    mb->last_rx_tick = 0;
    mb->frame_ready = 0;

    memset(mb->rx_buf, 0, MB_RX_BUF_SIZE);

    MB_RTU_StartReceive(mb);
}

/* ================= START RECEIVE ================= */
void MB_RTU_StartReceive(MB_RTU_t *mb)
{
    BSP_RS485_Receive_IT(mb->port, &mb->rx_byte);
}

/* ================= RX BYTE HANDLER ================= */
void MB_RTU_RxByteHandler(MB_RTU_t *mb)
{
    if(mb->rx_index < MB_RX_BUF_SIZE)
    {
        mb->rx_buf[mb->rx_index++] = mb->rx_byte;
    }
    mb->last_rx_tick = HAL_GetTick();
    MB_RTU_StartReceive(mb);
}

/* ================= SEND ================= */
HAL_StatusTypeDef MB_RTU_Send(MB_RTU_t *mb, uint8_t *buf, uint16_t len)
{
    return BSP_RS485_Send(mb->port, buf, len, 100);
}

/* ================= POLL - TỐI ƯU CHO WRITE 0x10 ================= */
void MB_RTU_Poll(MB_RTU_t *mb)
{
    if(mb->rx_index == 0)
        return;

    /* Timeout ngắn hơn cho lệnh Write (0x06 & 0x10) */
    uint32_t timeout = 120;   // 120ms đủ cho response Write

    if((HAL_GetTick() - mb->last_rx_tick) >= timeout)
    {
        mb->frame_ready = 1;
    }
}
