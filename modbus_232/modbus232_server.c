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

/* 232串口服务线程, 用于请求寄存器数据 */
void *thread_modbus232_server(void *arg) {
	char *name = "null";
	char *dev = "ttyO2";
	int rate = 9600;
	//Rtu_a118_t *self = &rtu_a118_t;

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

	if(0 != strcmp("server", name)) {
		return NULL;
	}
	printf("[thread] start name %s, dev %s, rate %d\n", name, dev, rate);

	ctx = modbus_new_rtu(dev, rate, 'N', 8, 1);
	modbus_set_slave(ctx, 1);
	modbus_connect(ctx);
	ctx->msgfd = -1;

	for (;;) {
		update_thread_time(THREAD_RS232_SERVER);
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
		rc = modbus_receive(ctx, query);
		if (rc >= 0) {
			modbus_reply(ctx, query, rc, NULL);
			update_thread_com_time(THREAD_RS232_SERVER);
		} else {
			/* Connection closed by the client or server */
//			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));
	modbus_free(ctx);

	return 0;
}
