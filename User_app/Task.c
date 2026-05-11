/*
 * Task.c
 *
 *  Created on: Apr 7, 2026
 *      Author: PCBOX
 */

#include "GPIO.h"
#include "event_driven.h"
#include "Task.h"
#include "main.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>

#include "modbus_rtu.h"
#include "modbus_master.h"
#include "modbus_crc.h"
#include "clo_rc68.h"
#include "modbus_slave.h"
#include "interface_lcd.h"
#include "button_hanlde.h"

/* TASK */
sEvent_struct ALLTASK[] =
{
    {_EVENT_LCD_DISPLAY, 1, 0, 200,  _Cb_LCD_Display},
    {_EVENT_BUTTON,      1, 0, 20,   _Cb_Button},
    {_EVENT_SENSOR,      1, 0, 50,   _Cb_Sensor},
};

/* CMD */
CMD_RESULT_t cmd_result = CMD_RES_NONE;

uint8_t  cmd_running = 0;
uint32_t cmd_tick    = 0;
uint32_t cmd_ui_tick = 0;

CMD_QUEUE_t cmd_queue[CMD_QUEUE_SIZE];

uint8_t cmd_head  = 0;
uint8_t cmd_tail  = 0;
uint8_t cmd_count = 0;

CMD_TYPE_t current_cmd = CMD_NONE;

/* MODBUS */
MB_RTU_t mb1;
MB_SLAVE_t mb_slave;    // Slave   - USART1 (Datalogger)

/* SENSOR */
float clo_value       = 0;
float mV_value        = 0;
float rc68_temp       = 0;

float slope_value     = 0;
float intercept_value = 0;

/* BUZZER */
uint8_t buzzer_done_state = 0;   // 0=idle, 1=active, 2=played
uint32_t buzzer_done_tick = 0;

/* UART RX */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)           // Master - Sensor
    {
        MB_RTU_RxByteHandler(&mb1);
    }
    else if(huart->Instance == USART1)      // Slave - Datalogger
    {
        MB_SLAVE_RxByteHandler(&mb_slave, mb_slave.rx_buf[0]);  // byte đã nhận
    }
}

/* =========================================================
 * QUEUE
 * ========================================================= */

void CMD_Enqueue(CMD_TYPE_t type)
{
    if(cmd_count >= CMD_QUEUE_SIZE)
        return;

    cmd_queue[cmd_tail].type = type;

    cmd_tail++;
    if(cmd_tail >= CMD_QUEUE_SIZE)
        cmd_tail = 0;

    cmd_count++;
}

static uint8_t CMD_Dequeue(CMD_QUEUE_t *cmd)
{
    if(cmd_count == 0)
        return 0;

    *cmd = cmd_queue[cmd_head];

    cmd_head++;
    if(cmd_head >= CMD_QUEUE_SIZE)
        cmd_head = 0;

    cmd_count--;

    return 1;
}

/* =========================================================
 * SEND CMD
 * ========================================================= */

static void send_command(CMD_TYPE_t type)
{
    switch(type)
    {
        case CMD_READ_ALL:
            RC68_ReadAll(&mb1);
            break;

        case CMD_READ_SLOPE_INTERCEPT:
            RC68_ReadSlopeIntercept(&mb1);
            break;

        case CMD_READ_GAIN:
            RC68_ReadGain(&mb1);
            break;

        case CMD_SET_ID_BAUD:
        {
            if(modbus_cursor == 0)
                RC68_WriteID(&mb1, modbus_id);
            else
                RC68_WriteBaud(&mb1, baud_index + 1);

            break;
        }

        case CMD_SET_SLOPE:
        {
            float slope =
                    slope_digit[0]
                    + slope_digit[1] * 0.1f
                    + slope_digit[2] * 0.01f;

            RC68_CalibHigh(&mb1, slope);
            break;
        }

        case CMD_SET_CALIB_ZERO:
            RC68_CalibZero(&mb1);
            break;

        case CMD_SET_OFFSET:
        {
        	float offset =
        	        offset_digit[1] * 100.0f
        	        + offset_digit[2] * 10.0f
        	        + offset_digit[3];

        	if(offset_digit[0] && offset != 0)
        	{
        	    offset = -offset;
        	}

            RC68_WriteOffset(&mb1, offset);
            break;
        }

        case CMD_SET_RANGE:
            RC68_WriteGain(&mb1, range_index);
            break;

        default:
            break;
    }
}

/* =========================================================
 * CHECK RESPONSE
 * ========================================================= */

