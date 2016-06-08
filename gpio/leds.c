/*
 * leds.c
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
#include "config.h"
#include "cJSON/cJSON.h"
#include "leds.h"
#include "data/data.h"
#include "data/map.h"

#define SET        1
#define GET        0
#define LEDS_DEV_PATH    "/dev/leds"    //led灯设备路径
#define GPIO_DEV_PATH    "/dev/gpio"    //goio设备名
#define ADC_DEV_PATH     "/sys/bus/iio/devices/iio:device0/in_voltage2_raw" //电压adc设备路径

#define RESET_DEFAULT_DELAY   5//5s

//#define printf(format, arg...)


/* 读取gpio口状态, 寄存器接口 */
int gpio_get_do(uint16_t addr, uint16_t *v) {
	Rtu_a118_t *self = &rtu_a118_t;
	if(self->io_fd < 0) {
		return -1;
	}
	Leds_io_t leds_io_t;
	switch(addr) {
#if RTU_HW_VERSION == 1
	case MAP_GET_DO0:
		leds_io_t.num = LEDS_GPIO_DO0;
		break;
	case MAP_GET_DO1:
		leds_io_t.num = LEDS_GPIO_DO1;
		break;
#endif
	case MAP_GET_DO2:
		leds_io_t.num = LEDS_GPIO_DO2;
		break;
	case MAP_GET_DO3:
		leds_io_t.num = LEDS_GPIO_DO3;
		break;
	case MAP_GET_DO4:
		leds_io_t.num = LEDS_GPIO_DO4;
		break;
	case MAP_GET_DO5:
		leds_io_t.num = LEDS_GPIO_DO5;
		break;
	case MAP_GET_DO6:
		leds_io_t.num = LEDS_GPIO_DO6;
		break;
	case MAP_GET_DO7:
		leds_io_t.num = LEDS_GPIO_DO7;
		break;
	case MAP_GET_DI0:
		leds_io_t.num = LEDS_GPIO_DI0;
		break;
	case MAP_GET_DI1:
		leds_io_t.num = LEDS_GPIO_DI1;
		break;
	case MAP_GET_DI2:
		leds_io_t.num = LEDS_GPIO_DI2;
		break;
	case MAP_GET_DI3:
		leds_io_t.num = LEDS_GPIO_DI3;
		break;
	case MAP_GET_DI4:
		leds_io_t.num = LEDS_GPIO_DI4;
		break;
	case MAP_GET_DI5:
		leds_io_t.num = LEDS_GPIO_DI5;
		break;
	case MAP_GET_DI6:
		leds_io_t.num = LEDS_GPIO_DI6;
		break;
	case MAP_GET_DI7:
		leds_io_t.num = LEDS_GPIO_DI7;
		break;
	}
	ioctl(self->io_fd, GET, &leds_io_t);
	*v = leds_io_t.status;
	return 0;
}

/* 设置gpio口状态, 寄存器接口 */
int gpio_set_do(uint16_t addr, uint16_t *v) {
	Rtu_a118_t *self = &rtu_a118_t;
	if(self->io_fd < 0) {
		return -1;
	}
	Leds_io_t leds_io_t;
	Leds_io_t leds_io_a_t;
	Leds_io_t leds_io_b_t;
	switch(addr) {
	case MAP_GET_DO0:
#if RTU_HW_VERSION == 2
		leds_io_a_t.num= LEDS_GPIO_DO0_A;
		leds_io_b_t.num = LEDS_GPIO_DO0_B;
		if(IO_HIGH != *v) {
			leds_io_a_t.status = IO_HIGH;
			leds_io_b_t.status = IO_LOW;
		} else {
			leds_io_a_t.status = IO_LOW;
			leds_io_b_t.status = IO_HIGH;
		}
		ioctl(self->io_fd, SET, &leds_io_a_t);
		ioctl(self->io_fd, SET, &leds_io_b_t);
		usleep(5000);
		leds_io_a_t.status = IO_LOW;
		leds_io_b_t.status = IO_LOW;
		ioctl(self->io_fd, SET, &leds_io_a_t);
		ioctl(self->io_fd, SET, &leds_io_b_t);
		return 0;
#else
		leds_io_t.num = LEDS_GPIO_DO0;
#endif
		break;
	case MAP_GET_DO1:
#if RTU_HW_VERSION == 2
		leds_io_a_t.num= LEDS_GPIO_DO1_A;
		leds_io_b_t.num = LEDS_GPIO_DO1_B;
		if(IO_HIGH != *v) {
			leds_io_a_t.status = IO_HIGH;
			leds_io_b_t.status = IO_LOW;
		} else {
			leds_io_a_t.status = IO_LOW;
			leds_io_b_t.status = IO_HIGH;
		}
		ioctl(self->io_fd, SET, &leds_io_a_t);
		ioctl(self->io_fd, SET, &leds_io_b_t);
		usleep(5000);
		leds_io_a_t.status = IO_LOW;
		leds_io_b_t.status = IO_LOW;
		ioctl(self->io_fd, SET, &leds_io_a_t);
		ioctl(self->io_fd, SET, &leds_io_b_t);
		return 0;
#else
		leds_io_t.num = LEDS_GPIO_DO1;
#endif
		break;
	case MAP_GET_DO2:
		leds_io_t.num = LEDS_GPIO_DO2;
		break;
	case MAP_GET_DO3:
		leds_io_t.num = LEDS_GPIO_DO3;
		break;
	case MAP_GET_DO4:
		leds_io_t.num = LEDS_GPIO_DO4;
		break;
	case MAP_GET_DO5:
		leds_io_t.num = LEDS_GPIO_DO5;
		break;
	case MAP_GET_DO6:
		leds_io_t.num = LEDS_GPIO_DO6;
		break;
	case MAP_GET_DO7:
		leds_io_t.num = LEDS_GPIO_DO7;
		break;
	}
#if RTU_HW_VERSION == 1
	if(MAP_GET_DO6 == addr || MAP_GET_DO7 == addr) {
		leds_io_t.status = *v;
		ioctl(self->io_fd, SET, &leds_io_t);
		Leds_io_t leds_io_t_power;
		leds_io_t_power.num = RELAY_PWR;
		leds_io_t_power.status = IO_HIGH;
		ioctl(self->io_fd, SET, &leds_io_t_power);
		usleep(4000);
		leds_io_t_power.status = IO_LOW;
		ioctl(self->io_fd, SET, &leds_io_t_power);
	} else
#endif
	{
		leds_io_t.status = *v;
		ioctl(self->io_fd, SET, &leds_io_t);
	}
	return 0;
}

