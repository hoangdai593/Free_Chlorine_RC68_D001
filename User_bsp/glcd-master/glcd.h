/**
   \file glcd.h
   \brief GLCD Library main header file. This file must be included into project.
   \author Andy Gock
 */ 

#ifndef _GLCD_H
#define _GLCD_H

#if defined(GLCD_DEVICE_AVR8)
	#include <avr/pgmspace.h>
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include "devices/AVR8.h"
	
	#if !defined(GLCD_USE_AVR_DELAY)
		extern void delay_ms(uint32_t ms);
	#else
		/* Use AVR __delay_ms() function */
		#include <util/delay.h>
		#define delay_ms(t) _delay_ms(t)
	#endif
	
#elif defined(GLCD_DEVICE_LPC111X)
	#include <LPC11xx.h>
	#include "devices/LPC111x.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM
	
#elif defined(GLCD_DEVICE_LPC11UXX)
	#include <LPC11Uxx.h>
	#include "devices/LPC11Uxx.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM

#elif defined(GLCD_DEVICE_STM32F0XX)
	#include <stm32f0xx.h>
	#include <stm32f0xx_gpio.h>
	#include "devices/inc/STM32F0xx.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM

#elif defined(GLCD_DEVICE_STM32F10X)
	#include <stm32f10x.h>
	#include <stm32f10x_gpio.h>
	#include "devices/inc/STM32F10x.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM

#elif defined(GLCD_DEVICE_STM32F4XX)
	#include <stm32f4xx.h>
	#include "devices/STM32F4.h"
	extern void delay_ms(uint32_t ms);
	#define PROGMEM

#elif defined(GLCD_DEVICE_STM32L4XX)
    #include "stm32l4xx_hal.h"
    #include "devices/STM32L4.h"
    #define PROGMEM

#elif defined(GLCD_DEVICE_PIC24H)
	#define FCY (FOSC/2)
	#include <stdint.h>
	#include <xc.h>
	#include <libpic30.h>
	#include "devices/PIC24H.h"
	#define PROGMEM
	#define delay_ms(t) __delay_ms(t)
#elif defined(GLCD_DEVICE_PIC32)
        #include "devices/PIC32.h"
        #define PROGMEM
        extern void delay_ms(unsigned int ms);
#else
//	#error "Device not supported or defined"

#endif

#if defined(GLCD_CONTROLLER_PCD8544)
	#include "controllers/PCD8544.h"
	
#elif defined(GLCD_CONTROLLER_ST7565R)
	#include "controllers/ST7565R.h"	
	
#elif defined(GLCD_CONTROLLER_NT75451)
	#include "controllers/NT75451.h"
		
#else
//	#error "Controller not supported or defined"
	
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#include <stdint.h>

/* Macros */

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

/* Defining new types */

/**
 * Font table type
 */
typedef enum {
	STANG,
	MIKRO,
	GLCD_UTILS
} font_table_type_t;

/**
 * Bounding box for pixels that need to be updated
 */
typedef struct {
	uint8_t x_min;
	uint8_t y_min;
	uint8_t x_max;
	uint8_t y_max;
} glcd_BoundingBox_t;

#include <stdint.h>
#include "glcd_devices.h"
#include "glcd_controllers.h"
#include "glcd_graphics.h"
#include "glcd_graphs.h"
#include "glcd_text_tiny.h"
#include "glcd_text.h"
#include "unit_tests.h"

/**
 * \name Colour Constants
 * @{
 */
#define BLACK 1
#define WHITE 0
/**@}*/

/**
 * \name LCD Dimensions
 * @{
 */

/* 
 * Set to custom value, or leave at 0 for automatic assignment.
 * For custom dimensions, users can define this in their compiler options.
 */

