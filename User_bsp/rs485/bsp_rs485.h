#ifndef __BSP_RS485_H
#define __BSP_RS485_H

#include "main.h"

/*=========================================================
    PORT SELECT
=========================================================*/
typedef enum
{
    RS485_PORT1 = 0,    /* USART1 - Slave (Datalogger) */
    RS485_PORT3         /* USART3 - Master (Sensor RC68) */
} RS485_PORT;

/*=========================================================
    DIR PIN CONFIG
    1 = TX MODE
    0 = RX MODE
=========================================================*/
#define RS485_1_DIR_PORT    GPIOB
#define RS485_1_DIR_PIN     GPIO_PIN_5

#define RS485_3_DIR_PORT    GPIOC
#define RS485_3_DIR_PIN     GPIO_PIN_12

/*=========================================================
    API
=========================================================*/
void BSP_RS485_Init(void);

void BSP_RS485_TX_Mode(RS485_PORT port);
void BSP_RS485_RX_Mode(RS485_PORT port);

/* Blocking Send - Khuyến khích dùng cho Slave Response */
HAL_StatusTypeDef BSP_RS485_Send(RS485_PORT port,
                                 uint8_t *buf,
                                 uint16_t len,
                                 uint32_t timeout);

/* IT và DMA (dành cho Master nếu cần) */
HAL_StatusTypeDef BSP_RS485_Send_IT(RS485_PORT port,
                                    uint8_t *buf,
                                    uint16_t len);

HAL_StatusTypeDef BSP_RS485_Send_DMA(RS485_PORT port,
                                     uint8_t *buf,
                                     uint16_t len);

/* Receive 1 byte Interrupt - Dùng cho cả Master & Slave */
HAL_StatusTypeDef BSP_RS485_Receive_IT(RS485_PORT port,
                                       uint8_t *byte);

/* DMA Receive (nếu sau này dùng) */
HAL_StatusTypeDef BSP_RS485_Receive_DMA(RS485_PORT port,
                                        uint8_t *buf,
                                        uint16_t len);

HAL_StatusTypeDef BSP_RS485_SetBaudrate(RS485_PORT port,
                                        uint32_t baudrate);

/* Wait Transmission Complete */
void BSP_RS485_WaitTC(RS485_PORT port);

/* Helper function - Lấy UART Handle */
UART_HandleTypeDef* BSP_RS485_GetHandle(RS485_PORT port);

#endif
