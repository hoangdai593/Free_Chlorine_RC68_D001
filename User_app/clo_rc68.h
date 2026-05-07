#ifndef __CLO_RC68_H
#define __CLO_RC68_H

#include "main.h"
#include "modbus_master.h"

/*=========================================================
    CONFIG
=========================================================*/
#define RC68_SLAVE_ID_DEFAULT  0x01

/*=========================================================
    REGISTER MAP (DEC)
=========================================================*/

/* ===== BASIC ===== */
#define RC68_REG_ID             16   // R/W uint16
#define RC68_REG_BAUD           15   // R/W uint16
#define RC68_REG_GAIN           146  // R/W uint16 (0-7)
#define RC68_REG_OFFSET         150  // R/W float (2 reg)

/* ===== MEASUREMENT ===== */
#define RC68_REG_CL_PPM         256  // R float (2 reg)
#define RC68_REG_CL_MV          258  // R float (2 reg)
#define RC68_REG_TEMP           260  // R float (2 reg)
#define RC68_REG_ERROR          262  // R uint16

/* ===== CALIB DATA ===== */
#define RC68_REG_CL_SLOPE       528  // R/W float (2 reg)
#define RC68_REG_CL_INTERCEPT   530  // R/W float (2 reg)

/* ===== CALIB COMMAND ===== */
#define RC68_REG_CALIB_ZERO     544  // W float (2 reg)
#define RC68_REG_CALIB_HIGH     546  // W float (2 reg)

/*=========================================================
    API
=========================================================*/

/* ================= READ ================= */

/* Đọc toàn bộ: ppm + mV + nhiệt độ */
MB_MASTER_STATUS RC68_ReadAll(MB_RTU_t *mb);

/* Đọc riêng */
MB_MASTER_STATUS RC68_ReadChlorine(MB_RTU_t *mb);
MB_MASTER_STATUS RC68_ReadTemperature(MB_RTU_t *mb);
MB_MASTER_STATUS RC68_ReadID(MB_RTU_t *mb);

/* Đọc slope + intercept */
MB_MASTER_STATUS RC68_ReadSlopeIntercept(MB_RTU_t *mb);

/* ================= WRITE ================= */

MB_MASTER_STATUS RC68_WriteID(MB_RTU_t *mb, uint16_t id);
MB_MASTER_STATUS RC68_WriteBaud(MB_RTU_t *mb, uint16_t baud_code);
MB_MASTER_STATUS RC68_WriteGain(MB_RTU_t *mb, uint16_t gain_code);
MB_MASTER_STATUS RC68_WriteOffset(MB_RTU_t *mb, float offset_mv);

/* ================= CALIBRATION ================= */

/* Zero (luôn = 0 ppm) */
MB_MASTER_STATUS RC68_CalibZero(MB_RTU_t *mb);

/* High point */
MB_MASTER_STATUS RC68_CalibHigh(MB_RTU_t *mb, float ppm);

/*=========================================================
    GET VALUE
=========================================================*/

/* Lấy float từ buffer */
float RC68_GetFloat(MB_RTU_t *mb, uint16_t index);

/* Lấy uint16 */
uint16_t RC68_GetU16(MB_RTU_t *mb, uint16_t index);

#endif