static uint8_t check_response(CMD_TYPE_t cmd)
{
    /* tối thiểu modbus frame */
    if(mb1.rx_index < 5)
        return 0;

    /* CRC */
    if(MB_CRC_Check(mb1.rx_buf, mb1.rx_index) == 0)
        return 0;

    /* slave id */
    if(mb1.rx_buf[0] != RC68_SLAVE_ID_DEFAULT)
        return 0;

    switch(cmd)
    {
        /* ================= READ ================= */

        case CMD_READ_ALL:

            /* func */
            if(mb1.rx_buf[1] != 0x03)
                return 0;

            /* 6 reg = 12 byte */
            if(mb1.rx_buf[2] != 12)
                return 0;

            return 1;

        case CMD_READ_SLOPE_INTERCEPT:

            if(mb1.rx_buf[1] != 0x03)
                return 0;

            /* 4 reg = 8 byte */
            if(mb1.rx_buf[2] != 8)
                return 0;

            return 1;

        case CMD_READ_GAIN:

            if(mb1.rx_buf[1] != 0x03)
                return 0;

            /* 1 reg = 2 byte */
            if(mb1.rx_buf[2] != 2)
                return 0;

            return 1;

        /* ================= WRITE ================= */

        case CMD_SET_ID_BAUD:
        case CMD_SET_SLOPE:
        case CMD_SET_CALIB_ZERO:
        case CMD_SET_OFFSET:
        case CMD_SET_RANGE:

            /* write multi response */
            if(mb1.rx_buf[1] != 0x10)
                return 0;

            return 1;

        default:
            return 0;
    }
}

/* =========================================================
 * PROCESS QUEUE
 * ========================================================= */

void process_cmd_queue(void)
{
    CMD_QUEUE_t cmd;

    /* =====================================================
     * ĐANG CHỜ RESPONSE
     * ===================================================== */
    if(cmd_running)
    {
        /* ===== có response ===== */
    	if(mb1.frame_ready)
    	{
    	    mb1.frame_ready = 0;

    	    /* ===== CHECK RESPONSE ===== */
    	    if(check_response(current_cmd) == 0)
    	    {
    	        mb1.rx_index = 0;

    	        cmd_running = 0;

    	        if(current_cmd >= CMD_SET_ID_BAUD)
    	        {
    	            cmd_result  = CMD_RES_FAIL;
    	            cmd_ui_tick = HAL_GetTick();
    	        }

    	        return;
    	    }
            /* ===== parse data ===== */
            if(mb1.rx_buf[1] == 0x03)
            {
                if(current_cmd == CMD_READ_ALL)
                {
                    clo_value = RC68_GetFloat(&mb1, 0);
                    mV_value  = RC68_GetFloat(&mb1, 2);
                    rc68_temp = RC68_GetFloat(&mb1, 4);
                    display_update_needed = 1;
                }
                else if(current_cmd == CMD_READ_SLOPE_INTERCEPT)
                {
                    slope_value     = RC68_GetFloat(&mb1, 0);
                    intercept_value = RC68_GetFloat(&mb1, 2);
                }
                else if(current_cmd == CMD_READ_GAIN)
                {
                    gain_current = RC68_GetU16(&mb1, 0);
                    display_update_needed = 1;
                }
            }

            mb1.rx_index = 0;

            cmd_running = 0;

            /* ===== chỉ hiện UI cho lệnh SET ===== */
            if(current_cmd >= CMD_SET_ID_BAUD)
            {
                cmd_result  = CMD_RES_DONE;
                cmd_ui_tick = HAL_GetTick();
            }

            return;
        }

        /* ===== timeout ===== */
        if(HAL_GetTick() - cmd_tick >= CMD_TIMEOUT_MS)
        {
            cmd_running = 0;

            mb1.rx_index   = 0;
            mb1.frame_ready = 0;

            /* ===== chỉ hiện UI cho lệnh SET ===== */
            if(current_cmd >= CMD_SET_ID_BAUD)
            {
                cmd_result  = CMD_RES_FAIL;
                cmd_ui_tick = HAL_GetTick();
            }
        }

        return;
    }

    /* =====================================================
     * KHÔNG CÓ CMD
     * ===================================================== */
    if(!CMD_Dequeue(&cmd))
        return;

    current_cmd = cmd.type;

    /* ===== gửi lệnh ===== */
    send_command(current_cmd);

    cmd_running = 1;

    cmd_tick = HAL_GetTick();

    /* ===== chỉ hiện UI cho lệnh SET ===== */
    if(current_cmd >= CMD_SET_ID_BAUD)
    {
        cmd_result = CMD_RES_SENDING;
    }
}

/* =========================================================
 * SENSOR
 * ========================================================= */

