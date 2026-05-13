#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

#define MB_BAUD_COUNT 11

static const uint32_t MB_BAUD[MB_BAUD_COUNT] =
{
    1200,2400,4800,9600,14400,
    19200,28800,38400,56000,57600,115200
};

static void MB_Send(MB_SLAVE_t *s,
                    uint8_t *buf,
                    uint16_t len);

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t func,
                      uint8_t ex);

/* ========================================================= */

static void MB_UpdateBaud(MB_SLAVE_t *s)
{
    uint16_t b = s->holding_reg[0x0001];

    if(b >= MB_BAUD_COUNT)
        return;

    BSP_RS485_SetBaudrate(s->port,
                          MB_BAUD[b]);
}

/* ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *s,
                   RS485_PORT port,
                   uint8_t id)
{
    memset(s, 0, sizeof(MB_SLAVE_t));

    s->port     = port;
    s->slave_id = id;

    s->holding_reg[0x0000] = id;
    s->holding_reg[0x0001] = 3;

    MB_SLAVE_SetFloat(s, 0x0006, 7.5f);
    MB_SLAVE_SetU16(s, 0x000D, 0);
    MB_SLAVE_SetFloat(s, 0x000E, 1.5f);
    MB_SLAVE_SetFloat(s, 0x0010, 0.1f);

    BSP_RS485_RX_Mode(port);

    BSP_RS485_Receive_IT(port,
                         &s->rx_byte);
}

/* ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s,
                            uint8_t b)
{
    if(s->rx_index >= MB_RX_BUF_SIZE)
    {
        s->rx_index = 0;
    }

    s->rx_buf[s->rx_index++] = b;

    s->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(s->port,
                         &s->rx_byte);
}

/* ========================================================= */

static void MB_Send(MB_SLAVE_t *s,
                    uint8_t *buf,
                    uint16_t len)
{
    UART_HandleTypeDef *huart;

    huart = BSP_RS485_GetHandle(s->port);

    if(huart == NULL)
        return;

    MB_CRC_Append(buf, len);

    BSP_RS485_TX_Mode(s->port);

    delay_us(100);

    HAL_UART_Transmit(huart,
                      buf,
                      len + 2,
                      200);

    BSP_RS485_WaitTC(s->port);

    delay_us(100);

    BSP_RS485_RX_Mode(s->port);
}

/* ========================================================= */

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t func,
                      uint8_t ex)
{
    s->tx_buf[0] = s->slave_id;
    s->tx_buf[1] = func | 0x80;
    s->tx_buf[2] = ex;

    MB_Send(s,
            s->tx_buf,
            3);
}

/* ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    uint32_t now;

    now = HAL_GetTick();

    if(s->rx_index < 8)
        return;

    /* frame timeout */
    if((now - s->last_rx_tick) <
        MB_SLAVE_T35_MS)
    {
        return;
    }

    /* reset stale */
    if((now - s->last_rx_tick) >
        MB_SLAVE_RX_RESET_MS)
    {
        s->rx_index = 0;
        return;
    }

    /* validate */
    if(s->rx_buf[0] != s->slave_id)
        goto reset;

    if(MB_CRC_Check(s->rx_buf,
                    s->rx_index) == 0)
    {
        goto reset;
    }

    uint8_t func = s->rx_buf[1];

    /* ===================================================== */
    /* READ HOLDING */
    /* ===================================================== */

    if(func == 0x03)
    {
        uint16_t addr;
        uint16_t qty;

        addr =
            ((uint16_t)s->rx_buf[2] << 8) |
             s->rx_buf[3];

        qty =
            ((uint16_t)s->rx_buf[4] << 8) |
             s->rx_buf[5];

        if((qty == 0) ||
           (qty > 125))
        {
            MB_Except(s, func, 0x03);
            goto reset;
        }

        if((addr + qty) >
            MB_SLAVE_MAX_REG)
        {
            MB_Except(s, func, 0x02);
            goto reset;
        }

        s->tx_buf[0] = s->slave_id;
        s->tx_buf[1] = 0x03;
        s->tx_buf[2] = qty * 2;

        for(uint16_t i = 0; i < qty; i++)
        {
            uint16_t v =
                s->holding_reg[addr + i];

            s->tx_buf[3 + i * 2] =
                v >> 8;

            s->tx_buf[4 + i * 2] =
                v & 0xFF;
        }

        MB_Send(s,
                s->tx_buf,
                3 + qty * 2);

        goto reset;
    }

    /* ===================================================== */
    /* WRITE SINGLE */
    /* ===================================================== */

    if(func == 0x06)
    {
        uint16_t addr;
        uint16_t val;

        addr =
            ((uint16_t)s->rx_buf[2] << 8) |
             s->rx_buf[3];

        val =
            ((uint16_t)s->rx_buf[4] << 8) |
             s->rx_buf[5];

        if(addr >= MB_SLAVE_MAX_REG)
        {
            MB_Except(s, func, 0x02);
            goto reset;
        }

        s->holding_reg[addr] = val;

        memcpy(s->tx_buf,
               s->rx_buf,
               6);

        MB_Send(s,
                s->tx_buf,
                6);

        /* đổi baud SAU ACK */
        if(addr == 0x0001)
        {
            HAL_Delay(20);
            MB_UpdateBaud(s);
        }

        goto reset;
    }

    /* ===================================================== */
    /* WRITE MULTI */
    /* ===================================================== */

    if(func == 0x10)
    {
        uint16_t addr;
        uint16_t qty;

        addr =
            ((uint16_t)s->rx_buf[2] << 8) |
             s->rx_buf[3];

        qty =
            ((uint16_t)s->rx_buf[4] << 8) |
             s->rx_buf[5];

        if((qty == 0) ||
           (qty > 123))
        {
            MB_Except(s, func, 0x03);
            goto reset;
        }

        if((addr + qty) >
            MB_SLAVE_MAX_REG)
        {
            MB_Except(s, func, 0x02);
            goto reset;
        }

        for(uint16_t i = 0; i < qty; i++)
        {
            s->holding_reg[addr + i] =
                ((uint16_t)s->rx_buf[7 + i * 2] << 8) |
                 s->rx_buf[8 + i * 2];
        }

        s->tx_buf[0] = s->slave_id;
        s->tx_buf[1] = 0x10;
        s->tx_buf[2] = s->rx_buf[2];
        s->tx_buf[3] = s->rx_buf[3];
        s->tx_buf[4] = s->rx_buf[4];
        s->tx_buf[5] = s->rx_buf[5];

        MB_Send(s,
                s->tx_buf,
                6);

        /* đổi baud SAU ACK */
        if((addr <= 0x0001) &&
           ((addr + qty) > 0x0001))
        {
            HAL_Delay(20);
            MB_UpdateBaud(s);
        }

        goto reset;
    }

    MB_Except(s,
              func,
              0x01);

reset:
    s->rx_index = 0;
}

/* ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s,
                       uint16_t a,
                       float v)
{
    uint32_t r;

    if((a + 1) >= MB_SLAVE_MAX_REG)
        return;

    memcpy(&r, &v, 4);

    s->holding_reg[a] =
        (r >> 16);

    s->holding_reg[a + 1] =
        (r & 0xFFFF);
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s,
                     uint16_t a,
                     uint16_t v)
{
    if(a >= MB_SLAVE_MAX_REG)
        return;

    s->holding_reg[a] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s,
                        uint16_t a)
{
    uint32_t r;
    float v;

    r =
        ((uint32_t)s->holding_reg[a] << 16) |
         s->holding_reg[a + 1];

    memcpy(&v, &r, 4);

    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s,
                         uint16_t a)
{
    return s->holding_reg[a];
}
