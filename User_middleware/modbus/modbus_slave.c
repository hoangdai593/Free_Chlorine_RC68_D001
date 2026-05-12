#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

/* ================= BAUD TABLE (GIỮ NGUYÊN 100%) ================= */
#define MB_SLAVE_BAUD_CODE_COUNT 11
static const uint32_t MB_SLAVE_BAUD_CODE[MB_SLAVE_BAUD_CODE_COUNT] =
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

/* =========================================================
 * PRIVATE
 * ========================================================= */

static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave,
                                  uint16_t len);

static void MB_SLAVE_SendException(MB_SLAVE_t *slave,
                                   uint8_t exception);

static void MB_SLAVE_UpdateBaudFromHoldingReg(MB_SLAVE_t *slave)
{
    uint16_t baud_code = slave->holding_reg[0x0001];
    if(baud_code >= MB_SLAVE_BAUD_CODE_COUNT)
        return;

    BSP_RS485_SetBaudrate(slave->port,
                          MB_SLAVE_BAUD_CODE[baud_code]);
}

/* =========================================================
 * INIT
 * ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *slave,
                   RS485_PORT port,
                   uint8_t slave_id)
{
    memset(slave, 0, sizeof(MB_SLAVE_t));

    slave->port     = port;
    slave->slave_id = slave_id;

    slave->rx_index     = 0;
    slave->last_rx_tick = HAL_GetTick();

    /* DEFAULT REG */
    slave->holding_reg[0x0000] = slave_id;
    slave->holding_reg[0x0001] = 3;

    MB_SLAVE_SetFloat(slave, 0x0006, 7.5f);
    MB_SLAVE_SetU16(slave, 0x000D, 0);
    MB_SLAVE_SetFloat(slave, 0x000E, 1.5f);
    MB_SLAVE_SetFloat(slave, 0x0010, 0.1f);

    BSP_RS485_RX_Mode(port);
    BSP_RS485_Receive_IT(port, &slave->rx_byte);
}

/* =========================================================
 * RX BYTE (FIX NOISE FILTER)
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave,
                            uint8_t byte)
{
    uint32_t now = HAL_GetTick();

    /* ===== chống nhiễu khoảng cách frame ===== */
    if((now - slave->last_rx_tick) > MB_INTERFRAME_GAP_MS)
    {
        /* reset frame mới nếu khoảng cách lớn */
        slave->rx_index = 0;
    }

    if(slave->rx_index >= MB_RX_BUF_SIZE)
    {
        slave->rx_index = 0;
    }

    slave->rx_buf[slave->rx_index++] = byte;
    slave->last_rx_tick = now;

    BSP_RS485_Receive_IT(slave->port, &slave->rx_byte);
}

