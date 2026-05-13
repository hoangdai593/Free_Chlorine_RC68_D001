/*=========================================================
 * File: modbus_slave.c
 *=========================================================*/
#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

/* ================= BAUD TABLE ================= */

#define MB_BAUD_COUNT 11

static const uint32_t MB_BAUD[MB_BAUD_COUNT] =
{
    1200,
    2400,
    4800,
    9600,
    14400,
    19200,
    28800,
    38400,
    56000,
    57600,
    115200
};

/* ================= INTERNAL ================= */

static void MB_Send(MB_SLAVE_t *s,
                    uint8_t *buf,
                    uint16_t len);

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t func,
                      uint8_t ex);

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

    MB_SLAVE_SetFloat(s, 0x0006, 7.5f);
    MB_SLAVE_SetU16(s,   0x000D, 0);
    MB_SLAVE_SetFloat(s, 0x000E, 1.5f);
    MB_SLAVE_SetFloat(s, 0x0010, 0.1f);

    BSP_RS485_RX_Mode(port);

    BSP_RS485_Receive_IT(port,
                         &s->rx_byte);
}

/* =========================================================
 * RX BYTE
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s,
                            uint8_t b)
{
    if(s->rx_index < MB_RX_BUF_SIZE)
    {
        s->rx_buf[s->rx_index++] = b;
    }
    else
    {
        s->rx_index = 0;
    }

    s->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(s->port,
                         &s->rx_byte);
}

/* =========================================================
 * SEND
 * ========================================================= */

static void MB_Send(MB_SLAVE_t *s,
                    uint8_t *buf,
                    uint16_t len)
{
    UART_HandleTypeDef *huart;

    huart = BSP_RS485_GetHandle(s->port);

    if(huart == NULL)
        return;

    /* STOP RX IRQ */
    HAL_UART_AbortReceive_IT(huart);

    uint32_t t = HAL_GetTick();

    while((huart->RxState != HAL_UART_STATE_READY) &&
          ((HAL_GetTick() - t) < 10));

    MB_CRC_Append(buf, len);

    BSP_RS485_TX_Mode(s->port);

    delay_us(20);

    HAL_UART_Transmit(huart,
                      buf,
                      len + 2,
                      200);

    BSP_RS485_WaitTC(s->port);

    delay_us(20);

    BSP_RS485_RX_Mode(s->port);

    /* CLEAR RX STATE */
    s->rx_index = 0;
    s->frame_ready = 0;

    /* RESTART RX */
    BSP_RS485_Receive_IT(s->port,
                         &s->rx_byte);
}

/* =========================================================
 * EXCEPTION
 * ========================================================= */

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t func,
                      uint8_t ex)
{
    uint8_t tx[5];

    tx[0] = s->slave_id;
    tx[1] = func | 0x80;
    tx[2] = ex;

    MB_Send(s, tx, 3);
}

