/*=========================================================
 * File: modbus_crc.h
 * Desc: Modbus RTU CRC16
 *=========================================================*/
#ifndef __MODBUS_CRC_H
#define __MODBUS_CRC_H

#include "main.h"

/*=========================================================
    API
=========================================================*/

/* Tính CRC16 của buffer */
uint16_t MB_CRC16(uint8_t *buf, uint16_t len);

/* Gắn CRC vào cuối frame
   frame[len]   = CRC Low
   frame[len+1] = CRC High
*/
void MB_CRC_Append(uint8_t *frame, uint16_t len);

/* Kiểm tra CRC của frame đã có CRC cuối */
uint8_t MB_CRC_Check(uint8_t *frame, uint16_t len);

#endif
