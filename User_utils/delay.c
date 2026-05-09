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
void delay_us(uint32_t us)
{
    uint32_t count;

    while(us--)
    {
        count = 16; // chỉnh theo clock CPU

        while(count--)
        {
            __NOP();
        }
    }
}
