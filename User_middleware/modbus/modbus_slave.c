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
 * BAUD UPDATE
 * ========================================================= */

static void MB_UpdateBaud(MB_SLAVE_t *s)
{
    uint16_t b;

    b = s->holding_reg[0x0001];

    if(b >= MB_BAUD_COUNT)
        return;

    BSP_RS485_SetBaudrate(s->port,
                          MB_BAUD[b]);
}

/* =========================================================
 * INIT
 * ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *s,
                   RS485_PORT port,
                   uint8_t id)
{
    memset(s, 0, sizeof(MB_SLAVE_t));

    s->port = port;
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

/* =========================================================
 * RX BYTE
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s,
                            uint8_t b)
{
    /* đang TX thì bỏ qua echo */
    if(s->tx_busy)
    {
        BSP_RS485_Receive_IT(s->port,
                             &s->rx_byte);
        return;
    }

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

    s->tx_busy = 1;

    MB_CRC_Append(buf, len);

    BSP_RS485_TX_Mode(s->port);

    delay_us(50);

    HAL_UART_Transmit(huart,
                      buf,
                      len + 2,
                      200);

    BSP_RS485_WaitTC(s->port);

    delay_us(50);

    BSP_RS485_RX_Mode(s->port);

    s->rx_index = 0;
    s->frame_ready = 0;
    s->tx_busy = 0;

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
    s->tx_buf[0] = s->slave_id;
    s->tx_buf[1] = func | 0x80;
    s->tx_buf[2] = ex;

    MB_Send(s,
            s->tx_buf,
            3);
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

    now = HAL_GetTick();

    if(s->rx_index < 8)
        return;

    /* frame timeout */
    if((now - s->last_rx_tick)
        > MB_SLAVE_RX_RESET_MS)
    {
        s->rx_index = 0;
        s->frame_ready = 0;
        return;
    }

    /* chưa hết frame */
    if((now - s->last_rx_tick)
        < MB_SLAVE_T35_MS)
    {
        return;
    }

    s->frame_ready = 1;

    if(s->frame_ready == 0)
        return;

    s->frame_ready = 0;

    /* check slave id */
    if(s->rx_buf[0] != s->slave_id)
        goto reset;

    /* CRC */
    if(MB_CRC_Check(s->rx_buf,
                    s->rx_index) == 0)
    {
        goto reset;
    }

    func = s->rx_buf[1];

    /* =====================================================
     * READ HOLDING
     * ===================================================== */

    if(func == 0x03)
    {
        addr =
            ((uint16_t)s->rx_buf[2] << 8) |
             s->rx_buf[3];

        qty =
            ((uint16_t)s->rx_buf[4] << 8) |
             s->rx_buf[5];

        if((qty == 0) ||
           ((addr + qty) > MB_SLAVE_MAX_REG))
        {
            MB_Except(s, func, 0x02);
            goto reset;
        }

        s->tx_buf[0] = s->slave_id;
        s->tx_buf[1] = 0x03;
        s->tx_buf[2] = qty * 2;

        for(uint16_t i = 0; i < qty; i++)
        {
            uint16_t v;

            v = s->holding_reg[addr + i];

            s->tx_buf[3 + i * 2] = v >> 8;
            s->tx_buf[4 + i * 2] = v & 0xFF;
        }

        MB_Send(s,
                s->tx_buf,
                3 + qty * 2);

        goto reset;
    }

    /* =====================================================
     * WRITE SINGLE
     * ===================================================== */

    else if(func == 0x06)
    {
        uint16_t val;
        uint8_t change_baud = 0;

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

        if(addr == 0x0001)
            change_baud = 1;

        MB_Send(s,
                s->tx_buf,
                6);

        if(change_baud)
        {
            HAL_Delay(20);
            MB_UpdateBaud(s);
        }

        goto reset;
    }

    /* =====================================================
     * WRITE MULTI
     * ===================================================== */

    else if(func == 0x10)
    {
        uint8_t byte_count;
        uint8_t change_baud = 0;

        addr =
            ((uint16_t)s->rx_buf[2] << 8) |
             s->rx_buf[3];

        qty =
            ((uint16_t)s->rx_buf[4] << 8) |
             s->rx_buf[5];

        byte_count = s->rx_buf[6];

        if((qty == 0) ||
           ((addr + qty) > MB_SLAVE_MAX_REG) ||
           (byte_count != qty * 2))
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

        if((addr <= 0x0001) &&
           ((addr + qty) > 0x0001))
        {
            change_baud = 1;
        }

        MB_Send(s,
                s->tx_buf,
                6);

        if(change_baud)
        {
            HAL_Delay(20);
            MB_UpdateBaud(s);
        }

        goto reset;
    }

    /* unsupported */
    MB_Except(s,
              func,
              0x01);

reset:

    s->rx_index = 0;
}

/* =========================================================
 * REGISTER API
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s,
                       uint16_t a,
                       float v)
{
    uint32_t r;

    if((a + 1) >= MB_SLAVE_MAX_REG)
        return;

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
