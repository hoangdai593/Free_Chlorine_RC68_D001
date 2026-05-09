/*=========================================================
 * File: modbus_master.h
 *=========================================================*/
#ifndef __MODBUS_MASTER_H
#define __MODBUS_MASTER_H

#include "main.h"
#include "modbus_rtu.h"

/* ================= STATUS ================= */
typedef enum
{
    MB_MASTER_OK = 0,
    MB_MASTER_ERROR,
    MB_MASTER_TIMEOUT,
    MB_MASTER_CRC_ERROR,
    MB_MASTER_INVALID_SLAVE,
    MB_MASTER_INVALID_FUNC,
    MB_MASTER_EXCEPTION
} MB_MASTER_STATUS;

/* ================= INIT ================= */
void MB_Master_Init(MB_RTU_t *mb);

/* ================= READ ================= */
MB_MASTER_STATUS MB_Master_ReadHoldingReg(MB_RTU_t *mb,
                                          uint8_t slave_id,
                                          uint16_t reg_addr,
                                          uint16_t reg_num);

MB_MASTER_STATUS MB_Master_ReadInputReg(MB_RTU_t *mb,
                                        uint8_t slave_id,
                                        uint16_t reg_addr,
                                        uint16_t reg_num);

/* ================= WRITE ================= */
MB_MASTER_STATUS MB_Master_WriteU16(MB_RTU_t *mb,
                                    uint8_t slave_id,
                                    uint16_t reg_addr,
                                    uint16_t value);

MB_MASTER_STATUS MB_Master_WriteFloat(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      float value);

MB_MASTER_STATUS MB_Master_WriteSingleReg(MB_RTU_t *mb,
                                          uint8_t slave_id,
                                          uint16_t reg_addr,
                                          uint16_t value);

MB_MASTER_STATUS MB_Master_WriteMulti(MB_RTU_t *mb,
                                      uint8_t slave_id,
                                      uint16_t reg_addr,
                                      uint16_t reg_num,
                                      uint8_t *data);

/* ================= CHECK RESPONSE ================= */
MB_MASTER_STATUS MB_Master_CheckResponse(MB_RTU_t *mb,
                                         uint8_t slave_id,
                                         uint8_t func_code);

/* ================= GET DATA ================= */
uint16_t MB_Master_GetReg16(MB_RTU_t *mb,
                            uint16_t index);

float MB_Master_GetFloat(MB_RTU_t *mb,
                         uint16_t index);

#endif
