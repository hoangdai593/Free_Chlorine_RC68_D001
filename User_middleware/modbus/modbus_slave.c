#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

/* ================= CONFIG ================= */

#define RX_RESET_MS   100
#define T35_MS        4

/* ================= INTERNAL ================= */

static void MB_Send(MB_SLAVE_t *s, uint16_t len);
static void MB_Except(MB_SLAVE_t *s, uint8_t ex);

/* =========================================================
 * INIT
 * ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *s,
                   RS485_PORT port,
                   uint8_t id)
{
    memset(s, 0, sizeof(MB_SLAVE_t));

    s->port     = port;
    s->slave_id = id;

    s->holding_reg[0x0000] = id;
    s->holding_reg[0x0001] = 3;

    BSP_RS485_RX_Mode(port);

    BSP_RS485_Receive_IT(port,
                         &s->rx_byte);
}

/* =========================================================
 * RX BYTE
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s,
                            uint8_t byte)
{
    if(s->rx_index < MB_RX_BUF_SIZE)
    {
        s->rx_buf[s->rx_index++] = byte;
    }
    else
    {
        s->rx_index = 0;
    }

    s->last_rx_tick = HAL_GetTick();
}

/* =========================================================
 * POLL
 * ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    uint32_t now = HAL_GetTick();

    if(s->rx_index == 0)
        return;

    /* timeout reset */
    if((now - s->last_rx_tick) > RX_RESET_MS)
    {
        s->rx_index = 0;
        return;
    }

    /* wait end frame */
    if((now - s->last_rx_tick) < T35_MS)
        return;

    /* minimal frame */
    if(s->rx_index < 8)
        goto reset;

    /* slave id */
    if(s->rx_buf[0] != s->slave_id)
        goto reset;

    /* crc */
    if(!MB_CRC_Check(s->rx_buf,
                     s->rx_index))
    {
        goto reset;
    }

    uint8_t func = s->rx_buf[1];

    /* =====================================================
     * READ HOLDING REGISTER
     * ===================================================== */

    if(func == 0x03)
    {
        uint16_t addr =
                (s->rx_buf[2] << 8) |
                 s->rx_buf[3];

        uint16_t qty =
                (s->rx_buf[4] << 8) |
                 s->rx_buf[5];

        if((addr + qty) > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        s->tx_buf[0] = s->slave_id;
        s->tx_buf[1] = 0x03;
        s->tx_buf[2] = qty * 2;

        for(uint16_t i = 0; i < qty; i++)
        {
            uint16_t v = s->holding_reg[addr + i];

            s->tx_buf[3 + i * 2] = v >> 8;
            s->tx_buf[4 + i * 2] = v & 0xFF;
        }

        MB_Send(s,
                3 + qty * 2);
    }

    /* =====================================================
     * WRITE SINGLE REGISTER
     * ===================================================== */

    else if(func == 0x06)
    {
        uint16_t addr =
                (s->rx_buf[2] << 8) |
                 s->rx_buf[3];

        uint16_t val =
                (s->rx_buf[4] << 8) |
                 s->rx_buf[5];

        if(addr >= MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        s->holding_reg[addr] = val;

        memcpy(s->tx_buf,
               s->rx_buf,
               6);

        MB_Send(s, 6);
    }

    /* =====================================================
     * WRITE MULTI REGISTER
     * ===================================================== */

    else if(func == 0x10)
    {
        uint16_t addr =
                (s->rx_buf[2] << 8) |
                 s->rx_buf[3];

        uint16_t qty =
                (s->rx_buf[4] << 8) |
                 s->rx_buf[5];

        if((addr + qty) > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        for(uint16_t i = 0; i < qty; i++)
        {
            s->holding_reg[addr + i] =
                    (s->rx_buf[7 + i * 2] << 8) |
                     s->rx_buf[8 + i * 2];
        }

        memcpy(s->tx_buf,
               s->rx_buf,
               6);

        MB_Send(s, 6);
    }

    else
    {
        MB_Except(s, 0x01);
    }

reset:
    s->rx_index = 0;
}

/* =========================================================
 * SEND
 * ========================================================= */

static void MB_Send(MB_SLAVE_t *s,
                    uint16_t len)
{
    UART_HandleTypeDef *huart;

    huart = BSP_RS485_GetHandle(s->port);

    if(huart == NULL)
        return;

    MB_CRC_Append(s->tx_buf, len);

    BSP_RS485_TX_Mode(s->port);

    HAL_UART_Transmit(huart,
                      s->tx_buf,
                      len + 2,
                      100);

    while(__HAL_UART_GET_FLAG(huart,
                              UART_FLAG_TC) == RESET);

    delay_us(100);

    BSP_RS485_RX_Mode(s->port);

    HAL_UART_Receive_IT(huart,
                        &s->rx_byte,
                        1);
}

/* =========================================================
 * EXCEPTION
 * ========================================================= */

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t ex)
{
    s->tx_buf[0] = s->slave_id;
    s->tx_buf[1] = s->rx_buf[1] | 0x80;
    s->tx_buf[2] = ex;

    MB_Send(s, 3);
}

/* =========================================================
 * API
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s,
                       uint16_t addr,
                       float v)
{
    if((addr + 1) >= MB_SLAVE_MAX_REG)
        return;

    uint32_t raw;

    memcpy(&raw, &v, 4);

    s->holding_reg[addr]     = raw >> 16;
    s->holding_reg[addr + 1] = raw & 0xFFFF;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s,
                     uint16_t addr,
                     uint16_t v)
{
    if(addr >= MB_SLAVE_MAX_REG)
        return;

    s->holding_reg[addr] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s,
                        uint16_t addr)
{
    uint32_t raw;

    raw =
        ((uint32_t)s->holding_reg[addr] << 16) |
         s->holding_reg[addr + 1];

    float v;

    memcpy(&v, &raw, 4);

    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s,
                         uint16_t addr)
{
    if(addr >= MB_SLAVE_MAX_REG)
        return 0;

    return s->holding_reg[addr];
}