/* 设置io状态 */
void gpio_set(int num, char status) {
	Rtu_a118_t *self = &rtu_a118_t;
	if(self->io_fd < 0) {
		return;
	}
	Leds_io_t leds_io_t;
	leds_io_t.num = num;
	leds_io_t.status = status;
	ioctl(self->io_fd, SET, &leds_io_t);
}

/* 读取io状态 */
void gpio_get(int num, uint16_t *status) {
	Rtu_a118_t *self = &rtu_a118_t;
	if(self->io_fd < 0) {
		return;
	}
	Leds_io_t leds_io_t;
	leds_io_t.num = num;
	ioctl(self->io_fd, GET, &leds_io_t);
	*status = leds_io_t.status;
}

void gpio_restart_action() {
	int i = 0;

	for(;i < 3;++i) {
		gpio_set(LED_SEN, LED_OFF);
		gpio_set(LED_LINK, LED_OFF);
		gpio_set(LED_RUN, LED_OFF);
		gpio_set(LED_F3, LED_ON);
		gpio_set(LED_F2, LED_ON);
		gpio_set(LED_F1, LED_ON);

		usleep(300000);
		gpio_set(LED_SEN, LED_ON);
		gpio_set(LED_LINK, LED_ON);
		gpio_set(LED_RUN, LED_ON);
		gpio_set(LED_F3, LED_OFF);
		gpio_set(LED_F2, LED_OFF);
		gpio_set(LED_F1, LED_OFF);
		usleep(300000);
	}
	gpio_set(LED_SEN, LED_OFF);
	gpio_set(LED_LINK, LED_OFF);
	gpio_set(LED_RUN, LED_OFF);
}

