#include "clo_rc68.h"
#include <string.h>

/* =========================================================
   READ (CHUẨN THEO DATASHEET)
   ========================================================= */

/* Đọc toàn bộ: ppm + mV + nhiệt độ (3 float = 6 regs) */
MB_MASTER_STATUS RC68_ReadAll(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    0x0100,
                                    6);
}

/* Đọc chlorine ppm */
MB_MASTER_STATUS RC68_ReadChlorine(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    0x0100,
                                    2);
}

/* Đọc nhiệt độ */
MB_MASTER_STATUS RC68_ReadTemperature(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    0x0104,
                                    2);
}

/* Đọc Slave ID */
MB_MASTER_STATUS RC68_ReadID(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    0x0010,
                                    1);
}

/* =========================================================
   WRITE (CHUẨN 0x10)
   ========================================================= */

/* Set Slave ID */
MB_MASTER_STATUS RC68_WriteID(MB_RTU_t *mb, uint16_t id)
{
    if(id == 0 || id > 247)
        return MB_MASTER_ERROR;

    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              0x0010,
                              id);
}

/* Set Baudrate */
MB_MASTER_STATUS RC68_WriteBaud(MB_RTU_t *mb, uint16_t baud_code)
{
    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              0x000F,
                              baud_code);
}

/* Set Gain */
MB_MASTER_STATUS RC68_WriteGain(MB_RTU_t *mb, uint16_t gain_code)
{
    if(gain_code > 7)
        return MB_MASTER_ERROR;

    return MB_Master_WriteU16(mb,
                              RC68_SLAVE_ID_DEFAULT,
                              0x0092,
                              gain_code);
}

/* Set Offset (mV) */
MB_MASTER_STATUS RC68_WriteOffset(MB_RTU_t *mb, float offset_mv)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                0x0096,
                                offset_mv);
}

/* =========================================================
   CALIBRATION
   ========================================================= */

/* Zero calibration */
MB_MASTER_STATUS RC68_CalibZero(MB_RTU_t *mb)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                0x0220,
                                0.0f);
}

/* High point calibration */
MB_MASTER_STATUS RC68_CalibHigh(MB_RTU_t *mb, float ppm)
{
    return MB_Master_WriteFloat(mb,
                                RC68_SLAVE_ID_DEFAULT,
                                0x0222,
                                ppm);
}

/* =========================================================
   READ SLOPE / INTERCEPT
   ========================================================= */

MB_MASTER_STATUS RC68_ReadSlopeIntercept(MB_RTU_t *mb)
{
    return MB_Master_ReadHoldingReg(mb,
                                    RC68_SLAVE_ID_DEFAULT,
                                    0x0210,
                                    4);
}

/* =========================================================
   GET VALUE (PARSE DATA)
   ========================================================= */

/* Lấy float từ buffer (BIG-ENDIAN chuẩn sensor) */
float RC68_GetFloat(MB_RTU_t *mb, uint16_t index)
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

/* Lấy uint16 */
uint16_t RC68_GetU16(MB_RTU_t *mb, uint16_t index)
{
    return MB_Master_GetReg16(mb, index);
}
