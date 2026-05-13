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
    volatile uint16_t rx_index;
    volatile uint32_t last_rx_tick;

    uint8_t     rx_byte;

    volatile uint8_t frame_ready;
    volatile uint8_t rx_lock;

    uint8_t     exception_code;

} MB_SLAVE_t;

void MB_SLAVE_Init(MB_SLAVE_t *s, RS485_PORT port, uint8_t id);

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *s, uint8_t b);

void MB_SLAVE_Poll(MB_SLAVE_t *s);

void MB_SLAVE_SetFloat(MB_SLAVE_t *s, uint16_t a, float v);
void MB_SLAVE_SetU16(MB_SLAVE_t *s, uint16_t a, uint16_t v);
float MB_SLAVE_GetFloat(MB_SLAVE_t *s, uint16_t a);
uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *s, uint16_t a);

#endif
