/*
 * leds_test.c
 *
 *  Created on: Oct 28, 2014
 *      Author: ygz
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio))

#define LED_SEN    GPIO_TO_PIN(2,1)
#define LED_F1     GPIO_TO_PIN(1,30)
#define LED_LINK   GPIO_TO_PIN(2,0)
#define LED_F2     GPIO_TO_PIN(3,2)
#define LED_RUN    GPIO_TO_PIN(3,1)
#define LED_F3     GPIO_TO_PIN(3,0)

#define ON         0
#define OFF        1
#define SET        1
#define GET        0

typedef struct Leds_io_t {
	int num;
	char status;
}Leds_io_t;

int fd;

void gpio_set(int num, char status);

void gpio_set(int num, char status) {
	Leds_io_t leds_io_t;
	leds_io_t.num = num;
	leds_io_t.status = status;
	ioctl(fd, SET, &leds_io_t);
}

int main(int argc, char * argv) {
	int i, n;
//	Leds_io_t leds_io_t;
	fd = open("/dev/leds", O_RDWR);
	if (fd < 0) {
		printf("can't open /dev/leds!\n");
		exit(1);
	}
	int rc = 0;
	while (1) {
//		leds_io_t.num = LED_F1;
//		leds_io_t.status = ON;
//		rc = ioctl(fd, SET, &leds_io_t);
		gpio_set(LED_F1, ON);
		gpio_set(LED_F2, ON);
		gpio_set(LED_F3, ON);
		printf("ON\n");
		usleep(1000000);
//		leds_io_t.num = LED_F1;
//		leds_io_t.status = OFF;
//		rc = ioctl(fd, SET, &leds_io_t);
		gpio_set(LED_F1, OFF);
		gpio_set(LED_F2, OFF);
		gpio_set(LED_F3, OFF);
		printf("OFF\n");
		usleep(1000000);
	}

	close(fd);

	return 0;
}

