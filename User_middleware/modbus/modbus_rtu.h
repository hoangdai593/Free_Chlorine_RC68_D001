#ifndef __MODBUS_RTU_H
#define __MODBUS_RTU_H

#include "stdint.h"
#include "bsp_rs485.h"

#define MB_RX_BUF_SIZE          256
#define MB_FRAME_TIMEOUT_MS     5

typedef enum
{
    MB_RTU_OK = 0,
    MB_RTU_BUSY,
    MB_RTU_TIMEOUT,
    MB_RTU_CRC_ERROR,
    MB_RTU_OVERFLOW
} MB_RTU_STATUS;

typedef struct
{
    RS485_PORT port;
    uint8_t    slave_id;

    uint8_t    rx_buf[MB_RX_BUF_SIZE];
    volatile uint16_t rx_index;

    uint8_t    rx_byte;
    uint32_t   last_rx_tick;

    volatile uint8_t frame_ready;
    MB_RTU_STATUS status;

} MB_RTU_t;

void MB_RTU_Init(MB_RTU_t *mb, RS485_PORT port, uint8_t slave_id);

void MB_RTU_StartReceive(MB_RTU_t *mb);

void MB_RTU_RxByteHandler(MB_RTU_t *mb);

void MB_RTU_Poll(MB_RTU_t *mb);

void MB_RTU_Clear(MB_RTU_t *mb);

HAL_StatusTypeDef MB_RTU_Send(MB_RTU_t *mb, uint8_t *buf, uint16_t len);

#endif
