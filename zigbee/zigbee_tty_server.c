/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus-private.h>
#include <modbus.h>
#include "network/network.h"
#include "zigbee/zigbee.h"
#include "config.h"
#include "cJSON/cJSON.h"
#include "gpio/leds.h"

/* zigbee串口服务 */
void *thread_zigbee_tty_server(void *arg) {
	Rtu_a118_t *self = &rtu_a118_t;
	char *name = "null";
	char *dev = "ttyO2";
	int rate = 9600;
	uint16_t zigbee_conf_buf[9];

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
	leaf = cJSON_GetObjectItem(config, "dev");
	if(NULL != leaf && cJSON_String == leaf->type) {
		dev = leaf->valuestring;
	} else {
		printf("no dev config\n");
	}
	leaf = cJSON_GetObjectItem(config, "rate");
	if(NULL != leaf && cJSON_Number == leaf->type) {
		rate = leaf->valueint;
	} else {
		printf("no rate config\n");
	}
	int debug = 0;
	leaf = cJSON_GetObjectItem(config, "debug");
	if(NULL != leaf && cJSON_String == leaf->type) {
		if(0 == strncmp("on", leaf->valuestring, strlen("on"))) {
			debug = 1;
		} else {
			//debug = 0;
		}
	} else {
		printf("no rate config\n");
	}
	modbus_t *ctx;
	int rc;


	ctx = modbus_new_rtu_zigbee(dev, rate, 'N', 8, 1);
	modbus_set_slave(ctx, 1);
	if(-1 == modbus_connect(ctx)) {
		perror("modbus_connect");
		return 0;
	}
	modbus_set_debug(ctx, debug);
	printf("[thread] start name %s, dev %s, rate %d\n", name, dev, rate);

	ctx->msgfd = self->zigbee_msg_fd;
	uint16_t syn_along_info[4];
	int msg_handler(int msgfd, MsgData *msg) {
		printf("message queue fd: %d, msg: %d\n", msgfd, *(int *)msg);
		int *msg_cmd = (int *)msg;
		int rc = 0;
		switch(*(int *)msg) {
		case MSG_CONFIG_START:{
			uint16_t buf[RTU_FLASH1_REGISTER_SIZE];
			zigbee_read_registers(buf, RTU_FLASH1_REGISTER_SIZE, 1, 0);
			set_ipv4_config_bin(buf + MAP_ADDR_IPV4_ADDR,
					buf + MAP_ADDR_IPV4_ADDR + 4,
					buf + MAP_ADDR_IPV4_ADDR + 8);
			set_ipv6_config_bin(buf + MAP_ADDR_IPV6_ADDR,
					buf + MAP_ADDR_IPV6_ADDR + 8,
					buf + MAP_ADDR_IPV6_ADDR + 9);
#if 0
			printf("zigbee config %hx %hx %hx %hx %hx %hx %hx %hx %hx\n",
				*(buf + MAP_ADDR_ZIGBEE_PID),
				*(buf + MAP_ADDR_ZIGBEE_PID + 1),
				*(buf + MAP_ADDR_ZIGBEE_PID + 2),
				*(buf + MAP_ADDR_ZIGBEE_PID + 3),
				*(buf + MAP_ADDR_ZIGBEE_PID + 4),
				*(buf + MAP_ADDR_ZIGBEE_PID + 5),
				*(buf + MAP_ADDR_ZIGBEE_PID + 6),
				*(buf + MAP_ADDR_ZIGBEE_PID + 7),
				*(buf + MAP_ADDR_ZIGBEE_PID + 8));
			printf("zigbee pre config %hx %hx %hx %hx %hx %hx %hx %hx %hx\n",
				*(zigbee_conf_buf),
				*(zigbee_conf_buf + 1),
				*(zigbee_conf_buf + 2),
				*(zigbee_conf_buf + 3),
				*(zigbee_conf_buf + 4),
				*(zigbee_conf_buf + 5),
				*(zigbee_conf_buf + 6),
				*(zigbee_conf_buf + 7),
				*(zigbee_conf_buf + 8));
#endif
			rc = memcmp(zigbee_conf_buf, buf + MAP_ADDR_ZIGBEE_PID, 
				sizeof(zigbee_conf_buf));
			printf("rc of memcmp %d\n", rc);
			if(0 != rc) {
				printf("zigbee config changed\n");
				memcpy(zigbee_conf_buf, buf + MAP_ADDR_ZIGBEE_PID, sizeof(zigbee_conf_buf));
				set_zigbee_para(ctx, (u8 *) (buf + MAP_ADDR_ZIGBEE_PID));
			} else {
				printf("zigbee config not changed\n");
			}
			break;
		}
		case MSG_CONFIG2_START:{
			printf("*(msg_cmd + 1) %d\n", *(msg_cmd + 1));
			break;
		}
		case MSG_LOAD_ELEC_SYSN:{
			memcpy(syn_along_info, msg_cmd + 1, sizeof(syn_along_info));
			printf("MSG_LOAD_ELEC_SYSN syn_along_info %hu %hu %hu %hu\n", 
				syn_along_info[0],
				syn_along_info[1],
				syn_along_info[2],
				syn_along_info[3]);
			send_sysn_electrical_param(ctx, (u16 *)syn_along_info);
		}
		}
		return 0;
	}
	ctx->msgfun = msg_handler;
    //设置ip及zigbee参数
	uint16_t buf_a[RTU_FLASH1_REGISTER_SIZE];
	zigbee_read_registers(buf_a, RTU_FLASH1_REGISTER_SIZE, 1, 0);
	set_ipv4_config_bin(buf_a + MAP_ADDR_IPV4_ADDR,
			buf_a + MAP_ADDR_IPV4_ADDR + 4,
			buf_a + MAP_ADDR_IPV4_ADDR + 8);
	set_ipv6_config_bin(buf_a + MAP_ADDR_IPV6_ADDR,
			buf_a + MAP_ADDR_IPV6_ADDR + 8,
			buf_a + MAP_ADDR_IPV6_ADDR + 9);
	memcpy(zigbee_conf_buf, buf_a + MAP_ADDR_ZIGBEE_PID, sizeof(zigbee_conf_buf));
	set_zigbee_para(ctx, (u8 *) (buf_a + MAP_ADDR_ZIGBEE_PID));
//	modbus_set_debug(ctx, 1);
	for (;;) {
		update_thread_time(THREAD_RS232_ZIGBEE);
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		rc = modbus_receive_zigbee(ctx, query);
		if (rc >= 0) {
//				printf("modbus_receive_zigbee begin\n");
			analysis_zigbee_rcv_data(ctx,query);
			self->led_status[0] += 1;
			update_thread_com_time(THREAD_RS232_ZIGBEE);
			//printf("modbus_receive_zigbee ok\n");
		} else {
			/* Connection closed by the client or server */
			//break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));
	modbus_free(ctx);

	return 0;
}
