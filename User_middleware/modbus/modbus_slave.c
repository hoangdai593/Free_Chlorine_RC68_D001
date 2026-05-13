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

void MB_SLAVE_Init(MB_SLAVE_t *s,
                   RS485_PORT port,
                   uint8_t id)
{
    UART_HandleTypeDef *huart;

    memset(s,0,sizeof(MB_SLAVE_t));

    s->port = port;
    s->slave_id = id;

    s->holding_reg[0x0000] = id;
    s->holding_reg[0x0001] = 3;

    huart = BSP_RS485_GetHandle(port);

    BSP_RS485_RX_Mode(port);

    BSP_RS485_Receive_IT(port,
                         &s->rx_byte);

    __HAL_UART_ENABLE_IT(huart,
                         UART_IT_IDLE);
}

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

    BSP_RS485_Receive_IT(s->port,
                         &s->rx_byte);
}

void MB_SLAVE_IdleHandler(MB_SLAVE_t *s)
{
    if(s->rx_index >= 4)
    {
        s->frame_ready = 1;
    }
}

static void MB_Send(MB_SLAVE_t *s,
                    uint8_t *buf,
                    uint16_t len)
{
    UART_HandleTypeDef *huart;

    huart = BSP_RS485_GetHandle(s->port);

    if(huart == NULL)
        return;

    MB_CRC_Append(buf,len);

    BSP_RS485_TX_Mode(s->port);

    delay_us(20);

    HAL_UART_Transmit(huart,
                      buf,
                      len + 2,
                      200);

    BSP_RS485_WaitTC(s->port);

    delay_us(20);

    BSP_RS485_RX_Mode(s->port);

    s->rx_index = 0;
    s->frame_ready = 0;

    BSP_RS485_Receive_IT(s->port,
                         &s->rx_byte);
}

static void MB_Except(MB_SLAVE_t *s,
                      uint8_t func,
                      uint8_t ex)
{
    uint8_t tx[5];

    tx[0] = s->slave_id;
    tx[1] = func | 0x80;
    tx[2] = ex;

    MB_Send(s,tx,3);
}

void MB_SLAVE_Poll(MB_SLAVE_t *s)
{
    uint8_t func;

    uint16_t addr;
    uint16_t qty;

    uint8_t tx[128];

    if(s->frame_ready == 0)
        return;

    s->frame_ready = 0;

    if(s->rx_index < 8)
    {
        s->rx_index = 0;
        return;
    }

    if(s->rx_buf[0] != s->slave_id)
    {
        s->rx_index = 0;
        return;
    }

    if(MB_CRC_Check(s->rx_buf,
                    s->rx_index) == 0)
    {
        s->rx_index = 0;
        return;
    }

    func = s->rx_buf[1];

    if(func == 0x03)
    {
        addr = (s->rx_buf[2]<<8) | s->rx_buf[3];
        qty  = (s->rx_buf[4]<<8) | s->rx_buf[5];

        if((qty == 0) ||
           ((addr + qty) > MB_SLAVE_MAX_REG))
        {
            MB_Except(s,func,0x02);
            s->rx_index = 0;
            return;
        }

        tx[0] = s->slave_id;
        tx[1] = 0x03;
        tx[2] = qty * 2;

        for(uint16_t i=0;i<qty;i++)
        {
            uint16_t v = s->holding_reg[addr+i];

            tx[3+i*2] = v >> 8;
            tx[4+i*2] = v;
        }

        MB_Send(s,tx,3 + qty*2);
    }
    else if(func == 0x06)
    {
        addr = (s->rx_buf[2]<<8) | s->rx_buf[3];

        uint16_t val =
                (s->rx_buf[4]<<8) |
                 s->rx_buf[5];

        if(addr >= MB_SLAVE_MAX_REG)
        {
            MB_Except(s,func,0x02);
            s->rx_index = 0;
            return;
        }

        s->holding_reg[addr] = val;

        memcpy(tx,s->rx_buf,6);

        MB_Send(s,tx,6);

        if(addr == 0x0001)
        {
            if(val < MB_BAUD_COUNT)
            {
                HAL_Delay(50);

                BSP_RS485_SetBaudrate(s->port,
                                      MB_BAUD[val]);
            }
        }
    }
    else if(func == 0x10)
    {
        addr = (s->rx_buf[2]<<8) | s->rx_buf[3];
        qty  = (s->rx_buf[4]<<8) | s->rx_buf[5];

        if((qty == 0) ||
           ((addr + qty) > MB_SLAVE_MAX_REG))
        {
            MB_Except(s,func,0x02);
            s->rx_index = 0;
            return;
        }

        for(uint16_t i=0;i<qty;i++)
        {
            s->holding_reg[addr+i] =
                (s->rx_buf[7+i*2]<<8) |
                 s->rx_buf[8+i*2];
        }

        tx[0] = s->slave_id;
        tx[1] = 0x10;
        tx[2] = s->rx_buf[2];
        tx[3] = s->rx_buf[3];
        tx[4] = s->rx_buf[4];
        tx[5] = s->rx_buf[5];

        MB_Send(s,tx,6);
    }
    else
    {
        MB_Except(s,func,0x01);
    }

    s->rx_index = 0;
}

void MB_SLAVE_SetFloat(MB_SLAVE_t *s,
                       uint16_t a,
                       float v)
{
    uint32_t r;

    memcpy(&r,&v,4);

    s->holding_reg[a] = r >> 16;
    s->holding_reg[a+1] = r;
}

void MB_SLAVE_SetU16(MB_SLAVE_t *s,
                     uint16_t a,
                     uint16_t v)
{
    s->holding_reg[a] = v;
}

float MB_SLAVE_GetFloat(MB_SLAVE_t *s,
                        uint16_t a)
{
    uint32_t r;

    float v;

    r =
        ((uint32_t)s->holding_reg[a]<<16) |
         s->holding_reg[a+1];

    memcpy(&v,&r,4);

    return v;
}

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s,
                         uint16_t a)
{
    return s->holding_reg[a];
}
