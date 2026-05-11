/*
 * interface_lcd.c
 *
 *  Created on: May 4, 2026
 *      Author: PCBOX
 */
#include "GPIO.h"
#include "Task.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "glcd.h"
#include "ST7565R.h"
#include "glcd_text.h"
#include "glcd_graphics.h"
#include "modbus_rtu.h"
#include "modbus_master.h"
#include "modbus_crc.h"
#include "clo_rc68.h"
#include "interface_lcd.h"
#include "button_hanlde.h"

// UI
LCD_INTERFACE interface = MAIN;
SETTING_CUR   setting_cursor = MODBUS_CUR;

uint8_t  blink = 0;
uint32_t blink_tick = 0;

uint8_t display_update_needed = 1;
LCD_INTERFACE previous_interface = MAIN;


// PASSWORD
uint8_t password[4]      = {0,0,0,0};
uint8_t password_true[4] = {0,0,0,0};

int8_t pass_cur = 0;


// MODBUS SETTING
uint8_t  modbus_cursor = 0;
uint8_t  modbus_edit   = 0;

uint16_t modbus_id = 1;

const uint32_t baud_list[BAUD_LIST_SIZE] =
{
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
    128000,
    256000
};

uint8_t baud_index = 2;


// CALIB
uint8_t calib_cursor = 0;
uint8_t calib_edit   = 0;

uint8_t slope_digit[4] = {0,7,5,0};

int8_t slope_pos = 0;


// OFFSET
uint8_t offset_cursor = 0;
uint8_t offset_edit   = 0;

uint8_t offset_digit[4] = {0,0,0,0};

uint8_t offset_pos = 0;


// WARNING
uint8_t warning_cursor = 0;
uint8_t warning_edit   = 0;

uint8_t warning_mode = 1;

uint8_t upper_digit[3] = {1,5,0};
uint8_t lower_digit[3] = {0,1,0};

uint8_t warning_pos = 0;


// RANGE
const uint16_t range_list[] =
{
    160,
    80,
    40,
    20,
    10,
    5,
    2,
    1
};

uint8_t range_index = 0;
uint8_t range_edit  = 0;

uint8_t gain_current = 0;


void blink_display(void)
{
	if(HAL_GetTick() - blink_tick >= 300)
	{
		blink_tick = HAL_GetTick();
		blink = !blink;
	}
}

void main_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);

    draw_string_small(5,1,"Clo du");
    draw_string_small(85,35,"mg/L");

    /* ================= CLO DISPLAY ================= */
    int clo_int = (int)(clo_value * 100 + 0.5f);

    if(clo_int < 0) clo_int = 0;
    if(clo_int > 9999) clo_int = 9999;

    int clo_i = clo_int / 100;
    int clo_d = clo_int % 100;

    char str[10];
    int idx = 0;

    if(clo_i >= 100)
    {
        str[idx++] = (clo_i / 100) + '0';
        str[idx++] = ((clo_i / 10) % 10) + '0';
        str[idx++] = (clo_i % 10) + '0';
    }
    else if(clo_i >= 10)
    {
        str[idx++] = (clo_i / 10) + '0';
        str[idx++] = (clo_i % 10) + '0';
    }
    else
    {
        str[idx++] = clo_i + '0';
    }

    str[idx++] = '.';
    str[idx++] = (clo_d / 10) + '0';
    str[idx++] = (clo_d % 10) + '0';
    str[idx++] = '\0';

    draw_string_big(30,22,str);

    /* ================= TEMP DISPLAY ================= */
    draw_string_small(5,55,"Temp :");

    /* convert float rc68_temp -> xx.x */
    int temp_int = (int)(rc68_temp * 10 + 0.5f);  // 1 số lẻ

    if(temp_int < 0) temp_int = 0;
    if(temp_int > 999) temp_int = 999;

    int temp_i = temp_int / 10;
    int temp_d = temp_int % 10;

    char temp_str[10];
    idx = 0;

    if(temp_i >= 100)
    {
        temp_str[idx++] = (temp_i / 100) + '0';
        temp_str[idx++] = ((temp_i / 10) % 10) + '0';
        temp_str[idx++] = (temp_i % 10) + '0';
    }
    else if(temp_i >= 10)
    {
        temp_str[idx++] = (temp_i / 10) + '0';
        temp_str[idx++] = (temp_i % 10) + '0';
    }
    else
    {
        temp_str[idx++] = temp_i + '0';
    }

    temp_str[idx++] = '.';
    temp_str[idx++] = temp_d + '0';
    temp_str[idx++] = '\0';

    draw_string_small(50,55,temp_str);

    draw_string_small(92,55,"C");
    glcd_draw_circle(88,55,1,BLACK);
}

