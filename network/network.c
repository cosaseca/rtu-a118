/*
 * network.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */


#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"

extern uint16_t rtu_crc16(uint8_t *buffer, uint16_t buffer_length);


/* 设置ipv4地址 */
int set_ipv4_config_bin(uint16_t *ip, uint16_t *netmask, uint16_t *gateway) {
	char buf[512];
	sprintf(buf, "ifconfig eth1 %hd.%hd.%hd.%hd "
			"netmask %hd.%hd.%hd.%hd;"
			"route add default gw %hd.%hd.%hd.%hd"
			,*ip, *(ip + 1), *(ip + 2), *(ip + 3)
			,*(netmask), *(netmask + 1), *(netmask + 2), *(netmask + 3)
			,*(gateway), *(gateway + 1), *(gateway + 2), *(gateway + 3));
	printf("%s\n", buf);
	int rc = -1;
	rc = system(buf);
	return rc;
}

/* 设置ipv6地址 */
int set_ipv6_config_bin(uint16_t *ip, uint16_t *netmask, uint16_t *gateway) {
	char buf[512];
	//ifconfig eth1 down;ifconfig eth1 up;
	sprintf(buf, "ip -f inet6 address add %hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx/%hd dev eth1;"
			"route -A inet6 add default gw %hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx"
			,*ip, *(ip + 1), *(ip + 2), *(ip + 3) , *(ip + 4), *(ip + 5), *(ip + 6), *(ip + 7)
			,*(netmask)
			,*(gateway), *(gateway + 1), *(gateway + 2), *(gateway + 3), *(gateway + 4), *(gateway + 5), *(gateway + 6), *(gateway + 7));
	printf("%s\n", buf);
	int rc = -1;
	rc = system(buf);
	return rc;
}

/* 设置tcp服务监听端口 */
int set_tcp_port(uint16_t port) {
	Rtu_a118_t *self = &rtu_a118_t;
	int cmd = MSG_NETWORK_SET_IP;
	pmsg_q_send(self->zigbee_msg_fd, &cmd, sizeof(int), 0);
	return 0;
}

int udp_heart_pack_check_integrity(uint8_t *buffer, uint16_t buffer_length) {
	uint16_t crc_calculated;
	uint16_t crc_received;

	crc_calculated = rtu_crc16(buffer + 2, buffer_length - 2);
	crc_received = (buffer[0] << 8) | (buffer[1]);

	//log_msg("%hu %hu\n", crc_calculated, crc_received);

	/* Check CRC of msg */
	if (crc_calculated == crc_received) {
		return crc_received;
	} else {
		return -1;
	}
}