#if !defined(GLCD_LCD_WIDTH) || !defined(GLCD_LCD_HEIGHT)

	/**
	 * User specified GLCD width in pixels
	 * Set to 0 for automatic assignment based on controller.
	 */
	#define GLCD_LCD_WIDTH  0

	/**
	 * User specified GLCD height in pixels
	 * Set to 0 for automatic assignment based on controller.
	 */

	#define GLCD_LCD_HEIGHT 0

	/* Automatic assignment of width and height, if required. */
	#if !GLCD_LCD_WIDTH && !GLCD_LCD_HEIGHT
		#undef GLCD_LCD_WIDTH
		#undef GLCD_LCD_HEIGHT
		#if defined(GLCD_CONTROLLER_PCD8544)
			/* 84x48 is standard for the popular Nokia LCD */
			#define GLCD_LCD_WIDTH 84
			#define GLCD_LCD_HEIGHT 48
		#elif defined(GLCD_CONTROLLER_ST7565R) || defined(GLCD_CONTROLLER_NT75451)
			/* 128x64 is the most popular for this, so we'll use that as default */
			#define GLCD_LCD_WIDTH 128
			#define GLCD_LCD_HEIGHT 64
		#else
			#define GLCD_LCD_WIDTH 128
			#define GLCD_LCD_HEIGHT 64
		#endif
	#endif

#endif /* !defined(GLCD_LCD_WIDTH) || !defined(GLCD_LCD_HEIGHT) */

/*
 * GLCD_NUMBER_OF_BANKS is typically GLCD_LCD_HEIGHT/8
 * Don't adjust these below unless required.
 */
#define GLCD_NUMBER_OF_BANKS (GLCD_LCD_WIDTH / 8)
#define GLCD_NUMBER_OF_COLS  GLCD_LCD_WIDTH

/**@}*/

#if !defined(GLCD_RESET_TIME)
	/** Reset duration by glcd_reset(), in milliseconds */
	#define GLCD_RESET_TIME 1
#endif

/* Global variables used for GLCD library */
extern uint8_t glcd_buffer[GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8];
extern glcd_BoundingBox_t glcd_bbox;
extern uint8_t *glcd_buffer_selected;
extern glcd_BoundingBox_t *glcd_bbox_selected;

/** \name Base Functions 
 *  @{
 */

/**
 * Update bounding box.
 *
 * The bounding box defines a rectangle in which needs to be refreshed next time
 * glcd_write() is called. glcd_write() only writes to those pixels inside the bounding box plus any
 * surrounding pixels which are required according to the bank/column write method of the controller.
 *
 * Define a rectangle here, and it will be <em>added</em> to the existing bounding box.
 *
 * \param xmin Minimum x value of rectangle
 * \param ymin Minimum y value of rectangle
 * \param xmax Maximum x value of rectangle
 * \param ymax Maximum y value of rectangle
 * \see glcd_bbox
 * \see glcd_bbox_selected
 */
void glcd_update_bbox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax);

/**
 * Reset the bounding box.
 * After resetting the bounding box, no pixels are marked as needing refreshing.
 */
void glcd_reset_bbox(void);

/**
 * Same as glcd_reset_bbox()
 */
void glcd_bbox_reset(void);

/**
 * Marks the entire display for re-writing.
 */
void glcd_bbox_refresh(void);
	
/**
 * Clear the display. This will clear the buffer and physically write and commit it to the LCD
 */
void glcd_clear(void);

/**
 * Clear the display buffer only. This does not physically write the changes to the LCD
 */
void glcd_clear_buffer(void);

/**
 * Select screen buffer and bounding box structure.
 * This should be selected at initialisation. There are future plans to support multiple screen buffers
 * but this not yet available.
 * \param buffer Pointer to screen buffer
 * \param bbox   Pointer to bounding box object.
 * \see glcd_BoundingBox_t
 */
void glcd_select_screen(uint8_t *buffer, glcd_BoundingBox_t *bbox);

/**
 * Scroll entire screne buffer by x and y pixels. (not implemented yet)
 * \note Items scrolled off the extents of the display dimensions will be lost.
 *
 * \param x X distance to scroll
 * \param y Y distance to scroll
 */
void glcd_scroll(int8_t x, int8_t y);

/**
 * Scroll screen buffer up by 8 pixels.
 * This is designed to be used in conjunciton with tiny text functions which are 8 bits high.
 * \see Tiny Text
 */
void glcd_scroll_line(void);

/** @}*/

typedef struct {
	const char *font_table;
	uint8_t width;
	uint8_t height;
	char start_char;
	char end_char;
	font_table_type_t table_type;
} glcd_FontConfig_t;

extern uint8_t *glcd_buffer_selected;
extern glcd_BoundingBox_t *glcd_bbox_selected;
extern glcd_FontConfig_t font_current;

#endif
