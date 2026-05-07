/*=========================================================
 * File: modbus_crc.c
 * Desc: Modbus RTU CRC16
 *=========================================================*/
#include "modbus_crc.h"

/*=========================================================
    CALCULATE CRC16
    Polynomial = 0xA001
=========================================================*/
uint16_t MB_CRC16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for(i = 0; i < len; i++)
    {
        crc ^= buf[i];

        for(j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/*=========================================================
    APPEND CRC TO FRAME
=========================================================*/
void MB_CRC_Append(uint8_t *frame, uint16_t len)
{
    uint16_t crc = MB_CRC16(frame, len);

    frame[len]     = (uint8_t)(crc & 0x00FF);       /* Low byte */
    frame[len + 1] = (uint8_t)((crc >> 8) & 0x00FF); /* High byte */
}

/*=========================================================
    CHECK FRAME CRC
    len = total bytes including CRC
=========================================================*/
uint8_t MB_CRC_Check(uint8_t *frame, uint16_t len)
{
    uint16_t crc_calc;
    uint16_t crc_recv;

    if(len < 3)
        return 0;

    crc_calc = MB_CRC16(frame, len - 2);

    crc_recv  = frame[len - 2];
    crc_recv |= ((uint16_t)frame[len - 1] << 8);

    if(crc_calc == crc_recv)
        return 1;

    return 0;
}
