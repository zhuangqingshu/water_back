/*
 * board.h
 *
 *  Created on: 2018Äê12ÔÂ30ÈÕ
 *      Author: 79222
 */

#ifndef BOARD_H_
#define BOARD_H_

//#define ENABLE_UART_LOG_OUT 

//led
typedef enum {
	APP_E_LEDS_LED_1 = 0
} APP_teLeds;
#define APP_LEDS_NUM          (1UL)
#define APP_LEDS_LED_1        (GPIO_NUM_15)
#define APP_LEDS_DIO_MASK     (GPIO_Pin_15)

//button
typedef enum {
        APP_E_BUTTONS_BUTTON_1 = 0,
		APP_E_BUTTONS_BUTTON_2,
		APP_E_BUTTONS_BUTTON_3,
		APP_E_BUTTONS_BUTTON_4,
		APP_E_BUTTONS_BUTTON_5,
} APP_teButtons;

#ifdef ENABLE_UART_LOG_OUT
#define APP_BUTTONS_NUM             (3UL)
#else
#define APP_BUTTONS_NUM             (5UL)
#endif
#define APP_BUTTONS_BUTTON_1        (GPIO_NUM_16)
#define APP_BUTTONS_BUTTON_2        (GPIO_NUM_2)
#define APP_BUTTONS_BUTTON_3        (GPIO_NUM_0)
#define APP_BUTTONS_BUTTON_4        (GPIO_NUM_3)
#define APP_BUTTONS_BUTTON_5        (GPIO_NUM_1)
#ifdef ENABLE_UART_LOG_OUT
#define APP_BUTTONS_DIO_MASK        ( BIT(APP_BUTTONS_BUTTON_1) | BIT(APP_BUTTONS_BUTTON_2) | BIT(APP_BUTTONS_BUTTON_3))
#else
#define APP_BUTTONS_DIO_MASK        ( BIT(APP_BUTTONS_BUTTON_1) | BIT(APP_BUTTONS_BUTTON_2) | BIT(APP_BUTTONS_BUTTON_3) | BIT(APP_BUTTONS_BUTTON_4) | BIT(APP_BUTTONS_BUTTON_5))
#endif



#define LED_DISPALY_STB_IO_NUM (GPIO_NUM_12)
#define LED_DISPALY_DIO_IO_NUM (GPIO_NUM_14)
#define LED_DISPALY_CLK_IO_NUM (GPIO_NUM_13)

#define FLOW_SENSOR_IO_NUM     (GPIO_NUM_5)

#define WATER_PUMP_IO_NUM      (GPIO_NUM_4)


#endif /* BOARD_H_ */
