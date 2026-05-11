#include "interface_lcd.h"
#include "Task.h"
#include "button_hanlde.h"
#include <string.h>

/* ================= ENTER BUTTON ================= */

void send_cmd(CMD_TYPE_t type)
{
    CMD_Enqueue(type);
}

void enter_button_handle(void)
{
    switch(interface)
    {
        case MAIN:
            interface = LOGIN;
            break;

        case LOGIN:
            if(pass_cur == 3)
            {
                pass_cur = 0;
                if(memcmp(password, password_true, 4) == 0)
                    interface = SETTING;
                else
                    interface = MAIN;
                memset(password, 0, 4);
            }
            else
            {
                pass_cur++;
            }
            break;

        case SETTING:
            interface = (LCD_INTERFACE)(MODBUS + setting_cursor);
            break;

        case MODBUS:
            if(!modbus_edit)
            {
                modbus_edit = 1;
            }
            else
            {
                modbus_edit = 0;
                send_cmd(CMD_SET_ID_BAUD);
            }
            break;

        case CALIB:
            if(calib_cursor == 0)   // Zero
            {
                send_cmd(CMD_SET_CALIB_ZERO);
            }
            else                    // Slope
            {
                if(!calib_edit)
                {
                    calib_edit = 1;
                    slope_pos = 0;
                }
                else
                {
                    slope_pos++;
                    if(slope_pos >= 3)
                    {
                        calib_edit = 0;
                        send_cmd(CMD_SET_SLOPE);
                    }
                }
            }
            break;

        /* ================= OFFSET ================= */
        case OFFSET:
            if(!offset_edit)
            {
                offset_edit = 1;
                offset_pos = 0;
            }
            else
            {
                offset_pos++;

                if(offset_pos >= 4)
                {
                    offset_edit = 0;
                    send_cmd(CMD_SET_OFFSET);
                }
            }
            break;

        /* ================= WARNING ================= */
        case WARNING:
            if(!warning_edit)
            {
                warning_edit = 1;
                warning_pos = 0;
            }
            else
            {
                if(warning_cursor == 0)
                {
                    warning_edit = 0;
                    cmd_result = CMD_RES_DONE;
                    cmd_ui_tick = HAL_GetTick();
                }
                else
                {
                    warning_pos++;
                    if(warning_pos >= 3)
                    {
                        warning_edit = 0;
                        cmd_result = CMD_RES_DONE;
                        cmd_ui_tick = HAL_GetTick();
                    }
                }
            }
            break;

        case RANGE:
            if(!range_edit)
            {
                range_edit = 1;   // vào edit
            }
            else
            {
                range_edit = 0;   // xác nhận
                send_cmd(CMD_SET_RANGE);
                gain_current = range_index;
            }
            break;

        default:
            break;
    }
}

/* ================= UP ================= */
void up_button_handle(void)
{
    if(interface == LOGIN)
    {
        password[pass_cur]++;
    }

    else if(interface == SETTING)
    {
        if(setting_cursor == MODBUS_CUR)
            setting_cursor = INFO_CUR;
        else
            setting_cursor--;
    }

    else if(interface == MODBUS)
    {
        if(modbus_edit)
        {
            if(modbus_cursor == 0)
            {
                modbus_id++;
                if(modbus_id > 247) modbus_id = 1;
            }
            else
            {
                baud_index++;
                if(baud_index >= BAUD_LIST_SIZE)
                    baud_index = 0;
            }
        }
        else modbus_cursor ^= 1;
    }

    else if(interface == CALIB)
    {
        if(calib_edit)
        {
            slope_digit[slope_pos]++;
            if(slope_digit[slope_pos] > 9) slope_digit[slope_pos] = 0;
        }
        else calib_cursor ^= 1;
    }

    else if(interface == OFFSET)
    {
        if(offset_edit)
        {
            /* SIGN */
            if(offset_pos == 0)
            {
                offset_digit[0] ^= 1;
            }
            else
            {
                offset_digit[offset_pos]++;

                if(offset_digit[offset_pos] > 9)
                    offset_digit[offset_pos] = 0;
            }
        }
    }

    else if(interface == WARNING)
    {
        if(warning_edit)
        {
            if(warning_cursor == 0)
                warning_mode ^= 1;

            else if(warning_cursor == 1)
            {
                upper_digit[warning_pos]++;
                if(upper_digit[warning_pos] > 9) upper_digit[warning_pos] = 0;
            }
            else
            {
                lower_digit[warning_pos]++;
                if(lower_digit[warning_pos] > 9) lower_digit[warning_pos] = 0;
            }
        }
        else
        {
            if(warning_cursor == 0) warning_cursor = 2;
            else warning_cursor--;
        }
    }

    else if(interface == RANGE)
    {
        if(range_edit)
        {
            range_index++;
            if(range_index > 7) range_index = 0;
        }
    }
}

