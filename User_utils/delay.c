/*
 * delay.c
 *
 *  Created on: Apr 22, 2026
 *      Author: PCBOX
 */
#include "delay.h"

void delay_ms(uint16_t ms)
{
	HAL_Delay(ms);
}
