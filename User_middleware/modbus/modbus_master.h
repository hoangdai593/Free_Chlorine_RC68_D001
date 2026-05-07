/*=========================================================
 * File: modbus_master.h
 * Desc: Modbus RTU Master Layer (Clean Version)
 * Note:
 *  - Chỉ xử lý protocol Modbus
 *  - Tất cả write dùng Function 0x10
 *=========================================================*/
#ifndef __MODBUS_MASTER_H
#define __MODBUS_MASTER_H

#include "main.h"
#include "modbus_rtu.h"

/*=========================================================
    STATUS
=========================================================*/
typedef enum
{
    MB_MASTER_OK = 0,
    MB_MASTER_ERROR,
    MB_MASTER_TIMEOUT,
    MB_MASTER_CRC_ERROR
} MB_MASTER_STATUS;

/*=========================================================
    CORE
=========================================================*/
void MB_Master_Init(MB_RTU_t *mb);

/*=========================================================
    READ
=========================================================*/

/* Function 03 - Holding Register */
MB_MASTER_STATUS MB_Master_ReadHoldingReg(MB_RTU_t *mb,
                                          uint8_t slave_id,
                                          uint16_t reg_addr,
                                          uint16_t reg_num);

/* (OPTIONAL) Function 04 - Input Register */
MB_MASTER_STATUS MB_Master_ReadInputReg(MB_RTU_t *mb,
                                        uint8_t slave_id,
                                        uint16_t reg_addr,
                                        uint16_t reg_num);

/*=========================================================
    WRITE (CHUẨN 0x10)
=========================================================*/

/* Ghi 1 register (uint16) */
MB_MASTER_STATUS MB_Master_WriteU16(MB_RTU_t *mb,
                                    uint8_t slave_id,
                                    uint16_t reg_addr,
                                    uint16_t value);

/* Ghi float (2 registers) */
MB_MASTER_STATUS MB_Master_WriteFloat(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      float value);

/* Ghi nhiều register (raw) */
MB_MASTER_STATUS MB_Master_WriteMulti(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      uint16_t reg_num,
                                      uint8_t *data);

/*=========================================================
    POLL
=========================================================*/
MB_MASTER_STATUS MB_Master_Poll(MB_RTU_t *mb);

/*=========================================================
    GET DATA
=========================================================*/

/* Lấy uint16 */
uint16_t MB_Master_GetReg16(MB_RTU_t *mb, uint16_t index);

/* Lấy float (2 register) */
float MB_Master_GetFloat(MB_RTU_t *mb, uint16_t index);

#endif /* __MODBUS_MASTER_H */
