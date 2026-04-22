#include "devices/STM32L4.h"
#include "glcd.h"
#include "delay.h"

extern SPI_HandleTypeDef hspi2;

/*=========================================================
  INIT
=========================================================*/
void glcd_init(void)
{
    /* GPIO đã init bởi CubeMX */

    GLCD_SELECT();
    GLCD_A0_HIGH();
    GLCD_RESET_HIGH();

    HAL_Delay(20);

    /* chọn framebuffer chính */
    glcd_select_screen((uint8_t *)&glcd_buffer, &glcd_bbox);

    /* reset LCD */
    glcd_reset();

    /* init controller */
    glcd_ST7565R_init();

    /* clear màn */
    glcd_clear();
}

/*=========================================================
  SPI WRITE
=========================================================*/
void glcd_spi_write(uint8_t data)
{
	GLCD_SELECT();
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
    GLCD_DESELECT();
}

/*=========================================================
  RESET LCD
=========================================================*/
void glcd_reset(void)
{
    GLCD_RESET_LOW();
    HAL_Delay(20);

    GLCD_RESET_HIGH();
    HAL_Delay(20);
}

/*=========================================================
  BACKLIGHT
=========================================================*/
void glcd_enable_backlight(uint8_t state)
{
    (void)state;
}

void glcd_change_backlight(uint8_t value)
{
    (void)value;
}