uint8_t _Cb_Sensor(uint8_t x)
{
    static uint32_t read_tick = 0;
    static uint32_t slope_tick = 0;

    /* Poll Master (Sensor) */
    MB_RTU_Poll(&mb1);

    /* Poll Slave (Datalogger) */
    MB_SLAVE_Poll(&mb_slave);

    /* Xử lý command queue cho Master */
    process_cmd_queue();

    /* Đọc dữ liệu realtime từ Sensor */
    if(HAL_GetTick() - read_tick >= 1000)
    {
        read_tick = HAL_GetTick();
        CMD_Enqueue(CMD_READ_ALL);
    }

    /* Đọc Slope/Intercept định kỳ */
    if(HAL_GetTick() - slope_tick >= 10000)
    {
        slope_tick = HAL_GetTick();
        CMD_Enqueue(CMD_READ_SLOPE_INTERCEPT);
    }

    /* Cập nhật dữ liệu lên Slave Registers (cho Datalogger đọc) */
    MB_SLAVE_SetFloat(&mb_slave, 0x0002, clo_value);      // Giá trị Clo dư
    MB_SLAVE_SetFloat(&mb_slave, 0x0004, rc68_temp);      // Nhiệt độ

    float offset_value =
            offset_digit[1] * 100.0f
            + offset_digit[2] * 10.0f
            + offset_digit[3];
    if(offset_digit[0] && offset_value != 0.0f)
    {
        offset_value = -offset_value;
    }

    float upper_threshold = upper_digit[0] + upper_digit[1] * 0.1f + upper_digit[2] * 0.01f;
    float lower_threshold = lower_digit[0] + lower_digit[1] * 0.1f + lower_digit[2] * 0.01f;

    /* Đồng bộ giá trị và tắt còi nếu không phải âm báo DONE */
    if(buzzer_done_state == 1)
    {
        uint32_t elapsed = HAL_GetTick() - buzzer_done_tick;
        if(elapsed < 200)  // 0.2s pulse
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else
        {
            buzzer_done_state = 2;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }

    MB_SLAVE_SetFloat(&mb_slave, 0x0006, offset_value);   // Bù pH cho Clo
    MB_SLAVE_SetU16(&mb_slave, 0x000D, warning_mode ? 1 : 0); // Cảnh báo bật/tắt
    MB_SLAVE_SetFloat(&mb_slave, 0x000E, upper_threshold); // Ngưỡng trên
    MB_SLAVE_SetFloat(&mb_slave, 0x0010, lower_threshold); // Ngưỡng dưới

    return 0;
}

/* =========================================================
 * LCD
 * ========================================================= */

uint8_t _Cb_LCD_Display(uint8_t x)
{
    static uint32_t display_tick = 0;

    blink_display();

    /* CMD UI */
    if(cmd_result != CMD_RES_NONE)
    {
        glcd_clear_buffer();

        if(cmd_result == CMD_RES_SENDING)
        {
            draw_string_small(10,20,"SENDING...");
            buzzer_done_state = 0;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
        else if(cmd_result == CMD_RES_DONE)
        {
            draw_string_small(10,20,"DONE");
            if(buzzer_done_state == 0)
            {
                buzzer_done_state = 1;
                buzzer_done_tick = HAL_GetTick();
            }
        }
        else if(cmd_result == CMD_RES_FAIL)
        {
            draw_string_small(10,20,"FAIL");
            buzzer_done_state = 0;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }

        if(cmd_result != CMD_RES_SENDING)
        {
            if(display_tick == 0)
                display_tick = HAL_GetTick();

            if(HAL_GetTick() - display_tick >= 1000)
            {
                cmd_result = CMD_RES_NONE;
                display_tick = 0;

                interface = SETTING;
            }
        }

        return 0;
    }

    display_tick = 0;

    if(interface != previous_interface)
    {
        display_update_needed = 1;
        previous_interface = interface;
    }

    switch(interface)
    {
        case MAIN:
            if(display_update_needed)
            {
                main_display();
                display_update_needed = 0;
            }
            break;

        case LOGIN:
            login_display();
            break;

        case SETTING:
            setting_display();
            break;

        case MODBUS:
            modbus_display();
            break;

        case CALIB:
            calib_display();
            break;

        case OFFSET:
            offset_display();
            break;

        case WARNING:
            warning_display();
            break;

        case RANGE:
            range_display();
            break;

        case INFO:
            info_display();
            break;

        default:
            break;
    }

    return 0;
}

/* =========================================================
 * BUTTON
 * ========================================================= */

uint8_t _Cb_Button(uint8_t x)
{
    if(HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0)
    {
        while(HAL_GPIO_ReadPin(BT_ENTER_PORT, BT_ENTER_PIN) == 0){}

        enter_button_handle();
    }
    else if(HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0)
    {
        while(HAL_GPIO_ReadPin(BT_DOWN_PORT, BT_DOWN_PIN) == 0){}

        down_button_handle();
    }
    else if(HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0)
    {
        while(HAL_GPIO_ReadPin(BT_UP_PORT, BT_UP_PIN) == 0){}

        up_button_handle();
    }
    else if(HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0)
    {
        while(HAL_GPIO_ReadPin(BT_EXIT_PORT, BT_EXIT_PIN) == 0){}

        exit_button_handle();
    }

    return 0;
}

/* =========================================================
 * COMM TASK
 * ========================================================= */

uint8_t Comm_Task(void)
{
    uint8_t i;

    for(i = 0; i < 3; i++)
    {
        if(ALLTASK[i].e_status == 1)
        {
            if((ALLTASK[i].e_systick == 0)
            || ((HAL_GetTick() - ALLTASK[i].e_systick)
            >= ALLTASK[i].e_period))
            {
                ALLTASK[i].e_systick = HAL_GetTick();

                ALLTASK[i].e_function_handler(i);
            }
        }
    }

    return 0;
}
