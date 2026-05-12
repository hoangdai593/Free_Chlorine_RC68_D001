#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_rtu.h"
#include <stdint.h>

#define MB_SLAVE_MAX_REG 32

typedef struct
{
    uint8_t     slave_id;
    RS485_PORT  port;

    uint16_t    holding_reg[MB_SLAVE_MAX_REG];

    uint8_t     rx_buf[MB_RX_BUF_SIZE];
    uint16_t    rx_index;
    uint32_t    last_rx_tick;

    uint8_t     rx_byte;

} MB_SLAVE_t;

/* INIT */
void MB_SLAVE_Init(MB_SLAVE_t *slave,
                   RS485_PORT port,
                   uint8_t slave_id);

/* RX BYTE */
void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave,
                            uint8_t byte);

/* POLL */
void MB_SLAVE_Poll(MB_SLAVE_t *slave);

/* API */
void MB_SLAVE_SetFloat(MB_SLAVE_t *slave, uint16_t reg_addr, float value);
float MB_SLAVE_GetFloat(MB_SLAVE_t *slave, uint16_t reg_addr);

void MB_SLAVE_SetU16(MB_SLAVE_t *slave, uint16_t reg_addr, uint16_t value);
uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *slave, uint16_t reg_addr);

#endif
