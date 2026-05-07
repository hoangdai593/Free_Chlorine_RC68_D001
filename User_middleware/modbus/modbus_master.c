#include "modbus_master.h"
#include "modbus_crc.h"
#include <string.h>
#include <stdio.h>

#define MB_DEBUG_TX 1
#define MB_DEBUG_RX 1

/* ================= INIT ================= */
void MB_Master_Init(MB_RTU_t *mb)
{
    mb->frame_ready = 0;
    mb->rx_index = 0;
}

/* ================= DEBUG ================= */
static void MB_Debug_Tx(uint8_t *data, uint16_t len)
{
    extern UART_HandleTypeDef hlpuart1;
    char msg[150];
    int idx = 0;
    const char hex[] = "0123456789ABCDEF";

    idx += sprintf(msg, "TX: ");
    for(int i = 0; i < len; i++)
    {
        uint8_t b = data[i];
        msg[idx++] = hex[(b>>4)&0x0F];
        msg[idx++] = hex[b&0x0F];
        msg[idx++] = ' ';
    }
    msg[idx++] = '\r'; msg[idx++] = '\n';
    HAL_UART_Transmit(&hlpuart1, (uint8_t*)msg, idx, 100);
}

static void MB_Debug_Rx(MB_RTU_t *mb)
{
    extern UART_HandleTypeDef hlpuart1;
    char msg[150];
    int idx = 0;
    const char hex[] = "0123456789ABCDEF";

    idx += sprintf(msg, "RX (%d): ", mb->rx_index);
    for(int i = 0; i < mb->rx_index; i++)
    {
        uint8_t b = mb->rx_buf[i];
        msg[idx++] = hex[(b>>4)&0x0F];
        msg[idx++] = hex[b&0x0F];
        msg[idx++] = ' ';
    }
    msg[idx++] = '\r'; msg[idx++] = '\n';
    HAL_UART_Transmit(&hlpuart1, (uint8_t*)msg, idx, 100);
}

/* ================= SEND CORE ================= */
static MB_MASTER_STATUS MB_Master_SendReq(MB_RTU_t *mb, uint8_t *tx, uint16_t len)
{
    MB_CRC_Append(tx, len);

#if MB_DEBUG_TX
    MB_Debug_Tx(tx, len + 2);
#endif

    if(MB_RTU_Send(mb, tx, len + 2) != HAL_OK)
        return MB_MASTER_ERROR;

    return MB_MASTER_OK;
}

/* ================= READ ================= */
MB_MASTER_STATUS MB_Master_ReadHoldingReg(MB_RTU_t *mb,
                                          uint8_t slave_id,
                                          uint16_t reg_addr,
                                          uint16_t reg_num)
{
    uint8_t tx[8];

    tx[0] = slave_id;
    tx[1] = 0x03;

    tx[2] = reg_addr >> 8;
    tx[3] = reg_addr & 0xFF;

    tx[4] = reg_num >> 8;
    tx[5] = reg_num & 0xFF;

    return MB_Master_SendReq(mb, tx, 6);
}

/* ================= WRITE U16 (DÙNG 0x10) ================= */
MB_MASTER_STATUS MB_Master_WriteU16(MB_RTU_t *mb,
                                    uint8_t slave_id,
                                    uint16_t reg_addr,
                                    uint16_t value)
{
    uint8_t tx[11];

    tx[0] = slave_id;
    tx[1] = 0x10;

    tx[2] = reg_addr >> 8;
    tx[3] = reg_addr & 0xFF;

    tx[4] = 0x00;
    tx[5] = 0x01;   // 1 register

    tx[6] = 0x02;   // byte count

    tx[7] = value >> 8;
    tx[8] = value & 0xFF;

    return MB_Master_SendReq(mb, tx, 9);
}

/* ================= WRITE FLOAT (2 REG) ================= */
MB_MASTER_STATUS MB_Master_WriteFloat(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      float value)
{
    uint8_t tx[13];
    uint32_t raw;

    memcpy(&raw, &value, 4);

    tx[0] = slave_id;
    tx[1] = 0x10;

    tx[2] = reg_addr >> 8;
    tx[3] = reg_addr & 0xFF;

    tx[4] = 0x00;
    tx[5] = 0x02;   // 2 registers

    tx[6] = 0x04;   // 4 bytes

    /* BIG-ENDIAN WORD */
    tx[7]  = (raw >> 24) & 0xFF;
    tx[8]  = (raw >> 16) & 0xFF;
    tx[9]  = (raw >> 8) & 0xFF;
    tx[10] = raw & 0xFF;

    return MB_Master_SendReq(mb, tx, 11);
}

/* ================= WRITE MULTI RAW (NÂNG CAO) ================= */
MB_MASTER_STATUS MB_Master_WriteMulti(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      uint16_t reg_num,
                                      uint8_t *data)
{
    uint8_t tx[64];

    tx[0] = slave_id;
    tx[1] = 0x10;

    tx[2] = reg_addr >> 8;
    tx[3] = reg_addr & 0xFF;

    tx[4] = reg_num >> 8;
    tx[5] = reg_num & 0xFF;

    tx[6] = reg_num * 2;

    memcpy(&tx[7], data, tx[6]);

    return MB_Master_SendReq(mb, tx, 7 + tx[6]);
}

/* ================= POLL ================= */
MB_MASTER_STATUS MB_Master_Poll(MB_RTU_t *mb)
{
    MB_RTU_Poll(mb);

    if(mb->frame_ready == 0)
        return MB_MASTER_TIMEOUT;

    if(MB_CRC_Check(mb->rx_buf, mb->rx_index) == 0)
    {
        mb->frame_ready = 0;
        mb->rx_index = 0;
        return MB_MASTER_CRC_ERROR;
    }

#if MB_DEBUG_RX
    MB_Debug_Rx(mb);
#endif

    return MB_MASTER_OK;
}

/* ================= GET DATA ================= */
uint16_t MB_Master_GetReg16(MB_RTU_t *mb, uint16_t index)
{
    uint16_t pos = 3 + index * 2;

    if(pos + 1 >= mb->rx_index)
        return 0;

    return ((uint16_t)mb->rx_buf[pos] << 8) |
            mb->rx_buf[pos + 1];
}

/* ================= GET FLOAT ================= */
float MB_Master_GetFloat(MB_RTU_t *mb, uint16_t index)
{
    uint16_t pos = 3 + index * 2;

    if(pos + 3 >= mb->rx_index)
        return 0;

    uint32_t raw = 0;

    raw |= (uint32_t)mb->rx_buf[pos] << 24;
    raw |= (uint32_t)mb->rx_buf[pos + 1] << 16;
    raw |= (uint32_t)mb->rx_buf[pos + 2] << 8;
    raw |= (uint32_t)mb->rx_buf[pos + 3];

    float val;
    memcpy(&val, &raw, 4);

    return val;
}
