#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_rtu.h"
#include <stdint.h>

#define MB_SLAVE_MAX_REG        32

typedef struct
{
    uint8_t     slave_id;
    RS485_PORT  port;

    uint16_t    holding_reg[MB_SLAVE_MAX_REG];

    uint8_t     rx_buf[MB_RX_BUF_SIZE];
    uint16_t    rx_index;
    uint32_t    last_rx_tick;
    uint8_t     rx_byte;

    uint8_t     frame_ready;
    uint8_t     exception_code;

    /* ===== FIX ADD ===== */
    uint8_t     tx_lock;

} MB_SLAVE_t;

/* INIT */
void MB_SLAVE_Init(MB_SLAVE_t *slave,
                   RS485_PORT port,
                   uint8_t slave_id);

/* PROCESS */
void MB_SLAVE_Poll(MB_SLAVE_t *slave);

/* RX */
void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave,
                            uint8_t byte);

/* API */
void MB_SLAVE_SetFloat(MB_SLAVE_t *slave,
                       uint16_t reg_addr,
                       float value);

void MB_SLAVE_SetU16(MB_SLAVE_t *slave,
                     uint16_t reg_addr,
                     uint16_t value);

float MB_SLAVE_GetFloat(MB_SLAVE_t *slave,
                        uint16_t reg_addr);

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *slave,
                         uint16_t reg_addr);

#endif
