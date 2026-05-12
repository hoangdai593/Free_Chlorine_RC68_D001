#include "modbus_slave.h"
#include "modbus_crc.h"
#include "bsp_rs485.h"
#include "delay.h"
#include <string.h>

/* Private */
static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave, uint16_t len);
static void MB_SLAVE_SendException(MB_SLAVE_t *slave, uint8_t exception);

/* ====================== INIT ====================== */
void MB_SLAVE_Init(MB_SLAVE_t *slave, RS485_PORT port, uint8_t slave_id)
{
    slave->port = port;
    slave->slave_id = slave_id;
    slave->rx_index = 0;
    slave->last_rx_tick = 0;
    slave->frame_ready = 0;
    slave->exception_code = 0;

    memset(slave->rx_buf, 0, MB_RX_BUF_SIZE);
    memset(slave->holding_reg, 0, sizeof(slave->holding_reg));

    slave->holding_reg[0x0000] = slave_id;     // Address
    slave->holding_reg[0x0001] = 3;            // Baudrate code 9600

    BSP_RS485_RX_Mode(port);
    BSP_RS485_Receive_IT(port, &slave->rx_buf[0]);
}

/* ====================== RX BYTE ====================== */
void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave, uint8_t byte)
{
    if(slave->rx_index >= MB_RX_BUF_SIZE)
    {
        slave->rx_index = 0;
    }

    slave->rx_buf[slave->rx_index++] = byte;

    slave->last_rx_tick = HAL_GetTick();

    BSP_RS485_Receive_IT(slave->port,
                         &slave->rx_byte);
}

/* ====================== POLL ====================== */
void MB_SLAVE_Poll(MB_SLAVE_t *slave)
{
    if(slave->rx_index < 4) return;
    if((HAL_GetTick() - slave->last_rx_tick) < MB_FRAME_TIMEOUT_MS) return;

    if(slave->rx_buf[0] != slave->slave_id)
    {
        slave->rx_index = 0;
        return;
    }

    if(!MB_CRC_Check(slave->rx_buf, slave->rx_index))
    {
        slave->rx_index = 0;
        return;
    }

    uint8_t func = slave->rx_buf[1];

    switch(func)
    {
        case 0x03:  // Read Holding Registers
        {
            uint16_t addr = (slave->rx_buf[2] << 8) | slave->rx_buf[3];
            uint16_t qty  = (slave->rx_buf[4] << 8) | slave->rx_buf[5];

            if(addr + qty > MB_SLAVE_MAX_REG || qty > 125 || qty == 0)
            {
                MB_SLAVE_SendException(slave, 0x02);
                break;
            }

            slave->rx_buf[1] = 0x03;
            slave->rx_buf[2] = qty * 2;

            for(uint16_t i = 0; i < qty; i++)
            {
                uint16_t val = slave->holding_reg[addr + i];
                slave->rx_buf[3 + i*2]     = val >> 8;
                slave->rx_buf[3 + i*2 + 1] = val & 0xFF;
            }

            MB_SLAVE_SendResponse(slave, 3 + qty*2);
            break;
        }

        default:  // Hiện tại chỉ hỗ trợ đọc
            MB_SLAVE_SendException(slave, 0x01); // Illegal Function
            break;
    }

    slave->rx_index = 0;
}

/* ====================== SEND RESPONSE ====================== */
static void MB_SLAVE_SendResponse(MB_SLAVE_t *slave, uint16_t len)
{
    BSP_RS485_TX_Mode(slave->port);
    delay_us(8);

    // Sử dụng hàm CRC của bạn
    MB_CRC_Append(slave->rx_buf, len);        // ← Cách tốt nhất

    UART_HandleTypeDef *huart = BSP_RS485_GetHandle(slave->port);
    if(huart != NULL)
    {
        HAL_UART_Transmit(huart, slave->rx_buf, len + 2, 100);
    }

    BSP_RS485_WaitTC(slave->port);
    delay_us(8);
    BSP_RS485_RX_Mode(slave->port);
}

/* ====================== EXCEPTION ====================== */
static void MB_SLAVE_SendException(MB_SLAVE_t *slave, uint8_t exception)
{
    slave->rx_buf[1] |= 0x80;
    slave->rx_buf[2] = exception;
    MB_SLAVE_SendResponse(slave, 3);
}

/* ====================== USER API ====================== */
void MB_SLAVE_SetFloat(MB_SLAVE_t *slave, uint16_t reg_addr, float value)
{
    if(reg_addr + 1 >= MB_SLAVE_MAX_REG) return;

    uint32_t *p = (uint32_t*)&value;
    slave->holding_reg[reg_addr]     = *p & 0xFFFF;      // CDAB format
    slave->holding_reg[reg_addr + 1] = *p >> 16;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *slave, uint16_t reg_addr, uint16_t value)
{
    if(reg_addr < MB_SLAVE_MAX_REG)
        slave->holding_reg[reg_addr] = value;
}
