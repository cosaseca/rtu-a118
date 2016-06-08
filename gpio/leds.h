/*
 * leds.h
 *
 *  Created on: Nov 6, 2014
 *      Author: ygz
 */

#ifndef LEDS_H_
#define LEDS_H_
#include <stdint.h>
#include "data/map.h"

#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio))

/*led*/
#define LED_RUN    GPIO_TO_PIN(2,1) //传感器指示灯
#define LED_F3     GPIO_TO_PIN(1,30)

#define LED_LINK   GPIO_TO_PIN(2,0) //连接指示灯
#define LED_F2     GPIO_TO_PIN(3,2)

#define LED_SEN    GPIO_TO_PIN(3,1) //运行指示灯
#define LED_F1     GPIO_TO_PIN(3,0)

#define XBEE_PWR   GPIO_TO_PIN(1,9) //set high(1) -> delay 100ms ->set low(0)
#define XBEE_RET   GPIO_TO_PIN(1,8) //set high(1)

//#define DIR2       GPIO_TO_PIN(0,5)
//#define DIR1       GPIO_TO_PIN(3,0)
//#define PWR_CTRL   GPIO_TO_PIN(0,28)

/*DO*/
#if RTU_HW_VERSION == 2
#define LEDS_GPIO_DO7           GPIO_TO_PIN(0,20) //SSR_A8
#define LEDS_GPIO_DO6           GPIO_TO_PIN(0,29) //SSR_A7
#define LEDS_GPIO_DO5           GPIO_TO_PIN(3,19) //SSR_A6
#define LEDS_GPIO_DO4           GPIO_TO_PIN(3,17) //SSR_A5
#define LEDS_GPIO_DO3           GPIO_TO_PIN(3,14) //SSR_A4
#define LEDS_GPIO_DO2           GPIO_TO_PIN(3,15) //SSR_A3
#define LEDS_GPIO_DO1_A         GPIO_TO_PIN(3,20) //OFF A(1) B(0) -> delay >4ms -> A(0) B(0)
#define LEDS_GPIO_DO1_B         GPIO_TO_PIN(3,21) //ON  A(0) B(1) -> delay >4ms -> A(0) B(0)
#define LEDS_GPIO_DO0_A         GPIO_TO_PIN(0,4)  //OFF A(1) B(0) -> delay >4ms -> A(0) B(0)
#define LEDS_GPIO_DO0_B         GPIO_TO_PIN(3,16) //ON  A(0) B(1) -> delay >4ms -> A(0) B(0)
#else
#define RELAY_PWR      			GPIO_TO_PIN(3,16) //RELAY_PWR
#define LEDS_GPIO_DO7           GPIO_TO_PIN(3,21) //RELAY_SET_A2
#define LEDS_GPIO_DO6           GPIO_TO_PIN(3,20) //RELAY_SET_A1
#define LEDS_GPIO_DO5           GPIO_TO_PIN(0,20) //SSR_A6
#define LEDS_GPIO_DO4           GPIO_TO_PIN(0,29) //SSR_A5
#define LEDS_GPIO_DO3           GPIO_TO_PIN(3,19) //SSR_A4
#define LEDS_GPIO_DO2           GPIO_TO_PIN(3,17) //SSR_A3
#define LEDS_GPIO_DO1           GPIO_TO_PIN(3,14) //SSR_A2
#define LEDS_GPIO_DO0           GPIO_TO_PIN(3,15) //SSR_A1
#endif

#define LEDS_GPIO_DI7           GPIO_TO_PIN(1, 15)
#define LEDS_GPIO_DI6           GPIO_TO_PIN(1, 14)
#define LEDS_GPIO_DI5           GPIO_TO_PIN(1, 13)
#define LEDS_GPIO_DI4           GPIO_TO_PIN(1, 12)
#define LEDS_GPIO_DI3           GPIO_TO_PIN(0, 27)
#define LEDS_GPIO_DI2           GPIO_TO_PIN(0, 26)
#define LEDS_GPIO_DI1           GPIO_TO_PIN(0, 23)
#define LEDS_GPIO_DI0           GPIO_TO_PIN(0, 22)
#define LEDS_GPIO_DI_ALARM      GPIO_TO_PIN(0, 31)
#define LEDS_GPIO_DI_RESET      GPIO_TO_PIN(0, 7)



#define IO_HIGH        1
#define IO_LOW         0

#define LED_ON         IO_LOW
#define LED_OFF        IO_HIGH

#define BUTTON_DOWN           1
#define BUTTON_UP             0

/* led灯控制属性 */
typedef struct Leds_io_t {
	int num;  //管脚名
	char status; //管脚状态
}Leds_io_t;


extern void gpio_set(int num, char status);
extern void *thread_gpio_test(void *arg);
extern int gpio_init(Rtu_a118_t *self);
extern void *thread_gpio_irq(void *arg);

extern int gpio_get_do(uint16_t addr, uint16_t *v);
extern int gpio_set_do(uint16_t addr, uint16_t *v);
extern int zigbee_restart();


#endif /* LEDS_H_ */