/* io口控制线程 */
void *thread_gpio_test(void *arg) {
	Rtu_a118_t *self = &rtu_a118_t;
	char *name = "null";

	cJSON *config = (cJSON *)arg;
	if(NULL == config) {
		printf("cJSON_Parse error\n");
		return NULL;
	}
	cJSON *leaf = cJSON_GetObjectItem(config, "name");
	if(NULL != leaf && cJSON_String == leaf->type) {
		name = leaf->valuestring;
	} else {
		printf("no name config\n");
		return NULL;
	}

	if(0 == strcmp("leds", name)) {

	} else {
		printf("name is not leds\n");
		return NULL;
	}

	char led_status[6] = {-1, -1, -1, -1, -1, -1};
	self->time_set_status = -1;
	char time_set_status = -1;
	memset(self->led_status, -1, sizeof(self->led_status));

	self->flash1_reset_status = -1;
	self->flash1_reset_button = -1;
	int button_down_time = 0;
	char flash1_reset_status = -1;

	printf("[thread] start %s\n", name);
	int i = 0;
#if 0

	for(;i < 3;++i) {
		gpio_set(LED_TMP_SEN, LED_OFF);
		gpio_set(LED_TMP_LINK, LED_OFF);
		gpio_set(LED_TMP_RUN, LED_OFF);
		gpio_set(LED_TMP_F3, LED_ON);
		gpio_set(LED_TMP_F2, LED_ON);
		gpio_set(LED_TMP_F1, LED_ON);

		usleep(300000);
		gpio_set(LED_TMP_SEN, LED_ON);
		gpio_set(LED_TMP_LINK, LED_ON);
		gpio_set(LED_TMP_RUN, LED_ON);
		gpio_set(LED_TMP_F3, LED_OFF);
		gpio_set(LED_TMP_F2, LED_OFF);
		gpio_set(LED_TMP_F1, LED_OFF);
		usleep(300000);
	}
	i = 0;
	gpio_set(LED_TMP_SEN, LED_OFF);
	gpio_set(LED_TMP_LINK, LED_OFF);
	gpio_set(LED_TMP_RUN, LED_OFF);
#endif
	gpio_restart_action();
	uint16_t ipv6_addr[9];
	memset(ipv6_addr, 0, sizeof(ipv6_addr));
	time_t tm0;

	while (1) {
//		gpio_set(LED_F1, LED_ON);
//		gpio_set(LED_F2, LED_ON);
//		gpio_set(LED_F3, LED_ON);
//		printf("ON\n");
//		usleep(1000000);
//		gpio_set(LED_F1, LED_OFF);
//		gpio_set(LED_F2, LED_OFF);
//		gpio_set(LED_F3, LED_OFF);
//		printf("OFF\n");
//		usleep(1000000);
		time(&tm0);
		if(0 == i % 90) {
			get_ipv6_addr(ipv6_addr);
			zigbee_write_registers(ipv6_addr,9,1,144);
		}
		if(0 == i % 20) {
			if(tm0 - self->thread_com_time[THREAD_RS232_ZIGBEE] > THREAD_MAX_COM_DELAY_TIME
				&& tm0 - self->thread_com_time[THREAD_RS485_MASTER] > THREAD_MAX_COM_DELAY_TIME) {
				gpio_set(LED_F1, LED_ON);
			} else {
				gpio_set(LED_F1, LED_OFF);
			}
			
			if(tm0 - self->thread_com_time[THREAD_TCP_SERVER] > THREAD_MAX_COM_DELAY_TIME
				&& tm0 - self->thread_com_time[THREAD_TCP_CLIENT] > THREAD_MAX_COM_DELAY_TIME
				&& tm0 - self->thread_com_time[THREAD_RS485_SLAVE] > THREAD_MAX_COM_DELAY_TIME
				&& tm0 - self->thread_com_time[THREAD_UDP_SERVER] > THREAD_MAX_COM_DELAY_TIME
				&& tm0 - self->thread_com_time[THREAD_RS232_SERVER] > THREAD_MAX_COM_DELAY_TIME) {
				gpio_set(LED_F2, LED_ON);
			} else {
				gpio_set(LED_F2, LED_OFF);
			}

			if(tm0 - self->thread_com_time[THREAD_DATA_SERVER] > THREAD_MAX_COM_DELAY_TIME) {
				gpio_set(LED_F3, LED_ON);
			} else {
				gpio_set(LED_F3, LED_OFF);
			}
		}
		if(0 == i % 4) {
			update_thread_time(THREAD_GPIO_LED);
			gpio_set(LED_RUN, LED_ON);
			if(time_set_status != self->time_set_status) {
				time_set_status = self->time_set_status;
				system("hwclock -f /dev/rtc1 -w");
			}
			if(flash1_reset_status != self->flash1_reset_status) {
				flash1_reset_status = self->flash1_reset_status;
				log_msg("30 reset\n");
				gpio_restart_action();
			}

			if(BUTTON_DOWN == self->flash1_reset_button) {
				++button_down_time;
				if(button_down_time == 3) {
					reset_flash1_to_default();
					log_msg("press reset button");
				}
			} else {
				button_down_time = 0;
			}
		}
//		gpio_set(LED_TMP_SEN, LED_ON);
		if(led_status[0] != self->led_status[0] && 0 == i % 3) {
			gpio_set(LED_SEN, LED_ON);
			led_status[0] = self->led_status[0];
		}
		if(led_status[1] != self->led_status[1] && 0 == i % 2) {
			gpio_set(LED_LINK, LED_ON);
			led_status[1] = self->led_status[1];
		}
		usleep(200000);
		if(0 == i % 4) {
			gpio_set(LED_RUN, LED_OFF);
		}
		if(0 == i % 3) {
			gpio_set(LED_SEN, LED_OFF);
		}
		if(0 == i % 2) {
			gpio_set(LED_LINK, LED_OFF);
		}
		usleep(200000);

		++i;
	}
	return NULL;
}

