/*=========================================================
  FILE: st7565r.c
=========================================================*/
#include "st7565r.h"
#include "glcd.h"

extern SPI_HandleTypeDef hspi2;

/*=========================================================
  LOW LEVEL
=========================================================*/
void glcd_spi_write(uint8_t data)
{
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
}

static void glcd_reset_hw(void)
{
    GLCD_RST_HIGH();
    HAL_Delay(2);

    GLCD_RST_LOW();
    HAL_Delay(5);

    GLCD_RST_HIGH();
    HAL_Delay(5);
}

/*=========================================================
  COMMAND / DATA
=========================================================*/
void glcd_command(uint8_t c)
{
    GLCD_A0_LOW();
    GLCD_CS_LOW();
    glcd_spi_write(c);
    GLCD_CS_HIGH();
}

void glcd_data(uint8_t c)
{
    GLCD_A0_HIGH();
    GLCD_CS_LOW();
    glcd_spi_write(c);
    GLCD_CS_HIGH();
}

/*=========================================================
  BASIC CONTROL
=========================================================*/
void glcd_set_contrast(uint8_t val)
{
    if(val > 63) val = 63;

    glcd_command(ST7565_CMD_CONTRAST_MODE);
    glcd_command(val);
}

void glcd_all_on(void)
{
    glcd_command(ST7565_CMD_ALL_PIXEL_ON);
}

void glcd_normal(void)
{
    glcd_command(ST7565_CMD_NORMAL_DISPLAY);
}

/*=========================================================
  ADDRESS
=========================================================*/
void glcd_set_y_address(uint8_t y)
{
    glcd_command(ST7565_CMD_PAGE_ADDR | (y & 0x0F));
}

void glcd_set_column_upper(uint8_t addr)
{
    glcd_command(ST7565_CMD_COL_UPPER | ((addr >> 4) & 0x0F));
}

void glcd_set_column_lower(uint8_t addr)
{
    glcd_command(ST7565_CMD_COL_LOWER | (addr & 0x0F));
}

void glcd_set_x_address(uint8_t x)
{
    glcd_set_column_upper(x);
    glcd_set_column_lower(x);
}

void glcd_set_start_line(uint8_t addr)
{
    glcd_command(ST7565_CMD_START_LINE | (addr & 0x3F));
}

/*=========================================================
  SCREEN
=========================================================*/
void glcd_clear_now(void)
{
    for(uint8_t page = 0; page < GLCD_NUMBER_OF_BANKS; page++)
    {
        glcd_set_y_address(page);
        glcd_set_x_address(0);

        for(uint8_t col = 0; col < GLCD_NUMBER_OF_COLS; col++)
        {
            glcd_data(0x00);
        }
    }
}

void glcd_pattern(void)
{
    for(uint8_t page = 0; page < GLCD_NUMBER_OF_BANKS; page++)
    {
        glcd_set_y_address(page);
        glcd_set_x_address(0);

        for(uint8_t col = 0; col < GLCD_NUMBER_OF_COLS; col++)
        {
            glcd_data(((col / 8) % 2) ? 0xFF : 0x00);
        }
    }
}

void glcd_write(void)
{
    uint8_t bank;

    for(bank = 0; bank < GLCD_NUMBER_OF_BANKS; bank++)
    {
        glcd_set_y_address(bank);
        glcd_set_x_address(0);

        for(uint8_t col = 0; col < GLCD_NUMBER_OF_COLS; col++)
        {
            glcd_data(glcd_buffer_selected[bank * GLCD_NUMBER_OF_COLS + col]);
        }
    }

    glcd_reset_bbox();
}

/*=========================================================
  POWER
=========================================================*/
void glcd_power_down(void)
{
    glcd_command(0xAE);
}

void glcd_power_up(void)
{
    glcd_command(0xAF);
}

/*=========================================================
  INIT
=========================================================*/
void glcd_ST7565R_init(void)
{
    GLCD_CS_HIGH();

    glcd_reset_hw();

    HAL_Delay(20);

    /* Datasheet sequence */
    glcd_command(ST7565_CMD_INTERNAL_RESET);

    glcd_command(ST7565_CMD_BIAS_9);
    glcd_command(ST7565_CMD_ADC_NORMAL);
    glcd_command(ST7565_CMD_COM_REVERSE);

    glcd_command(ST7565_CMD_START_LINE);

    glcd_command(0x25);          /* resistor ratio */

    glcd_set_contrast(0x19);     /* contrast */

    glcd_command(ST7565_CMD_POWER_CTRL);

    glcd_command(ST7565_CMD_DISPLAY_ON);

    HAL_Delay(20);

    glcd_clear_now();
}