/* ================= DOWN ================= */
void down_button_handle(void)
{
    if(interface == LOGIN)
    {
        password[pass_cur]--;
    }

    else if(interface == SETTING)
    {
        setting_cursor++;
        if(setting_cursor > INFO_CUR)
            setting_cursor = MODBUS_CUR;
    }

    else if(interface == MODBUS)
    {
        if(modbus_edit)
        {
            if(modbus_cursor == 0)
            {
                if(modbus_id == 1) modbus_id = 247;
                else modbus_id--;
            }
            else
            {
                if(baud_index == 0) baud_index = BAUD_LIST_SIZE - 1;
                else baud_index--;
            }
        }
        else modbus_cursor ^= 1;
    }

    else if(interface == CALIB)
    {
        if(calib_edit)
        {
            if(slope_digit[slope_pos] == 0) slope_digit[slope_pos] = 9;
            else slope_digit[slope_pos]--;
        }
        else calib_cursor ^= 1;
    }

    else if(interface == OFFSET)
    {
        if(offset_edit)
        {
            /* SIGN */
            if(offset_pos == 0)
            {
                offset_digit[0] ^= 1;
            }
            else
            {
                if(offset_digit[offset_pos] == 0)
                    offset_digit[offset_pos] = 9;
                else
                    offset_digit[offset_pos]--;
            }
        }
    }

    else if(interface == WARNING)
    {
        if(warning_edit)
        {
            if(warning_cursor == 0)
                warning_mode ^= 1;

            else if(warning_cursor == 1)
            {
                if(upper_digit[warning_pos] == 0) upper_digit[warning_pos] = 9;
                else upper_digit[warning_pos]--;
            }
            else
            {
                if(lower_digit[warning_pos] == 0) lower_digit[warning_pos] = 9;
                else lower_digit[warning_pos]--;
            }
        }
        else
        {
            warning_cursor++;
            if(warning_cursor > 2) warning_cursor = 0;
        }
    }

    else if(interface == RANGE)
    {
        if(range_edit)
        {
            if(range_index == 0) range_index = 7;
            else range_index--;
        }
    }
}


/* ================= EXIT ================= */
void exit_button_handle(void)
{
    if(interface == MODBUS)
    {
        if(modbus_edit) modbus_edit = 0;
        else interface = SETTING;
    }
    else if(interface == CALIB)
    {
        if(calib_edit) calib_edit = 0;
        else interface = SETTING;
    }
    else if(interface == OFFSET)
    {
        if(offset_edit) offset_edit = 0;
        else interface = SETTING;
    }
    else if(interface == WARNING)
    {
        if(warning_edit) warning_edit = 0;
        else interface = SETTING;
    }
    else if(interface == RANGE)
    {
        if(range_edit)
            range_edit = 0;
        else
            interface = SETTING;
    }
    else if(interface != MAIN && interface < MODBUS)
    {
        interface--;
        memset(password,0,4);
        pass_cur = 0;
    }
    else if(interface > MODBUS)
    {
        interface = SETTING;
    }
}