void login_display(void)
{
	char str[5];
	int i;

	glcd_clear_buffer();
	draw_string_small(5,1,"Login");
	glcd_draw_line(0,10,128,10,BLACK);
	draw_string_small(25,15,"Enter Password");

    for(i = 0; i < 4; i++)
    {
        str[i] = password[i] + '0';
        if(str[i] > '9') {
            str[i] = '0';
            password[i] = 0;
        } else if(str[i] < '0') {
            str[i] = '9';
            password[i] = 9;
        }
    }

	str[4] = '\0';
	if(blink == 0)
		str[pass_cur] = ' ';
	draw_string_small(50,30,str);
}

void setting_display(void)
{
	glcd_clear_buffer();
	glcd_draw_line(0,8,128,8,BLACK);
	draw_string_small(5,0,"SETTING");

	if(setting_cursor != MODBUS_CUR || blink)
		draw_string_small(5,10,"1. Modbus RTU");

	if(setting_cursor != CALIB_CUR || blink)
		draw_string_small(5,18,"2. Calib");

	if(setting_cursor != OFFSET_CUR || blink)
		draw_string_small(5,26,"3. Offset");

	if(setting_cursor != WARNING_CUR || blink)
		draw_string_small(5,34,"4. Warning");

	if(setting_cursor != RANGE_CUR || blink)
		draw_string_small(5,42,"5. Range");

	if(setting_cursor != INFO_CUR || blink)
		draw_string_small(5,50,"6. Information");
}

void modbus_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Modbus RTU");

    char str_id[20];
    char str_baud[20];

    /* ===== ID ===== */
    if(modbus_edit && modbus_cursor == 0 && blink == 0)
    {
        strcpy(str_id, "ID:   "); // ẩn số
    }
    else
    {
        snprintf(str_id, sizeof(str_id), "ID: %d", modbus_id);
    }

    if(!modbus_edit)
    {
        if(modbus_cursor != 0 || blink)
            draw_string_small(5,20,str_id);
    }
    else
    {
        draw_string_small(5,20,str_id);
    }

    /* ===== BAUD ===== */
    if(modbus_edit && modbus_cursor == 1 && blink == 0)
    {
        strcpy(str_baud, "Baud:     ");
    }
    else
    {
        snprintf(str_baud, sizeof(str_baud), "Baud: %lu", baud_list[baud_index]);
    }

    if(!modbus_edit)
    {
        if(modbus_cursor != 1 || blink)
            draw_string_small(5,30,str_baud);
    }
    else
    {
        draw_string_small(5,30,str_baud);
    }
}

void calib_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Calib");

    /* ===== ZERO ===== */
    if(calib_cursor != 0 || blink)
        draw_string_small(5,20,"Calib zero point");

    /* ===== SLOPE STRING ===== */
    char str[25];

    str[0] = 'S'; str[1] = 'l'; str[2] = 'o'; str[3] = 'p'; str[4] = 'e';
    str[5] = ' '; str[6] = 'p'; str[7] = 'o'; str[8] = 'i'; str[9] = 'n';
    str[10]= 't'; str[11]= ':'; str[12]= ' ';

    str[13] = slope_digit[0] + '0';
    str[14] = '.';
    str[15] = slope_digit[1] + '0';
    str[16] = slope_digit[2] + '0';
    str[17] = '\0';

    /* ===== EDIT MODE ===== */
    if(calib_edit && calib_cursor == 1)
    {
        if(blink == 0)
        {
            if(slope_pos == 0) str[13] = ' ';
            else if(slope_pos == 1) str[15] = ' ';
            else if(slope_pos == 2) str[16] = ' ';
        }

        draw_string_small(5,35,str);
    }
    else
    {
        /* chưa edit → blink cả dòng */
        if(calib_cursor != 1 || blink)
            draw_string_small(5,35,str);
    }

    /* ===== mV VALUE (không blink, không cursor) ===== */
    char mv_str[20];

    int mv_int;

    if(mV_value >= 0)
        mv_int = (int)(mV_value + 0.5f);
    else
        mv_int = (int)(mV_value - 0.5f);

    if(mv_int < -9999) mv_int = -9999;
    if(mv_int >  9999) mv_int =  9999;

    snprintf(mv_str,
             sizeof(mv_str),
             "Value: %+d mV",
             mv_int);

    draw_string_small(5,55,mv_str);
}

