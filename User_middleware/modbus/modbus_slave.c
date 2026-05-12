#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"

#include <string.h>

/* =========================================================
 * PRIVATE
 * ========================================================= */

static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave,
                                  uint16_t len);

static void MB_SLAVE_SendException(MB_SLAVE_t *slave,
                                   uint8_t exception);

/* =========================================================
 * INIT
 * ========================================================= */

void MB_SLAVE_Init(MB_SLAVE_t *slave,
                   RS485_PORT port,
                   uint8_t slave_id)
{
    slave->port = port;
    slave->slave_id = slave_id;

    slave->rx_index = 0;
    slave->last_rx_tick = 0;

    slave->frame_ready = 0;
    slave->exception_code = 0;

    slave->tx_lock = 0;

    memset(slave->rx_buf, 0, MB_RX_BUF_SIZE);
    memset(slave->holding_reg, 0, sizeof(slave->holding_reg));

    /* default */
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
 * RX BYTE (FIX: KHÔNG GHI KHI TX + CHỐNG RACE)
 * ========================================================= */

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave,
                            uint8_t byte)
{
    /* nếu đang TX thì bỏ RX (TRÁNH NHIỄU BUS) */
    if(slave->tx_lock)
    {
        BSP_RS485_Receive_IT(slave->port, &slave->rx_byte);
        return;
    }

    if(slave->rx_index >= MB_RX_BUF_SIZE)
        slave->rx_index = 0;

    slave->rx_buf[slave->rx_index++] = byte;
    slave->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(slave->port, &slave->rx_byte);
}

/* =========================================================
 * POLL
 * ========================================================= */

void MB_SLAVE_Poll(MB_SLAVE_t *slave)
{
    if(slave->rx_index < 4)
        return;

    /* FIX: timeout frame chuẩn */
    if((HAL_GetTick() - slave->last_rx_tick) < 5)
        return;

    /* check slave id */
    if(slave->rx_buf[0] != slave->slave_id)
    {
        slave->rx_index = 0;
        return;
    }

    /* CRC check */
    if(!MB_CRC_Check(slave->rx_buf, slave->rx_index))
    {
        slave->rx_index = 0;
        return;
    }

    uint8_t func = slave->rx_buf[1];

    switch(func)
    {
        case 0x03:
        {
            uint16_t addr = (slave->rx_buf[2]<<8)|slave->rx_buf[3];
            uint16_t qty  = (slave->rx_buf[4]<<8)|slave->rx_buf[5];

            slave->rx_buf[1] = 0x03;
            slave->rx_buf[2] = qty * 2;

            for(int i=0;i<qty;i++)
            {
                uint16_t v = slave->holding_reg[addr+i];
                slave->rx_buf[3+i*2] = v >> 8;
                slave->rx_buf[4+i*2] = v & 0xFF;
            }

            MB_SLAVE_SendResponse(slave, 3 + qty*2);
            break;
        }

        case 0x06:
        {
            uint16_t addr = (slave->rx_buf[2]<<8)|slave->rx_buf[3];
            uint16_t val  = (slave->rx_buf[4]<<8)|slave->rx_buf[5];

            slave->holding_reg[addr] = val;

            MB_SLAVE_SendResponse(slave, 6);
            break;
        }

        case 0x10:
        {
            uint16_t addr = (slave->rx_buf[2]<<8)|slave->rx_buf[3];
            uint16_t qty  = (slave->rx_buf[4]<<8)|slave->rx_buf[5];

            for(int i=0;i<qty;i++)
            {
                slave->holding_reg[addr+i] =
                    (slave->rx_buf[7+i*2]<<8) |
                     slave->rx_buf[8+i*2];
            }

            MB_SLAVE_SendResponse(slave, 6);
            break;
        }

        default:
            MB_SLAVE_SendException(slave, 0x01);
            break;
    }

    slave->rx_index = 0;
}

/* =========================================================
 * TX RESPONSE (FIX CHÍNH: RS485 SAFE TURNAROUND)
 * ========================================================= */

static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave,
                                  uint16_t len)
{
    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(slave->port);
    if(!huart) return;

    slave->tx_lock = 1;

    HAL_UART_AbortReceive(huart);

    MB_CRC_Append(slave->rx_buf, len);

    BSP_RS485_TX_Mode(slave->port);

    delay_us(20);   // FIX timing

    HAL_UART_Transmit(huart,
                      slave->rx_buf,
                      len + 2,
                      200);

    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

    delay_us(20);

    BSP_RS485_RX_Mode(slave->port);

    slave->rx_index = 0;
    slave->tx_lock = 0;

    BSP_RS485_Receive_IT(slave->port,
                         &slave->rx_byte);
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
 * FLOAT / U16 (GIỮ NGUYÊN)
 * ========================================================= */

void MB_SLAVE_SetFloat(MB_SLAVE_t *slave,
                       uint16_t reg,
                       float v)
{
    uint32_t r = *((uint32_t*)&v);
    slave->holding_reg[reg]   = r >> 16;
    slave->holding_reg[reg+1] = r & 0xFFFF;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *slave,
                     uint16_t reg,
                     uint16_t v)
{
    slave->holding_reg[reg] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *slave,
                        uint16_t reg)
{
    uint32_t r =
        ((uint32_t)slave->holding_reg[reg]<<16) |
        slave->holding_reg[reg+1];

    return *((float*)&r);
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *slave,
                         uint16_t reg)
{
    return slave->holding_reg[reg];
}