/* io中断读取函数 */
void *thread_gpio_irq(void *arg) {
	int rc = -1;
	Rtu_a118_t *self = &rtu_a118_t;
	char buf[64];
	int *pinNum = (int *)buf;
	int *pinStatus = pinNum + 1;
	int index = -1;
	uint16_t di_status0[10];
	time_t tm0 = time(NULL);
	gpio_get(GPIO_TO_PIN(1, 15), di_status0);
	gpio_get(GPIO_TO_PIN(1, 14), di_status0 + 1);
	gpio_get(GPIO_TO_PIN(1, 13), di_status0 + 2);
	gpio_get(GPIO_TO_PIN(1, 12), di_status0 + 3);
	gpio_get(GPIO_TO_PIN(0, 27), di_status0 + 4);
	gpio_get(GPIO_TO_PIN(0, 26), di_status0 + 5);
	gpio_get(GPIO_TO_PIN(0, 23), di_status0 + 6);
	gpio_get(GPIO_TO_PIN(0, 22), di_status0 + 7);

	gpio_get(GPIO_TO_PIN(0, 31), di_status0 + 8);
	gpio_get(GPIO_TO_PIN(0, 7), di_status0 + 9);

	//zigbee_write_registers(di_status0, 8, 1, 500 + 0);
	while(1) {
		update_thread_time(THREAD_GPIO_IRQ);
		rc = read(self->io_irq_fd, buf, sizeof(buf));
		if(rc != 2 * sizeof(int)) {

		} else {
			printf("fd %d %d %d %d\n", self->io_irq_fd, rc, *pinNum, *pinStatus);
			switch(*pinNum) {
			case GPIO_TO_PIN(1, 15):
				index = 0;
				break;
			case GPIO_TO_PIN(1, 14):
				index = 1;
				break;
			case GPIO_TO_PIN(1, 13):
				index = 2;
				break;
			case GPIO_TO_PIN(1, 12):
				index = 3;
				break;
			case GPIO_TO_PIN(0, 27):
				index = 4;
				break;
			case GPIO_TO_PIN(0, 26):
				index = 5;
				break;
			case GPIO_TO_PIN(0, 23):
				index = 6;
				break;
			case GPIO_TO_PIN(0, 22):
				index = 7;
				break;
			case GPIO_TO_PIN(0, 31):
				index = 8;
				printf("alarm %d\n", *pinStatus);
				break;
			case GPIO_TO_PIN(0, 7):
				index = 9;
				printf("reset to default %d\n", *pinStatus);
				if(1 == *pinStatus) {
#if 0
					if(time(NULL) - tm0 >= RESET_DEFAULT_DELAY) {
						reset_flash1_to_default();
					}
#endif
					self->flash1_reset_button = BUTTON_UP;
				} else {
					self->flash1_reset_button = BUTTON_DOWN;
					time(&tm0);
				}
				break;
			}
			if(index >= 0 && index < 8) {
				//uint16_t value = *(int *)(buf + 4);
				//zigbee_write_registers(&value, 1, 1, 500 + index);
			}
		}
	}
	return NULL;
}

//set high(1) -> delay 100ms ->set low(0)
int zigbee_restart() {
	gpio_set(XBEE_PWR, IO_HIGH);
	usleep(100000);
	gpio_set(XBEE_PWR, IO_LOW);
	return 0;
}

//set high(1)
int zigbee_init() {
	gpio_set(XBEE_RET, IO_HIGH);
//	zigbee_restart();
	return 0;
}


int gpio_do_init(Rtu_a118_t *self) {
	uint16_t buf[8];
	zigbee_read_registers(buf, sizeof(buf)/sizeof(uint16_t),
		1, MAP_GET_DO0);
	int i;
	for(i = 0;i < sizeof(buf)/sizeof(uint16_t);++i) {
		if(buf[i] != 1) {
			buf[i] = 0;
		}
	}
	zigbee_write_registers(buf, sizeof(buf)/sizeof(uint16_t),
		1, MAP_GET_DO0);
	return 0;
}

/* gpio初始化函数(-1 失败) */
int gpio_init(Rtu_a118_t *self) {
	self->io_fd = open(LEDS_DEV_PATH, O_RDWR);
	int rc = 0;
	if (self->io_fd < 0) {
		perror("can't open %s!\n", LEDS_DEV_PATH);
		rc = -1;
	}
	self->io_irq_fd = -1;
	self->io_irq_fd = open(GPIO_DEV_PATH, O_RDWR);
	if (self->io_irq_fd < 0) {
		perror("can't open %s!\n", GPIO_DEV_PATH);
		rc = -1;
	}

	self->adc_fd = open(ADC_DEV_PATH, O_RDONLY);
	if(-1 == self->adc_fd) {
		perror("ADC_DEV_PATH open error");
		return -1;
	}

	zigbee_init();
	gpio_do_init(self);
	return rc;
}

