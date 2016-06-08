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
#include "modbus_485.h"
#include "config.h"
#include "cJSON/cJSON.h"
#include "meter/wired_meter.h"

/* 睡眠函数 */
int timer_sleep(struct timeval *t1) {
	struct timeval tc;
	while (1) {
		gettimeofday(&tc, NULL);
//		printf("in while %u %u\n", tc.tv_sec, tc.tv_usec);
		if (tc.tv_sec * 1000 + tc.tv_usec / 1000
				> t1->tv_sec * 1000 + t1->tv_usec / 1000) {
			break;
		}
		usleep(100);
	}
	return 0;
}

/* rs485主串口服务线程 */
void *thread_modbus485_master(void *arg) {
	char *name = "null";
	char *dev = "ttyO2";
	int rate = 9600;
	uint16_t port_info[5];
	int data_flag = 8;
	int stop_flag = 1;
	char ver_flag = 'N';
	//int duplex_mod;

	cJSON *config = (cJSON *) arg;
	if (NULL == config) {
		printf("cJSON_Parse error\n");
		return NULL;
	}
	cJSON *leaf = cJSON_GetObjectItem(config, "name");
	if (NULL != leaf && cJSON_String == leaf->type) {
		name = leaf->valuestring;
	} else {
		printf("no name config\n");
		return NULL;
	}
	leaf = cJSON_GetObjectItem(config, "dev");
	if (NULL != leaf && cJSON_String == leaf->type) {
		dev = leaf->valuestring;
	} else {
		printf("no dev config\n");
	}
	leaf = cJSON_GetObjectItem(config, "rate");
	if (NULL != leaf && cJSON_Number == leaf->type) {
		rate = leaf->valueint;
	} else {
		printf("no rate config\n");
	}
	modbus_t *ctx;
	int rc;

	if (0 != strcmp("master", name)) {
		return NULL;
	}
	//printf("[thread] start master name %s, dev %s, rate %d\n", name, dev, rate);

	zigbee_read_registers(port_info, 5, 1, MAP_ADDR_RS485_MASTER_SERIAL_PORT);
	init_port_info(port_info, &rate, &ver_flag, &data_flag, &stop_flag);

	//ctx = modbus_new_rtu(dev, rate, 'N', 8, 1);
	ctx = modbus_new_rtu(dev, rate, ver_flag, data_flag, stop_flag);
	modbus_set_slave(ctx, 1);
	modbus_connect(ctx);
	modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485); //设置串口模式为rs485
	ctx->response_timeout.tv_usec = 70000;
	
	printf("[thread] start master name %s, dev %s, rate %d parity %d, data_bit %d, stop_bit %d\n", name, dev, rate,
						ver_flag, data_flag, stop_flag);
	Rtu_a118_t *self = &rtu_a118_t;
	uint8_t master_485_ctrl = 0x00;
	uint32_t wired_electrical_sample_time = 0x00;
	uint16_t wired_electrical_sample_interval = 0x00;
	uint32_t wired_actuator_sample_time = 0x00;
	uint16_t wired_actuator_sample_interval = 0x00;

	//struct timeval syn_t0;
	uint16_t syn_along_info[3];

	ctx->msgfd = self->rtu_client_msg_fd;
    /* 消息处理函数 */
	int msg_handler(int msgfd, MsgData *msg) {
		printf("message queue fd: %d, msg: %d\n", msgfd, *(int * )msg);
		int *msg_cmd = (int *) msg;
		int temp_rate = 9600;
		int temp_data_flag = 8;
		int temp_stop_flag = 1;
		char temp_ver_flag = 'N';
		log_msg("%d", *msg_cmd);
		switch (*(int *) msg) {
		case MSG_CONFIG_START: {
			zigbee_read_registers(port_info, 5, 1, MAP_ADDR_RS485_MASTER_SERIAL_PORT);
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
				//sleep(1);
			} else {
				return -1;
			}
			modbus_close(ctx);
			modbus_free(ctx);
			ctx = modbus_new_rtu(dev, rate, ver_flag, data_flag, stop_flag);
			modbus_set_slave(ctx, 1);
			ctx->msgfd = self->rtu_client_msg_fd;
			ctx->msgfun = msg_handler;
			modbus_connect(ctx);
			printf("[thread] start master name %s, dev %s, rate %d parity %d, data_bit %d, stop_bit %d\n", name, dev, rate,
						ver_flag, data_flag, stop_flag);

			modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
			ctx->response_timeout.tv_usec = 70000;
			break;
		}
		case MSG_CONFIG2_START: {
			uint16_t actuator_well_no;
			zigbee_read_registers((uint16_t *) &actuator_well_no, 1, 1,
					ACTUATOR_WELL_NO_ADDR);
			uint16_t electrical_well_no;
			zigbee_read_registers((uint16_t *) &electrical_well_no, 1, 1,
					ELEC_WELL_NO_ADDR);
			//外界修改的时候需要更新这两个变量
			zigbee_read_registers(
					(uint16_t *) &wired_electrical_sample_interval, 1, electrical_well_no,
					ELEC_INTERVAL_ADDR);
			zigbee_read_registers((uint16_t *) &wired_actuator_sample_interval,
					1, actuator_well_no, ACTUATOR_INTERVAL_ADDR);
			if((wired_electrical_sample_interval > 600) ||(wired_electrical_sample_interval < 10)){
				wired_electrical_sample_interval = 30;
			}
			if((wired_actuator_sample_interval > 600) ||(wired_actuator_sample_interval < 10)){
				wired_actuator_sample_interval = 30;
			}
			break;
		}
		case MSG_LOAD_ELEC_SYSN: {
			master_485_ctrl = MASTER_485_CTRL_ELECTRICAL_SYNC;
			memcpy(syn_along_info, msg_cmd + 1, sizeof(syn_along_info));
			log_msg("%hu %hu %hu", syn_along_info[0], syn_along_info[1], syn_along_info[2]);
			log_msg("%d", master_485_ctrl);
			break;
		}
		}
		return 0;
	}
	ctx->msgfun = msg_handler;

	uint16_t actuator_well_no;
	zigbee_read_registers((uint16_t *) &actuator_well_no, 1, 1,
	ACTUATOR_WELL_NO_ADDR);
	uint16_t electrical_well_no;
	zigbee_read_registers((uint16_t *) &electrical_well_no, 1, 1,
	ELEC_WELL_NO_ADDR);
	//外界修改的时候需要更新这两个变量
	zigbee_read_registers((uint16_t *) &wired_electrical_sample_interval, 1,
			electrical_well_no,
			ELEC_INTERVAL_ADDR);
	zigbee_read_registers((uint16_t *) &wired_actuator_sample_interval, 1,
			actuator_well_no, ACTUATOR_INTERVAL_ADDR);
	if ((wired_electrical_sample_interval > 600)
			|| (wired_electrical_sample_interval < 10)) {
		wired_electrical_sample_interval = 30;
	}
	if ((wired_actuator_sample_interval > 600)
			|| (wired_actuator_sample_interval < 10)) {
		wired_actuator_sample_interval = 30;
	}

	time_t tmp_time;

	fd_set fdset0;     //
	fd_set fdset1;     //
	FD_ZERO(&fdset0);
	FD_SET(ctx->msgfd, &fdset0);
	struct timeval timeout0 = { 5, 0 };
	struct timeval timeout1;
	for (;;) {
		update_thread_time(THREAD_RS485_MASTER);
		fdset1 = fdset0;
		timeout1 = timeout0;
		if ((master_485_ctrl != MASTER_485_CTRL_ELECTRICAL_SYNC)) {
			if (-1
					== (rc = select(ctx->msgfd + 1, &fdset1, NULL, NULL,
							&timeout1))) {
				//error
			}
			if (0 == rc) {
				//timeout
			}
			if (FD_ISSET(ctx->msgfd, &fdset1)) {
				//zhouqi, xiugaile flash
				pmsg_q_receive(ctx->msgfd, self->rtu_client_msg_buf, 0, NULL);
				ctx->msgfun(ctx->msgfd, (MsgData *) self->rtu_client_msg_buf);
				log_msg("%d", master_485_ctrl);
			}
		}
		time(&tmp_time);
		//log_msg("run %d", master_485_ctrl);
		printf("wired_actuator_sample_interval = %d\n",
				wired_actuator_sample_interval);
		if ((tmp_time - wired_electrical_sample_time
				> wired_electrical_sample_interval)
				&& (master_485_ctrl == MASTER_485_CTRL_DEFAULT)) {   //电参采集控制权赋值
			printf("wired_electrical_sample_interval = %d\n",
					wired_electrical_sample_interval);
			wired_electrical_sample_time = tmp_time;
			master_485_ctrl = MASTER_485_CTRL_ELECTRICAL;
		} else if ((tmp_time - wired_actuator_sample_time
				> wired_actuator_sample_interval)
				&& (master_485_ctrl == MASTER_485_CTRL_DEFAULT)) {    //执行器控制权赋值
			printf("wired_actuator_sample_interval = %d\n",
					wired_actuator_sample_interval);
			wired_actuator_sample_time = tmp_time;
			master_485_ctrl = MASTER_485_CTRL_ACTUATOR;
		} else {

		}
		//printf("tmp_master_ctrl = %d\n",master_485_ctrl);
		if(-1 == deal_master_485_packet(ctx, &master_485_ctrl, syn_along_info)) {

		} else {
			update_thread_com_time(THREAD_RS485_MASTER);
		}
//		switch(master_485_ctrl){
//		case :
//
//		}
//		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
//		//rc = modbus_receive(ctx, query);
//		if (rc >= 0) {
//			//modbus_reply(ctx, query, rc, NULL);
//		} else {
//			/* Connection closed by the client or server */
//			printf("modbus_receive error\n");
////			break;
//		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	modbus_free(ctx);

	return 0;
}