/* =========================================================
 * POLL
 * ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    uint32_t now;

    uint8_t func;

    uint16_t addr;
    uint16_t qty;

    uint8_t tx[128];

    now = HAL_GetTick();

    /* no data */
    if(s->rx_index == 0)
        return;

    /* frame timeout reset */
    if((now - s->last_rx_tick) > MB_SLAVE_RX_RESET_MS)
    {
        s->rx_index = 0;
        return;
    }

    /* wait end frame */
    if((now - s->last_rx_tick) < MB_SLAVE_T35_MS)
        return;

    /* LOCK FRAME */
    __disable_irq();

    s->frame_len = s->rx_index;

    memcpy(s->frame_buf,
           s->rx_buf,
           s->frame_len);

    s->rx_index = 0;

    __enable_irq();

    /* minimum frame */
    if(s->frame_len < 8)
        return;

    /* slave id */
    if(s->frame_buf[0] != s->slave_id)
        return;

    /* CRC */
    if(!MB_CRC_Check(s->frame_buf,
                     s->frame_len))
    {
        return;
    }

    func = s->frame_buf[1];

    /* =====================================================
     * READ HOLDING 0x03
     * ===================================================== */
    if(func == 0x03)
    {
        addr = ((uint16_t)s->frame_buf[2] << 8)
             | s->frame_buf[3];

        qty  = ((uint16_t)s->frame_buf[4] << 8)
             | s->frame_buf[5];

        if((qty == 0) ||
           (qty > 32) ||
           ((addr + qty) > MB_SLAVE_MAX_REG))
        {
            MB_Except(s, func, 0x02);
            return;
        }

        tx[0] = s->slave_id;
        tx[1] = 0x03;
        tx[2] = qty * 2;

        for(uint16_t i = 0; i < qty; i++)
        {
            uint16_t v;

            v = s->holding_reg[addr + i];

            tx[3 + i * 2] = v >> 8;
            tx[4 + i * 2] = v & 0xFF;
        }

        MB_Send(s,
                tx,
                3 + qty * 2);

        return;
    }

    /* =====================================================
     * WRITE SINGLE 0x06
     * ===================================================== */
    if(func == 0x06)
    {
        addr = ((uint16_t)s->frame_buf[2] << 8)
             | s->frame_buf[3];

        uint16_t val;

        val = ((uint16_t)s->frame_buf[4] << 8)
            | s->frame_buf[5];

        if(addr >= MB_SLAVE_MAX_REG)
        {
            MB_Except(s, func, 0x02);
            return;
        }

        s->holding_reg[addr] = val;

        memcpy(tx,
               s->frame_buf,
               6);

        /* ACK FIRST */
        MB_Send(s, tx, 6);

        /* CHANGE BAUD AFTER ACK */
        if(addr == 0x0001)
        {
            if(val < MB_BAUD_COUNT)
            {
                HAL_Delay(50);

                BSP_RS485_SetBaudrate(s->port,
                                      MB_BAUD[val]);
            }
        }

        return;
    }

    /* =====================================================
     * WRITE MULTI 0x10
     * ===================================================== */
    if(func == 0x10)
    {
        addr = ((uint16_t)s->frame_buf[2] << 8)
             | s->frame_buf[3];

        qty  = ((uint16_t)s->frame_buf[4] << 8)
             | s->frame_buf[5];

        if((qty == 0) ||
           ((addr + qty) > MB_SLAVE_MAX_REG))
        {
            MB_Except(s, func, 0x02);
            return;
        }

        for(uint16_t i = 0; i < qty; i++)
        {
            s->holding_reg[addr + i] =
                ((uint16_t)s->frame_buf[7 + i * 2] << 8)
              | s->frame_buf[8 + i * 2];
        }

        tx[0] = s->slave_id;
        tx[1] = 0x10;
        tx[2] = s->frame_buf[2];
        tx[3] = s->frame_buf[3];
        tx[4] = s->frame_buf[4];
        tx[5] = s->frame_buf[5];

        /* ACK FIRST */
        MB_Send(s, tx, 6);

        /* CHANGE BAUD AFTER ACK */
        if((addr <= 0x0001) &&
           ((addr + qty) > 0x0001))
        {
            uint16_t b = s->holding_reg[0x0001];

            if(b < MB_BAUD_COUNT)
            {
                HAL_Delay(50);

                BSP_RS485_SetBaudrate(s->port,
                                      MB_BAUD[b]);
            }
        }

        return;
    }

    /* unsupported */
    MB_Except(s,
              func,
              0x01);
}

/* =========================================================
 * API
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s,
                       uint16_t a,
                       float v)
{
    if((a + 1) >= MB_SLAVE_MAX_REG)
        return;

    uint32_t r;

    memcpy(&r, &v, 4);

    s->holding_reg[a]     = r >> 16;
    s->holding_reg[a + 1] = r & 0xFFFF;
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
        ((uint32_t)s->holding_reg[a] << 16)
      | ((uint32_t)s->holding_reg[a + 1]);

    memcpy(&v, &r, 4);

    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s,
                         uint16_t a)
{
    return s->holding_reg[a];
}
