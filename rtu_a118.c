/*
 * rtu_a118.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */

#include <stdio.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "config.h"
#include "libmodbus-3.0.6/src/modbus-private.h"
#include "common/thread.h"
#include "network/network.h"
#include "zigbee/zigbee.h"
#include "modbus_485/modbus_485.h"
#include "modbus_232/modbus_232.h"
#include "data/data.h"
#include "cJSON/cJSON.h"
#include "common/pmsg_q.h"
#include "gpio/leds.h"

Rtu_a118_t rtu_a118_t;  //全局属性

int main(int argc, char *argv[]) {
	log_init();
	Rtu_a118_t *self = &rtu_a118_t;
	//printf("size of equ_meter_para_time %d\n", sizeof(self->equ_meter_para_time));
	memset(self->equ_meter_para_time, 0, sizeof(self->equ_meter_para_time)); //初始化请求仪表参数计时
	memset(self->config_str, '\0', sizeof(self->config_str));  //清空配置缓冲区
	if(-1 == config_load_json_file(self->config_str,
			sizeof(self->config_str) - 1,
			RTU_CONF_FILE)) { //加载配置文件
		perror("load config string %s error\n", RTU_CONF_FILE);
	} else {
		printf("load config string %s ok\n", RTU_CONF_FILE);
	}

	cJSON *config = cJSON_Parse(self->config_str); //解析配置文件
	if(NULL == config) {
		printf("cJSON_Parse error\n");
		return -1;
	}

	if(-1 == map_init(&self->map)){ //初始化映射层
		perror("map_init");
		return -1;
	}

	if(-1 == rtu_sys_info_init(self)) { //初始化系统信息
		perror("rtu_sys_info_init");
		return -1;
	}

    /* 如果消息队列存在，删除消息队列 */
	pmsg_q_rm(ZIGBEE_MSG_PATH);
	pmsg_q_rm(RTU_SERVER_MSG_PATH);
	pmsg_q_rm(RTU_CLIENT_MSG_PATH);
	pmsg_q_rm(TCP_SERVER_MSG_PATH);
	pmsg_q_rm(TCP_CLIENT_MSG_PATH);
	pmsg_q_rm(DATA_MSG_PATH);
	pmsg_q_rm(UDP_SERVER_MSG_PATH);

    /* 创建消息队列 */
	self->zigbee_msg_fd = pmsg_q_open(ZIGBEE_MSG_PATH);           //zigbee
	self->rtu_server_msg_fd = pmsg_q_open(RTU_SERVER_MSG_PATH);   //rs485 slave
	self->rtu_client_msg_fd = pmsg_q_open(RTU_CLIENT_MSG_PATH);   //rs485 master
	self->tcp_server_msg_fd = pmsg_q_open(TCP_SERVER_MSG_PATH);
	self->tcp_client_msg_fd = pmsg_q_open(TCP_CLIENT_MSG_PATH);
	self->data_msg_fd = pmsg_q_open(DATA_MSG_PATH);
	self->udp_server_msg_fd = pmsg_q_open(UDP_SERVER_MSG_PATH);

	printf("pmsg_q_open %d %d %d %d %d %d %d\n", self->zigbee_msg_fd,
			self->rtu_server_msg_fd,
			self->rtu_client_msg_fd,
			self->tcp_server_msg_fd,
			self->tcp_client_msg_fd,
			self->data_msg_fd,
			self->udp_server_msg_fd);

	if(-1 == gpio_init(self)) { //初始化io口
		perror("gpio_init error");
	} else {
		printf("gpio_init ok\n");
	}

	if(-1 == data_init(self)) { //初始化数据库
		perror("data init error");
		return -1;
	} else {
		printf("data init ok\n");
	}

	/*开启线程模块*/
	ithread_start_with_name(thread_zigbee_tty_server,
			cJSON_GetObjectItem(config, "zigbee"),
			"zigbee");/*1*/
	ithread_start_with_name(thread_modbus485_slave,
			cJSON_GetObjectItem(config, "modbus485_slave"),
			"rs485_slave");/*2*/
    if (argc > 1 && 0 == strcmp("mod=f", argv[1])) {
        ithread_start_with_name(thread_modbus485_slave,
            cJSON_GetObjectItem(config, "modbus485_master"),
            "rs485_master");/*3*/
    } else {
        ithread_start_with_name(thread_modbus485_master,
			cJSON_GetObjectItem(config, "modbus485_master"),
			"rs485_master");/*3*/
    }
	ithread_start_with_name(thread_modbus232_server,
			cJSON_GetObjectItem(config, "modbus232_server"),
			"rs232_server");/*4*/
	ithread_start_with_name(thread_tcp_server,
			cJSON_GetObjectItem(config, "tcp_server"),
			"tcp_server");/*5*/
	ithread_start_with_name(thread_tcp_client,
			cJSON_GetObjectItem(config, "tcp_client_test"),
			"tcp_client");/*6*/
	ithread_start_with_name(thread_data,
			cJSON_GetObjectItem(config, "data"),
			"data_server");/*7*/
//	ithread_start_with_name(thread_webserver,
//			cJSON_GetObjectItem(config, "webserver"),
//			"webserver");/*8*/
	ithread_start_with_name(thread_gpio_test,
			cJSON_GetObjectItem(config, "gpio"),
			"gpio");/*9*/
	ithread_start_with_name(thread_gpio_irq,
			cJSON_GetObjectItem(config, "gpio_irq"),
			"gpio_irq");/*10*/
	ithread_start_with_name(thread_udp_server,
			cJSON_GetObjectItem(config, "udp_server"),
			"udp_server");/*11*/


    /* 记录录重启次数 */
	uint16_t start_count = 0;
	zigbee_read_registers(&start_count, 1, 1, MAP_ADDR_APP_START_COUNT);
	start_count += 1;
	zigbee_write_registers(&start_count, 1, 1, MAP_ADDR_APP_START_COUNT);

	//uint16_t rtu_version = 0;
	//zigbee_read_registers(&rtu_version, 1, 1, MAP_ADDR_RTU_VERSION);
	info_printf("rtu_ver=%d", RTU_VERSION);
	info_printf("rtu_start_count=%d", start_count);
	printf("hello rtu_a118\n");
	int i = 0;
	while(1) {
		clear_meter_rtdata(); //清除仪表信息(若规定时间内无数据上传)
		sleep(20);
		if(0 == i % 60) {
			test_thread_time();
		}
		++i;
	}
	return 0;
}