/* =========================================================
 * POLL (FIX TIMEOUT ROBUST)
 * ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *slave)
{
    uint32_t now = HAL_GetTick();

    /* ===== không đủ data ===== */
    if(slave->rx_index < 4)
        return;

    /* ===== CHỐNG FRAME RÁC: phải im lặng đủ lâu mới xử lý ===== */
    if((now - slave->last_rx_tick) < MB_FRAME_TIMEOUT_MS)
        return;

    /* ===== CRC FAIL -> DROP FRAME ===== */
    if(!MB_CRC_Check(slave->rx_buf, slave->rx_index))
    {
        slave->rx_index = 0;
        return;
    }

    /* ===== WRONG ID ===== */
    if(slave->rx_buf[0] != slave->slave_id)
    {
        slave->rx_index = 0;
        return;
    }

    uint8_t func = slave->rx_buf[1];

    switch(func)
    {
        case 0x03:
        {
            uint16_t addr = (slave->rx_buf[2] << 8) | slave->rx_buf[3];
            uint16_t qty  = (slave->rx_buf[4] << 8) | slave->rx_buf[5];

            if((addr + qty) > MB_SLAVE_MAX_REG || qty == 0 || qty > 125)
            {
                MB_SLAVE_SendException(slave, 0x02);
                break;
            }

            slave->rx_buf[1] = 0x03;
            slave->rx_buf[2] = qty * 2;

            for(uint16_t i = 0; i < qty; i++)
            {
                uint16_t val = slave->holding_reg[addr + i];
                slave->rx_buf[3 + i*2] = val >> 8;
                slave->rx_buf[4 + i*2] = val & 0xFF;
            }

            MB_SLAVE_SendResponse(slave, 3 + qty * 2);
            break;
        }

        case 0x06:
        {
            uint16_t addr  = (slave->rx_buf[2] << 8) | slave->rx_buf[3];
            uint16_t value = (slave->rx_buf[4] << 8) | slave->rx_buf[5];

            if(addr >= MB_SLAVE_MAX_REG)
            {
                MB_SLAVE_SendException(slave, 0x02);
                break;
            }

            slave->holding_reg[addr] = value;

            if(addr == 0x0000)
                slave->slave_id = value;

            if(addr == 0x0001)
                MB_SLAVE_UpdateBaudFromHoldingReg(slave);

            MB_SLAVE_SendResponse(slave, 6);
            break;
        }

        case 0x10:
        {
            uint16_t addr = (slave->rx_buf[2] << 8) | slave->rx_buf[3];
            uint16_t qty  = (slave->rx_buf[4] << 8) | slave->rx_buf[5];
            uint8_t bc    = slave->rx_buf[6];

            if((addr + qty) > MB_SLAVE_MAX_REG)
            {
                MB_SLAVE_SendException(slave, 0x02);
                break;
            }

            if(bc != qty * 2)
            {
                MB_SLAVE_SendException(slave, 0x03);
                break;
            }

            for(uint16_t i = 0; i < qty; i++)
            {
                slave->holding_reg[addr + i] =
                    (slave->rx_buf[7 + i*2] << 8) |
                     slave->rx_buf[8 + i*2];
            }

            if(addr == 0x0000)
                slave->slave_id = slave->holding_reg[0x0000];

            if((addr <= 0x0001) && (0x0001 < addr + qty))
                MB_SLAVE_UpdateBaudFromHoldingReg(slave);

            MB_SLAVE_SendResponse(slave, 6);
            break;
        }

        default:
            MB_SLAVE_SendException(slave, 0x01);
            break;
    }

    /* ===== RESET FRAME AFTER DONE ===== */
    slave->rx_index = 0;
}

/* =========================================================
 * TX RESPONSE
 * ========================================================= */

static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave,
                                  uint16_t len)
{
    BSP_RS485_TX_Mode(slave->port);
    delay_us(8);

    MB_CRC_Append(slave->rx_buf, len);

    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(slave->port);

    if(huart)
    {
        HAL_UART_Transmit(huart,
                          slave->rx_buf,
                          len + 2,
                          100);
    }

    BSP_RS485_WaitTC(slave->port);

    delay_us(8);
    BSP_RS485_RX_Mode(slave->port);

    BSP_RS485_Receive_IT(slave->port, &slave->rx_byte);
}

/* =========================================================
 * EXCEPTION
 * ========================================================= */

static void MB_SLAVE_SendException(MB_SLAVE_t *slave,
                                   uint8_t exception)
{
    slave->rx_buf[1] |= 0x80;
    slave->rx_buf[2] = exception;

    MB_SLAVE_SendResponse(slave, 3);
}

/* =========================================================
 * API (GIỮ NGUYÊN)
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *slave,
                       uint16_t reg_addr,
                       float value)
{
    if((reg_addr + 1) >= MB_SLAVE_MAX_REG)
        return;

    uint32_t raw = *((uint32_t*)&value);

    slave->holding_reg[reg_addr] = raw >> 16;
    slave->holding_reg[reg_addr + 1] = raw & 0xFFFF;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *slave,
                     uint16_t reg_addr,
                     uint16_t value)
{
    if(reg_addr >= MB_SLAVE_MAX_REG)
        return;

    slave->holding_reg[reg_addr] = value;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *slave,
                        uint16_t reg_addr)
{
    if((reg_addr + 1) >= MB_SLAVE_MAX_REG)
        return 0;

    uint32_t raw =
        ((uint32_t)slave->holding_reg[reg_addr] << 16) |
        slave->holding_reg[reg_addr + 1];

    return *((float*)&raw);
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *slave,
                         uint16_t reg_addr)
{
    if(reg_addr >= MB_SLAVE_MAX_REG)
        return 0;

    return slave->holding_reg[reg_addr];
}
