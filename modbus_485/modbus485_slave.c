/*
 * rtu_a118.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus-private.h>
#include <modbus.h>
#include "zigbee/zigbee.h"
#include "config.h"
#include "cJSON/cJSON.h"

/* 获取串口配置 */
int init_port_info(uint16_t *port_info, int *baud, char *parity,
		int *data_bit, int *stop_bit) {
	switch(port_info[0]) {
	case 0:
		*baud = 1200;
		break;
	case 1:
		*baud = 2400;
		break;
	case 2:
		*baud = 4800;
		break;
	case 3:
		*baud = 9600;
		break;
	case 4:
		*baud = 19200;
		break;
	case 5:
		*baud = 38400;
		break;
	case 6:
		*baud = 57600;
		break;
	}

	switch(port_info[1]) {
	case 0:
		*data_bit = 7;
		break;
	case 1:
		*data_bit = 8;
		break;
	}

	switch(port_info[2]) {
	case 0:
		*stop_bit = 1;
		break;
	case 1:
		*stop_bit = 2;
		break;
	}

	switch(port_info[3]) {
	case 0:
		*parity = 'N';
		break;
	case 1:
		*parity = 'E';
		break;
	case 2:
		*parity = 'O';
		break;
	}
	return 0;
}

/* rs485从串口服务线程 */
void *thread_modbus485_slave(void *arg) {
	char *name = "null";
	char *dev = "ttyO2";
	int rate = 9600;
	int data_flag = 8;
	int stop_flag = 1;
	char ver_flag = 'N';
	//int duplex_mod;
	Rtu_a118_t *self = &rtu_a118_t;

	uint16_t port_info[5];

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
	modbus_t *ctx;
	int rc;

	//if(0 != strcmp("slave", name)) {
	//	return NULL;
	//}
	printf("[thread] start slave name %s, dev %s, rate %d\n", name, dev, rate);

	zigbee_read_registers(port_info, 5, 1, MAP_ADDR_SERIAL_PORT);
	init_port_info(port_info, &rate, &ver_flag, &data_flag, &stop_flag);

//	ctx = modbus_new_rtu(dev, rate, 'N', 8, 1);
	ctx = modbus_new_rtu(dev, rate, ver_flag, data_flag, stop_flag);
	modbus_set_slave(ctx, 1);
	modbus_connect(ctx);
	printf("[thread] start slave name %s, dev %s, rate %d parity %d, data_bit %d, stop_bit %d\n", name, dev, rate,
			ver_flag, data_flag, stop_flag);

	modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
	//modbus_set_debug(ctx, 1);

	ctx->msgfd = self->rtu_server_msg_fd;
	int msg_handler(int msgfd, MsgData *msg) {
		printf("message queue fd: %d, msg: %d\n", msgfd, *(int *)msg);
		//int *msg_cmd = (int *)msg;

		int temp_rate = 9600;
		int temp_data_flag = 8;
		int temp_stop_flag = 1;
		char temp_ver_flag = 'N';
		
		switch(*(int *)msg) {
		case MSG_CONFIG_START:{
			zigbee_read_registers(port_info, 5, 1, MAP_ADDR_SERIAL_PORT);
			init_port_info(port_info, &temp_rate, &temp_ver_flag, &temp_data_flag, &temp_stop_flag);
			/* 如果串口配置未改变则返回-1 */
            if(temp_rate != rate
				|| temp_data_flag != data_flag
				|| temp_stop_flag != stop_flag
				|| temp_ver_flag != ver_flag) {
				rate = temp_rate;
				data_flag = temp_data_flag;
				stop_flag = temp_stop_flag;
				ver_flag = temp_ver_flag;
				sleep(1);
			} else {
				return -1;
			}
			modbus_close(ctx);
			modbus_free(ctx);
			ctx = modbus_new_rtu(dev, rate, ver_flag, data_flag, stop_flag);
			modbus_set_slave(ctx, 1);
			ctx->msgfd = self->rtu_server_msg_fd;
			ctx->msgfun = msg_handler;
			modbus_connect(ctx);
			printf("[thread] start slave name %s, dev %s, rate %d parity %d, data_bit %d, stop_bit %d\n", name, dev, rate,
						ver_flag, data_flag, stop_flag);

			modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
			break;
		}
		}
		return 0;
	}
	ctx->msgfun = msg_handler;

	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	for (;;) {
		update_thread_time(THREAD_RS485_SLAVE);
		rc = modbus_receive(ctx, query); //读取串口数据
		if (rc >= 0) {
			modbus_reply(ctx, query, rc, NULL); //回复
			update_thread_com_time(THREAD_RS485_SLAVE);
		} else {
			/* Connection closed by the client or server */
//			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	modbus_free(ctx);

	return 0;
}
