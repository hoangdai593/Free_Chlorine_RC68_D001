#ifndef __CLO_RC68_H
#define __CLO_RC68_H

#include "main.h"
#include "modbus_master.h"

/* ================= CONFIG ================= */
#define RC68_SLAVE_ID_DEFAULT   0x01

/* ================= REGISTER ================= */

/* BASIC */
#define RC68_REG_BAUD           15
#define RC68_REG_ID             16
#define RC68_REG_GAIN           146
#define RC68_REG_OFFSET         150

/* MEASURE */
#define RC68_REG_CL_PPM         256
#define RC68_REG_CL_MV          258
#define RC68_REG_TEMP           260
#define RC68_REG_ERROR          262

/* CALIB DATA */
#define RC68_REG_CL_SLOPE       528
#define RC68_REG_CL_INTERCEPT   530

/* CALIB CMD */
#define RC68_REG_CALIB_ZERO     544
#define RC68_REG_CALIB_HIGH     546

/* ================= READ ================= */

MB_MASTER_STATUS RC68_ReadAll(MB_RTU_t *mb);

MB_MASTER_STATUS RC68_ReadChlorine(MB_RTU_t *mb);

MB_MASTER_STATUS RC68_ReadTemperature(MB_RTU_t *mb);

MB_MASTER_STATUS RC68_ReadID(MB_RTU_t *mb);

MB_MASTER_STATUS RC68_ReadSlopeIntercept(MB_RTU_t *mb);

/* ================= WRITE ================= */

/* write single */
MB_MASTER_STATUS RC68_WriteSingleU16(MB_RTU_t *mb,
                                     uint16_t reg,
                                     uint16_t value);

/* write multi */
MB_MASTER_STATUS RC68_WriteID(MB_RTU_t *mb,
                              uint16_t id);

MB_MASTER_STATUS RC68_WriteBaud(MB_RTU_t *mb,
                                uint16_t baud_code);

MB_MASTER_STATUS RC68_WriteGain(MB_RTU_t *mb,
                                uint16_t gain_code);

MB_MASTER_STATUS RC68_WriteOffset(MB_RTU_t *mb,
                                  float offset_mv);

/* ================= CALIB ================= */

MB_MASTER_STATUS RC68_CalibZero(MB_RTU_t *mb);

MB_MASTER_STATUS RC68_CalibHigh(MB_RTU_t *mb,
                                float ppm);

/* ================= GET ================= */

float RC68_GetFloat(MB_RTU_t *mb,
                    uint16_t index);

uint16_t RC68_GetU16(MB_RTU_t *mb,
                     uint16_t index);

#endif
