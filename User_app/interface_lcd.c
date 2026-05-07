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


/* =========================================================
 * LCD / UI CONTROL
 * ========================================================= */
LCD_INTERFACE interface = MAIN;
SETTING_CUR setting_cursor = MODBUS_CUR;

uint8_t  blink = 0;
uint32_t blink_tick = 0;


/* =========================================================
 * PASSWORD
 * ========================================================= */
uint8_t password[4]      = {0, 0, 0, 0};
uint8_t password_true[4] = {0, 0, 0, 0};
int8_t pass_cur = 0;

/* =========================================================
 * MODBUS / SENSOR
 * ========================================================= */
MB_RTU_t mb1;

float clo_value  = 0;
float mV_value   = 0;
float rc68_temp  = 0;
float slope_value = 0;
float intercept_value = 0;

/* =========================================================
 * MODBUS SETTING
 * ========================================================= */
uint8_t  modbus_cursor = 0;   // 0: ID, 1: Baud
uint8_t  modbus_edit   = 0;

uint16_t modbus_id = 1;

/* baud list */
const uint32_t baud_list[] =
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

uint8_t baud_index = 3;   // default 19200


/* =========================================================
 * CALIBRATION
 * ========================================================= */
uint8_t calib_cursor = 0;   // 0: zero, 1: slope
uint8_t calib_edit   = 0;

uint8_t slope_digit[4] = {0,5,5,0};   // 0.55
int8_t  slope_pos = 0;


/* =========================================================
 * OFFSET
 * ========================================================= */
uint8_t offset_cursor = 0;
uint8_t offset_edit   = 0;

uint8_t offset_digit[2] = {1,2};   // 12 mV
uint8_t offset_pos = 0;


/* =========================================================
 * WARNING
 * ========================================================= */
uint8_t warning_cursor = 0;   // 0:mode 1:upper 2:lower
uint8_t warning_edit   = 0;

uint8_t warning_mode = 1;     // 1=ON, 0=OFF

uint8_t upper_digit[3] = {1,0,0};   // 1.00
uint8_t lower_digit[3] = {0,2,0};   // 0.20

uint8_t warning_pos = 0;


/* =========================================================
 * RANGE
 * ========================================================= */
const uint16_t range_list[] = {160,80,40,20,10,5,2,1};

uint8_t range_index = 0;
uint8_t range_edit  = 0;


/* =========================================================
 * CMD CONTROL
 * ========================================================= */
CMD_TYPE_t cmd_type = 0;

void cmd_wait_display(void)
{
    glcd_clear_buffer();

    if(cmd_retry == 0)
        draw_string_small(10,20,"CMD is setting...");
    else
        draw_string_small(10,20,"Retry...");
}

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
    int mv_int = (int)(mV_value + 0.5f);
    if(mv_int < -9999) mv_int = -9999;
    if(mv_int >  9999) mv_int =  9999;
    snprintf(mv_str, sizeof(mv_str), "Value: %d mV", mv_int);
    draw_string_small(5,55,mv_str);
}

void offset_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Offset");

    char str[10];

    str[0] = offset_digit[0] + '0';
    str[1] = offset_digit[1] + '0';
    str[2] = 'm';
    str[3] = 'V';
    str[4] = '\0';

    if(offset_edit)
    {
        if(blink == 0)
        {
            if(offset_pos == 0) str[0] = ' ';
            if(offset_pos == 1) str[1] = ' ';
        }
        draw_string_small(5,30,str);
    }
    else
    {
        if(blink || offset_cursor != 0)
            draw_string_small(5,30,str);
    }
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
    if(warning_edit && warning_cursor == 0 && blink == 0)
        draw_string_small(5,20,"Mode:   ");
    else
        draw_string_small(5,20, warning_mode ? "Mode: ON" : "Mode: OFF");

    /* UPPER */
    if(warning_edit && warning_cursor == 1)
    {
        if(blink == 0)
        {
            if(warning_pos == 0) upper[0]=' ';
            if(warning_pos == 1) upper[2]=' ';
            if(warning_pos == 2) upper[3]=' ';
        }
        draw_string_small(5,30,upper);
    }
    else
    {
        if(warning_cursor != 1 || blink)
            draw_string_small(5,30,upper);
    }

    /* LOWER */
    if(warning_edit && warning_cursor == 2)
    {
        if(blink == 0)
        {
            if(warning_pos == 0) lower[0]=' ';
            if(warning_pos == 1) lower[2]=' ';
            if(warning_pos == 2) lower[3]=' ';
        }
        draw_string_small(5,40,lower);
    }
    else
    {
        if(warning_cursor != 2 || blink)
            draw_string_small(5,40,lower);
    }
}

void range_display(void)
{
    glcd_clear_buffer();
    glcd_draw_line(0,10,128,10,BLACK);
    draw_string_small(5,0,"Range");

    char str[30];

    /* tạo chuỗi bình thường */
    snprintf(str, sizeof(str), "Range:0-%d ppm",
             range_list[range_index]);

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
