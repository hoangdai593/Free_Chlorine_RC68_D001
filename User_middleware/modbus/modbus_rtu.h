#ifndef __MODBUS_RTU_H
#define __MODBUS_RTU_H

#include "stdint.h"
#include "bsp_rs485.h"

/* ================= CONFIG ================= */
#define MB_RX_BUF_SIZE        256
#define MB_FRAME_TIMEOUT_MS   5   // ~3.5 char time (tuỳ baud)

/* ================= STRUCT ================= */
typedef struct
{
    RS485_PORT port;
    uint8_t slave_id;

    uint8_t  rx_buf[MB_RX_BUF_SIZE];
    uint16_t rx_index;

    uint8_t  rx_byte;        // buffer nhận từng byte (interrupt)

    uint32_t last_rx_tick;   // thời điểm nhận byte cuối

    uint8_t  frame_ready;    // flag báo đã nhận xong frame

} MB_RTU_t;

/* ================= API ================= */
void MB_RTU_Init(MB_RTU_t *mb,
                 RS485_PORT port,
                 uint8_t slave_id);

void MB_RTU_StartReceive(MB_RTU_t *mb);

void MB_RTU_RxByteHandler(MB_RTU_t *mb);

void MB_RTU_Poll(MB_RTU_t *mb);

HAL_StatusTypeDef MB_RTU_Send(MB_RTU_t *mb,
                              uint8_t *buf,
                              uint16_t len);

#endif