void offset_display(void)
{
    glcd_clear_buffer();

    glcd_draw_line(0,10,128,10,BLACK);

    draw_string_small(5,0,"Offset");

    /* =====================================================
     * OFFSET STRING
     * format:
     * Offset: +123 mV
     * ===================================================== */
    char offset_str[25];

    offset_str[0]  = 'O';
    offset_str[1]  = 'f';
    offset_str[2]  = 'f';
    offset_str[3]  = 's';
    offset_str[4]  = 'e';
    offset_str[5]  = 't';
    offset_str[6]  = ':';
    offset_str[7]  = ' ';

    /* sign */
    offset_str[8]  = (offset_digit[0]) ? '-' : '+';

    /* number */
    offset_str[9]  = offset_digit[1] + '0';
    offset_str[10] = offset_digit[2] + '0';
    offset_str[11] = offset_digit[3] + '0';

    offset_str[12] = ' ';
    offset_str[13] = 'm';
    offset_str[14] = 'V';
    offset_str[15] = '\0';

    /* =====================================================
     * BLINK EDIT
     * ===================================================== */
    if(offset_edit)
    {
        if(blink == 0)
        {
            switch(offset_pos)
            {
                case 0:
                    offset_str[8] = ' ';
                    break;

                case 1:
                    offset_str[9] = ' ';
                    break;

                case 2:
                    offset_str[10] = ' ';
                    break;

                case 3:
                    offset_str[11] = ' ';
                    break;

                default:
                    break;
            }
        }

        draw_string_small(5,20,offset_str);
    }
    else
    {
        if(blink || offset_cursor != 0)
        {
            draw_string_small(5,20,offset_str);
        }
    }

    /* =====================================================
     * INTERCEPT
     * ===================================================== */
    char intercept_str[30];

    int intercept_mv;

    if(intercept_value >= 0)
        intercept_mv = (int)(intercept_value + 0.5f);
    else
        intercept_mv = (int)(intercept_value - 0.5f);

    if(intercept_mv < -9999) intercept_mv = -9999;
    if(intercept_mv >  9999) intercept_mv =  9999;

    snprintf(intercept_str,
             sizeof(intercept_str),
             "Intercept: %+0.2f mV",
             intercept_value);

    draw_string_small(5,38,intercept_str);

    /* =====================================================
     * SLOPE
     * ===================================================== */
    char slope_str[30];

    int slope_mv = (int)(slope_value + 0.5f);

    snprintf(slope_str,
             sizeof(slope_str),
             "Slope: %d mV/ppm",
             slope_mv);

    draw_string_small(5,50,slope_str);
}

void warning_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Warning");

    char upper[10];
    char lower[10];

    /* upper */
    upper[0] = upper_digit[0] + '0';
    upper[1] = '.';
    upper[2] = upper_digit[1] + '0';
    upper[3] = upper_digit[2] + '0';
    upper[4] = '\0';

    /* lower */
    lower[0] = lower_digit[0] + '0';
    lower[1] = '.';
    lower[2] = lower_digit[1] + '0';
    lower[3] = lower_digit[2] + '0';
    lower[4] = '\0';

    /* MODE */
    if(warning_cursor == 0 && blink == 0)
    {
        draw_string_small(5,20,"Mode:      ");
    }
    else
    {
        draw_string_small(5,20, warning_mode ? "Mode: ON" : "Mode: OFF");
    }

    /* UPPER */
    if(warning_cursor == 1 && blink == 0)
    {
        if(warning_edit)
        {
            if(warning_pos == 0) upper[0] = ' ';
            if(warning_pos == 1) upper[2] = ' ';
            if(warning_pos == 2) upper[3] = ' ';
        }
        else
        {
            upper[0] = ' ';
            upper[2] = ' ';
            upper[3] = ' ';
        }
    }
    draw_string_small(5,30,"Upper:");
    draw_string_small(65,30,upper);

    /* LOWER */
    if(warning_cursor == 2 && blink == 0)
    {
        if(warning_edit)
        {
            if(warning_pos == 0) lower[0] = ' ';
            if(warning_pos == 1) lower[2] = ' ';
            if(warning_pos == 2) lower[3] = ' ';
        }
        else
        {
            lower[0] = ' ';
            lower[2] = ' ';
            lower[3] = ' ';
        }
    }
    draw_string_small(5,40,"Lower:");
    draw_string_small(65,40,lower);
}

void range_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Range");

    char str[30];

    /* tạo chuỗi bình thường */
    uint8_t display_index = range_edit ? range_index : gain_current;
    snprintf(str, sizeof(str), "Range:0-%d ppm",
             range_list[display_index]);

    if(range_edit)
    {
        if(blink == 0)
        {
            /* ===== ẨN PHẦN SỐ ===== */
            int len = strlen(str);

            /* tìm vị trí số (sau dấu '-') */
            for(int i = 0; i < len; i++)
            {
                if(str[i] == '-')
                {
                    int j = i + 1;

                    /* thay toàn bộ số thành space */
                    while(str[j] >= '0' && str[j] <= '9')
                    {
                        str[j] = ' ';
                        j++;
                    }
                    break;
                }
            }
        }

        draw_string_small(5,30,str);
    }
    else
    {
        /* chưa edit → blink cả dòng */
        if(blink)
            draw_string_small(5,30,str);
    }
}

void info_display(void)
{
	glcd_clear_buffer();
	glcd_draw_line(0,10,128,10,BLACK);
	draw_string_small(5,0,"Information");
	draw_string_small(15,25,"Sao Viet");
	draw_string_small(15,50,"Free Chlorine RC68");
}
