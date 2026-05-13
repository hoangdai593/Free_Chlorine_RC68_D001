#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"
#include <string.h>

/* ================= INTERNAL ================= */
static void MB_Send(MB_SLAVE_t *s, uint16_t len);
static void MB_Except(MB_SLAVE_t *s, uint8_t ex);

/* ================= INIT ================= */
void MB_SLAVE_Init(MB_SLAVE_t *s, RS485_PORT port, uint8_t id)
{
    memset(s, 0, sizeof(MB_SLAVE_t));

    s->port = port;
    s->slave_id = id;

    s->holding_reg[0] = id;

    BSP_RS485_RX_Mode(port);
    BSP_RS485_Receive_IT(port, &s->rx_byte);
}

/* ================= RX HANDLER (ISR SAFE) ================= */
void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s, uint8_t b)
{
    if(s->rx_lock) return;

    if(s->rx_index < MB_RX_BUF_SIZE)
    {
        s->rx_buf[s->rx_index++] = b;
        s->last_rx_tick = HAL_GetTick();
    }
    else
    {
        s->rx_index = 0;
    }

    BSP_RS485_Receive_IT(s->port, &s->rx_byte);
}

/* ================= POLL ================= */
void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    if(s->rx_index == 0) return;
    if(s->frame_ready) return;

    uint32_t now = HAL_GetTick();

    /* wait end frame */
    if(now - s->last_rx_tick < 3)
        return;

    s->rx_lock = 1;

    /* validate slave */
    if(s->rx_buf[0] != s->slave_id)
        goto reset;

    if(!MB_CRC_Check(s->rx_buf, s->rx_index))
        goto reset;

    uint8_t f = s->rx_buf[1];

    /* ================= FUNC 0x03 ================= */
    if(f == 0x03)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t qty  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        if(qty == 0 || addr + qty > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        s->rx_buf[1] = 0x03;
        s->rx_buf[2] = qty * 2;

        for(int i=0;i<qty;i++)
        {
            uint16_t v = s->holding_reg[addr+i];
            s->rx_buf[3+i*2] = v >> 8;
            s->rx_buf[4+i*2] = v;
        }

        MB_Send(s, 3 + qty*2);
    }

    /* ================= FUNC 0x06 ================= */
    else if(f == 0x06)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t val  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        if(addr < MB_SLAVE_MAX_REG)
            s->holding_reg[addr] = val;

        MB_Send(s, 6);
    }

    /* ================= FUNC 0x10 ================= */
    else if(f == 0x10)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t qty  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        if(addr + qty > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        for(int i=0;i<qty;i++)
        {
            s->holding_reg[addr+i] =
                (s->rx_buf[7+i*2]<<8) |
                (s->rx_buf[8+i*2]);
        }

        MB_Send(s, 6);
    }
    else
    {
        MB_Except(s, 0x01);
    }

reset:
    s->rx_index = 0;
    s->rx_lock = 0;
}

/* ================= TX SAFE ================= */
static void MB_Send(MB_SLAVE_t *s, uint16_t len)
{
    MB_CRC_Append(s->rx_buf, len);

    /* TX non-blocking */
    BSP_RS485_Send_IT(s->port, s->rx_buf, len + 2);
}

/* ================= EXCEPTION ================= */
static void MB_Except(MB_SLAVE_t *s, uint8_t ex)
{
    s->rx_buf[1] |= 0x80;
    s->rx_buf[2] = ex;

    MB_Send(s, 3);
}

/* ================= API ================= */
void MB_SLAVE_SetFloat(MB_SLAVE_t *s, uint16_t a, float v)
{
    if(a+1 >= MB_SLAVE_MAX_REG) return;

    uint32_t r;
    memcpy(&r, &v, 4);

    s->holding_reg[a]   = r >> 16;
    s->holding_reg[a+1] = r;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s, uint16_t a, uint16_t v)
{
    if(a >= MB_SLAVE_MAX_REG) return;
    s->holding_reg[a] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s, uint16_t a)
{
    uint32_t r =
        ((uint32_t)s->holding_reg[a] << 16) |
        s->holding_reg[a+1];

    float v;
    memcpy(&v, &r, 4);
    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s, uint16_t a)
{
    return s->holding_reg[a];
}
