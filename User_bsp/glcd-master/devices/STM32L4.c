/**
 * @file    STM32L4.c
 * @brief   Functions specific to STM32L4xx devices for Andy Gock GLCD library
 *
 * Adapted for STM32CubeIDE + HAL
 */

#include "devices/STM32L4.h"
#include "glcd.h"
#include "delay.h"
/* =========================================================
   EXTERN HANDLES FROM CubeMX
   ========================================================= */
extern SPI_HandleTypeDef hspi2;

/* =========================================================
   LOCAL HELPERS
   ========================================================= */

static void MX_GLCD_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* CS + A0 + RST output */
    GPIO_InitStruct.Pin   = CONTROLLER_SPI_SS_PIN |
                            CONTROLLER_SPI_DC_PIN |
                            CONTROLLER_SPI_RST_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GLCD_DESELECT();
    GLCD_A0_HIGH();
    GLCD_RESET_HIGH();
}

/* =========================================================
   INIT
   ========================================================= */

void glcd_init(void)
{
    MX_GLCD_GPIO_Init();

    /* Select main buffer */
    glcd_select_screen((uint8_t *)&glcd_buffer, &glcd_bbox);

#if defined(GLCD_CONTROLLER_PCD8544)

    glcd_reset();
    glcd_PCD8544_init();
    glcd_clear();

#elif defined(GLCD_CONTROLLER_ST7565R)

    glcd_reset();
    glcd_ST7565R_init();
    glcd_clear();

#else
//#error "Unsupported controller"
#endif
}

/* =========================================================
   SPI WRITE
   ========================================================= */

void glcd_spi_write(uint8_t c)
{
    HAL_SPI_Transmit(&hspi2, &c, 1, HAL_MAX_DELAY);
}

/* =========================================================
   RESET LCD
   ========================================================= */

void glcd_reset(void)
{
    GLCD_RESET_LOW();
    delay_ms(GLCD_RESET_TIME);
    GLCD_RESET_HIGH();
    delay_ms(GLCD_RESET_TIME);
}

/* =========================================================
   BACKLIGHT (OPTIONAL)
   ========================================================= */

void glcd_enable_backlight(uint8_t state)
{
    /* Nếu LCD bạn có chân LED riêng thì tự thêm GPIO ở đây */
    (void)state;
}

void glcd_change_backlight(uint8_t value)
{
    (void)value;
}

/* =========================================================
   DELAY
   ========================================================= */

//void delay_ms(uint32_t ms)
//{
//    HAL_Delay(ms);
//}


