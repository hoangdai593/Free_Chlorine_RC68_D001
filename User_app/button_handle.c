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

            switch(setting_cursor)
            {
                case MODBUS_CUR:
                    modbus_cursor = 0;
                    modbus_edit = 0;
                    break;

                case CALIB_CUR:
                    calib_cursor = 1;
                    calib_edit = 0;
                    slope_pos = 0;
                    break;

                case OFFSET_CUR:
                    offset_cursor = 0;
                    offset_edit = 0;
                    break;

                case WARNING_CUR:
                    warning_cursor = 0;
                    warning_edit = 0;
                    break;

                case RANGE_CUR:
                    range_edit = 0;
                    break;

                default:
                    break;
            }

            interface = (LCD_INTERFACE)(MODBUS + setting_cursor);
            break;

        /* =====================================================
         * MODBUS
         * ===================================================== */
        case MODBUS:

            if(!modbus_edit)
            {
                modbus_edit = 1;
            }
            else
            {
                modbus_edit = 0;

                confirm_cmd = CMD_SET_SENSOR_ID_BAUD;
                confirm_return_interface = MODBUS;

                interface = CONFIRM_SETUP;
            }

            break;

        /* =====================================================
         * CALIB
         * ===================================================== */
        case CALIB:

            if(calib_cursor == 0)
            {
                confirm_cmd = CMD_SET_CALIB_ZERO;
                confirm_return_interface = CALIB;

                interface = CONFIRM_SETUP;
            }
            else
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

                        confirm_cmd = CMD_SET_SLOPE;
                        confirm_return_interface = CALIB;

                        interface = CONFIRM_SETUP;
                    }
                }
            }

            break;

        /* =====================================================
         * OFFSET
         * ===================================================== */
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

                    confirm_cmd = CMD_SET_OFFSET;
                    confirm_return_interface = OFFSET;

                    interface = CONFIRM_SETUP;
                }
            }

            break;

        /* =====================================================
         * WARNING
         * ===================================================== */
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

                    confirm_cmd = CMD_NONE;
                    confirm_return_interface = WARNING;

                    interface = CONFIRM_SETUP;
                }
                else
                {
                    warning_pos++;

                    if(warning_pos >= 3)
                    {
                        warning_edit = 0;

                        confirm_cmd = CMD_NONE;
                        confirm_return_interface = WARNING;

                        interface = CONFIRM_SETUP;
                    }
                }
            }

            break;

        /* =====================================================
         * RANGE
         * ===================================================== */
        case RANGE:

            if(!range_edit)
            {
                range_edit = 1;
            }
            else
            {
                range_edit = 0;

                confirm_cmd = CMD_SET_RANGE;
                confirm_return_interface = RANGE;

                interface = CONFIRM_SETUP;
            }

            break;

        /* =====================================================
         * CONFIRM SCREEN
         * ===================================================== */
        case CONFIRM_SETUP:

            interface = confirm_return_interface;

            if(confirm_cmd != CMD_NONE)
            {
                send_cmd(confirm_cmd);
            }
            else
            {
                /* WARNING local save */
                warning_mode_saved = warning_mode;

                memcpy(upper_digit_saved, upper_digit, 3);
                memcpy(lower_digit_saved, lower_digit, 3);

                float upper_threshold =
                        upper_digit[0]
                        + upper_digit[1] * 0.1f
                        + upper_digit[2] * 0.01f;

                float lower_threshold =
                        lower_digit[0]
                        + lower_digit[1] * 0.1f
                        + lower_digit[2] * 0.01f;

                MB_SLAVE_SetFloat(&mb_slave, 0x000E, upper_threshold);
                MB_SLAVE_SetFloat(&mb_slave, 0x0010, lower_threshold);
                MB_SLAVE_SetU16(&mb_slave, 0x000D,
                                warning_mode ? 1 : 0);

                buzzer_done_state = 0;
                cmd_result = CMD_RES_DONE;
                cmd_ui_tick = HAL_GetTick();
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
        else 
        {
            calib_cursor ^= 1;
            calib_zero_confirm = 0;
        }
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
        else 
        {
            calib_cursor ^= 1;
            calib_zero_confirm = 0;
        }
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
        if(calib_edit) 
        {
            calib_edit = 0;
            calib_zero_confirm = 0;
        }
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
    else if(interface == SETTING)
    {
        interface = MAIN;
        memset(password,0,4);
        pass_cur = 0;
    }
    else if(interface == LOGIN)
    {
        interface = MAIN;
        memset(password,0,4);
        pass_cur = 0;
    }
    else if(interface == CONFIRM_SETUP)
    {
        interface = confirm_return_interface;
    }
    else if(interface > MODBUS)
    {
        interface = SETTING;
    }
}
