/*
 * button_hanlde.h
 *
 *  Created on: May 5, 2026
 *      Author: PCBOX
 */

#ifndef BUTTON_HANLDE_H_
#define BUTTON_HANLDE_H_

#define BT_ENTER_PORT   GPIOC
#define BT_ENTER_PIN    GPIO_PIN_5

#define BT_EXIT_PORT    GPIOB
#define BT_EXIT_PIN     GPIO_PIN_0

#define BT_DOWN_PORT    GPIOB
#define BT_DOWN_PIN     GPIO_PIN_1

#define BT_UP_PORT      GPIOB
#define BT_UP_PIN     GPIO_PIN_2


void enter_button_handle(void);
void down_button_handle(void);
void up_button_handle(void);
void exit_button_handle(void);


#endif /* BUTTON_HANLDE_H_ */
