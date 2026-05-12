#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

/* ================= BAUD TABLE (GIỮ NGUYÊN CỦA BẠN) ================= */
#define MB_BAUD_COUNT 11
static const uint32_t MB_BAUD[MB_BAUD_COUNT] =
{
    1200,2400,4800,9600,14400,
    19200,28800,38400,56000,57600,115200
};

/* ================= INTERNAL ================= */

static void MB_Send(MB_SLAVE_t *s, uint16_t len)
{
    BSP_RS485_TX_Mode(s->port);
    delay_us(10);

    MB_CRC_Append(s->rx_buf, len);

    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(s->port);

    if(huart)
    {
        HAL_UART_Transmit(huart,
                          s->rx_buf,
                          len + 2,
                          100);
    }

    BSP_RS485_WaitTC(s->port);
    delay_us(10);

    BSP_RS485_RX_Mode(s->port);
    BSP_RS485_Receive_IT(s->port, &s->rx_byte);
}

/* ================= INIT ================= */

void MB_SLAVE_Init(MB_SLAVE_t *s,
                   RS485_PORT port,
                   uint8_t id)
{
    memset(s, 0, sizeof(MB_SLAVE_t));

    s->port = port;
    s->slave_id = id;

    s->holding_reg[0x0000] = id;
    s->holding_reg[0x0001] = 3;   // default baud index

    BSP_RS485_RX_Mode(port);
    BSP_RS485_Receive_IT(port, &s->rx_byte);
}

/* ================= RX ================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s, uint8_t byte)
{
    if(s->rx_index >= MB_RX_BUF_SIZE)
        s->rx_index = 0;

    s->rx_buf[s->rx_index++] = byte;
    s->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(s->port, &s->rx_byte);
}

/* ================= PROCESS ================= */

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    if(s->rx_index < 4) return;

    if(HAL_GetTick() - s->last_rx_tick < 3)
        return;

    uint8_t id = s->rx_buf[0];
    uint8_t func = s->rx_buf[1];

    if(id != s->slave_id)
    {
        s->rx_index = 0;
        return;
    }

    if(!MB_CRC_Check(s->rx_buf, s->rx_index))
    {
        s->rx_index = 0;
        return;
    }

    /* ================= FC03 ================= */
    if(func == 0x03)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t qty  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        s->rx_buf[1] = 0x03;
        s->rx_buf[2] = qty * 2;

        for(int i=0;i<qty;i++)
        {
            uint16_t v = s->holding_reg[addr+i];
            s->rx_buf[3+i*2] = v>>8;
            s->rx_buf[4+i*2] = v;
        }

        MB_Send(s, 3 + qty*2);
    }

    /* ================= FC06 ================= */
    else if(func == 0x06)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t val  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        s->holding_reg[addr] = val;

        /* ================= GIỮ NGUYÊN SET BAUD CỦA BẠN ================= */
        if(addr == 0x0001)
        {
            uint16_t code = s->holding_reg[0x0001];

            if(code < MB_BAUD_COUNT)
            {
                BSP_RS485_SetBaudrate(s->port, MB_BAUD[code]);
            }
        }

        MB_Send(s, 6);
    }

    /* ================= FC10 ================= */
    else if(func == 0x10)
    {
        uint16_t addr = (s->rx_buf[2]<<8)|s->rx_buf[3];
        uint16_t qty  = (s->rx_buf[4]<<8)|s->rx_buf[5];

        uint8_t bc = s->rx_buf[6];

        if(bc != qty*2)
        {
            s->rx_index = 0;
            return;
        }

        for(int i=0;i<qty;i++)
        {
            s->holding_reg[addr+i] =
                (s->rx_buf[7+i*2]<<8)|s->rx_buf[8+i*2];
        }

        /* ================= GIỮ NGUYÊN BAUD UPDATE ================= */
        if(addr <= 0x0001 && (addr+qty) > 0x0001)
        {
            uint16_t code = s->holding_reg[0x0001];

            if(code < MB_BAUD_COUNT)
            {
                BSP_RS485_SetBaudrate(s->port, MB_BAUD[code]);
            }
        }

        MB_Send(s, 6);
    }

    s->rx_index = 0;
}

/* ================= FLOAT ================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s, uint16_t a, float v)
{
    uint32_t r = *((uint32_t*)&v);
    s->holding_reg[a] = r>>16;
    s->holding_reg[a+1] = r&0xFFFF;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s, uint16_t a)
{
    uint32_t r =
        ((uint32_t)s->holding_reg[a]<<16)
        | s->holding_reg[a+1];

    return *((float*)&r);
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s, uint16_t a, uint16_t v)
{
    s->holding_reg[a] = v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s, uint16_t a)
{
    return s->holding_reg[a];
}
