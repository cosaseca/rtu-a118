/*
 * adc.c
 *
 *  Created on: Dec 18, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#define ADC_DEV_PATH     "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"

int main() {
	int fd = open(ADC_DEV_PATH, O_RDONLY);
	if(-1 == fd) {
		perror("ADC_DEV_PATH open error");
		return -1;
	}
	uint16_t v;
	while(1) {
		char buf[64];
		int rc;
		lseek(fd, 0, SEEK_SET);
		rc = read(fd, buf, sizeof(buf) - 1);
		if(rc <= 0) {
			return -1;
		}
		buf[rc - 1] = '\0';
		rc = sscanf(buf, "%hd", &v);
		if(rc != 1) {
			return -1;
		}
		printf("%hd\n", v);
		v =
		printf("hd\n", v * 50 * 100 * 1.8 / (2^12 -1 ));
		usleep(20000);
	}
	return 0;
}
