#include "clo_rc68.h"
#include <string.h>

/* =========================================================
 * READ
 * ========================================================= */

MB_MASTER_STATUS RC68_ReadAll(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    RC68_REG_CL_PPM,
                                    6);
}

MB_MASTER_STATUS RC68_ReadChlorine(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    RC68_REG_CL_PPM,
                                    2);
}

MB_MASTER_STATUS RC68_ReadTemperature(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    RC68_REG_TEMP,
                                    2);
}

MB_MASTER_STATUS RC68_ReadID(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    RC68_REG_ID,
                                    1);
}

MB_MASTER_STATUS RC68_ReadSlopeIntercept(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    RC68_REG_CL_SLOPE,
                                    4);
}

/* =========================================================
 * WRITE SINGLE
 * ========================================================= */

MB_MASTER_STATUS RC68_WriteSingleU16(MB_RTU_t *mb,
                                     uint16_t reg,
                                     uint16_t value)
{
    return MB_Master_WriteSingleReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    reg,
                                    value);
}

/* =========================================================
 * WRITE MULTI
 * ========================================================= */

MB_MASTER_STATUS RC68_WriteID(MB_RTU_t *mb,
                              uint16_t id)
{
    if(id == 0 || id > 247)
        return MB_MASTER_ERROR;

    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              RC68_REG_ID,
                              id);
}

MB_MASTER_STATUS RC68_WriteBaud(MB_RTU_t *mb,
                                uint16_t baud_code)
{
    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              RC68_REG_BAUD,
                              baud_code);
}

MB_MASTER_STATUS RC68_WriteGain(MB_RTU_t *mb,
                                uint16_t gain_code)
{
    if(gain_code > 7)
        return MB_MASTER_ERROR;

    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              RC68_REG_GAIN,
                              gain_code);
}

MB_MASTER_STATUS RC68_WriteOffset(MB_RTU_t *mb,
                                  float offset_mv)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                RC68_REG_OFFSET,
                                offset_mv);
}

/* =========================================================
 * CALIB
 * ========================================================= */

MB_MASTER_STATUS RC68_CalibZero(MB_RTU_t *mb)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                RC68_REG_CALIB_ZERO,
                                0.0f);
}

MB_MASTER_STATUS RC68_CalibHigh(MB_RTU_t *mb,
                                float ppm)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                RC68_REG_CALIB_HIGH,
                                ppm);
}

/* =========================================================
 * GET VALUE
 * ========================================================= */

float RC68_GetFloat(MB_RTU_t *mb,
                    uint16_t index)
{
    return MB_Master_GetFloat(mb, index);
}

uint16_t RC68_GetU16(MB_RTU_t *mb,
                     uint16_t index)
{
    return MB_Master_GetReg16(mb, index);
}
