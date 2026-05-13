#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"
#include <string.h>

/* ================= BAUD TABLE ================= */

#define MB_BAUD_COUNT 11

static const uint32_t MB_BAUD[MB_BAUD_COUNT] =
{
    1200,2400,4800,9600,14400,
    19200,28800,38400,56000,57600,115200
};

/* ================= TIMING ================= */

#define T35_MS         4
#define RX_RESET_MS    50

volatile uint8_t frame_ready = 0;

/* ================= INTERNAL ================= */

static void MB_Send(MB_SLAVE_t *s, uint16_t len);
static void MB_Except(MB_SLAVE_t *s, uint8_t ex);

/* =========================================================
 * BAUD UPDATE
 * ========================================================= */

static void MB_UpdateBaud(MB_SLAVE_t *s)
{
    uint16_t b = s->holding_reg[0x0001];

    if(b >= MB_BAUD_COUNT)
        return;

    BSP_RS485_SetBaudrate(s->port, MB_BAUD[b]);
}

/* =========================================================
 * INIT
 * ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *s, RS485_PORT port, uint8_t id)
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
    BSP_RS485_Receive_IT(port, &s->rx_byte);
}

/* =========================================================
 * RX BYTE
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s, uint8_t b)
{
    if(s->rx_index < MB_RX_BUF_SIZE)
    {
        s->rx_buf[s->rx_index++] = b;
    }
    else
    {
        s->rx_index = MB_RX_BUF_SIZE - 1;
    }

    s->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(s->port, &s->rx_byte);
}

/* =========================================================
 * POLL
 * ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    uint32_t now = HAL_GetTick();

    if(s->rx_index == 0)
        return;

    /* ===== timeout ===== */

    if(now - s->last_rx_tick > RX_RESET_MS)
    {
        s->rx_index = 0;
        s->frame_ready = 0;
        return;
    }

    /* ===== wait T3.5 ===== */

    if(now - s->last_rx_tick < T35_MS)
        return;

    s->frame_ready = 1;

    if(!s->frame_ready)
        return;

    s->frame_ready = 0;

    /* ===== validate ===== */

    if(s->rx_buf[0] != s->slave_id)
        goto reset;

    if(!MB_CRC_Check(s->rx_buf, s->rx_index))
        goto reset;

    uint8_t f = s->rx_buf[1];

    /* =====================================================
     * READ HOLDING REGISTER
     * ===================================================== */

    if(f == 0x03)
    {
        uint16_t addr =
                (s->rx_buf[2] << 8) |
                 s->rx_buf[3];

        uint16_t qty =
                (s->rx_buf[4] << 8) |
                 s->rx_buf[5];

        if(addr + qty > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        s->rx_buf[1] = 0x03;
        s->rx_buf[2] = qty * 2;

        for(int i = 0; i < qty; i++)
        {
            uint16_t reg_index = addr + i;
            uint16_t v;

            /* =================================================
             * CDAB FLOAT OUTPUT
             * swap từng cặp register khi TX
             * ================================================= */

            if((reg_index & 0x01) == 0)
            {
                v = s->holding_reg[reg_index + 1];
            }
            else
            {
                v = s->holding_reg[reg_index - 1];
            }

            s->rx_buf[3 + i * 2] = (v >> 8) & 0xFF;
            s->rx_buf[4 + i * 2] = v & 0xFF;
        }

        MB_Send(s, 3 + qty * 2);
    }

    /* =====================================================
     * WRITE SINGLE REGISTER
     * ===================================================== */

    else if(f == 0x06)
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

        if(addr == 0x0001)
            MB_UpdateBaud(s);

        MB_Send(s, 6);
    }

    /* =====================================================
     * WRITE MULTI REGISTER
     * ===================================================== */

    else if(f == 0x10)
    {
        uint16_t addr =
                (s->rx_buf[2] << 8) |
                 s->rx_buf[3];

        uint16_t qty =
                (s->rx_buf[4] << 8) |
                 s->rx_buf[5];

        uint8_t byte_count = s->rx_buf[6];

        /* ===== range check ===== */

        if(addr + qty > MB_SLAVE_MAX_REG)
        {
            MB_Except(s, 0x02);
            goto reset;
        }

        /* ===== byte count check ===== */

        if(byte_count != qty * 2)
        {
            MB_Except(s, 0x03);
            goto reset;
        }

        /* ===== frame length ===== */

        if(s->rx_index < (9 + byte_count))
        {
            goto reset;
        }

        for(int i = 0; i < qty; i++)
        {
            s->holding_reg[addr + i] =
                    (s->rx_buf[7 + i * 2] << 8) |
                     s->rx_buf[8 + i * 2];
        }

        if(addr <= 0x0001 &&
           (addr + qty) > 0x0001)
        {
            MB_UpdateBaud(s);
        }

        MB_Send(s, 6);
    }

    /* =====================================================
     * UNSUPPORTED FUNCTION
     * ===================================================== */

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

static void MB_Send(MB_SLAVE_t *s, uint16_t len)
{
    BSP_RS485_TX_Mode(s->port);

    delay_us(20);

    MB_CRC_Append(s->rx_buf, len);

    UART_HandleTypeDef *huart =
            BSP_RS485_GetHandle(s->port);

    if(huart)
    {
        HAL_UART_Transmit(
                huart,
                s->rx_buf,
                len + 2,
                200);
    }

    BSP_RS485_WaitTC(s->port);

    delay_us(20);

    BSP_RS485_RX_Mode(s->port);

    BSP_RS485_Receive_IT(
            s->port,
            &s->rx_byte);
}

/* =========================================================
 * EXCEPTION
 * ========================================================= */

static void MB_Except(MB_SLAVE_t *s, uint8_t ex)
{
    s->rx_buf[1] |= 0x80;
    s->rx_buf[2] = ex;

    MB_Send(s, 3);
}

/* =========================================================
 * API
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *s, uint16_t a, float v)
{
    if(a + 1 >= MB_SLAVE_MAX_REG)
        return;

    uint32_t r;

    memcpy(&r, &v, 4);

    /* ABCD internal storage */

    s->holding_reg[a] =
            (r >> 16) & 0xFFFF;

    s->holding_reg[a + 1] =
            r & 0xFFFF;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s, uint16_t a, uint16_t v)
{
    if(a >= MB_SLAVE_MAX_REG)
        return;

    s->holding_reg[a] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s, uint16_t a)
{
    uint32_t r =
            ((uint32_t)s->holding_reg[a] << 16) |
             s->holding_reg[a + 1];

    float v;

    memcpy(&v, &r, 4);

    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s, uint16_t a)
{
    return s->holding_reg[a];
}
