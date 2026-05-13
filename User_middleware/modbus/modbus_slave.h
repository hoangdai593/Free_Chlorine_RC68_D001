#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_rtu.h"
#include <stdint.h>

/* ================= CONFIG ================= */

#define MB_SLAVE_MAX_REG        32

#ifndef MB_RX_BUF_SIZE
#define MB_RX_BUF_SIZE          256
#endif

/* ================= STRUCT ================= */

typedef struct
{
    uint8_t     slave_id;
    RS485_PORT  port;

    uint16_t    holding_reg[MB_SLAVE_MAX_REG];

    /* RX */
    uint8_t     rx_buf[MB_RX_BUF_SIZE];
    uint16_t    rx_index;
    uint8_t     rx_byte;
    uint32_t    last_rx_tick;

    /* TX */
    uint8_t     tx_buf[MB_RX_BUF_SIZE];

} MB_SLAVE_t;

/* ================= API ================= */

void MB_SLAVE_Init(MB_SLAVE_t *slave,
                   RS485_PORT port,
                   uint8_t slave_id);

void MB_SLAVE_Poll(MB_SLAVE_t *slave);

void MB_SLAVE_RxByteHandler(MB_SLAVE_t *slave,
                            uint8_t byte);

/* register API */
void MB_SLAVE_SetFloat(MB_SLAVE_t *slave,
                       uint16_t addr,
                       float v);

void MB_SLAVE_SetU16(MB_SLAVE_t *slave,
                     uint16_t addr,
                     uint16_t v);

float MB_SLAVE_GetFloat(MB_SLAVE_t *slave,
                        uint16_t addr);

uint16_t MB_SLAVE_GetU16(MB_SLAVE_t *slave,
                         uint16_t addr);

#endif
