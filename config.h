/*
 * config.h
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "modbus.h"
#include "common/pmsg_q.h"
#include <sys/mman.h>
#include "data/map.h"
#include "sqlite/sqlite3.h"
#include "common/rtu_log.h"

#define MAX_CONFIG_LEN      (5 * 1024 + 1)  //buf size for json file load
#define MAX_MSG_BUF_SIZE    1024            //max item buf size of message queue

#define MAX_NUM_METER_TYPE  10               //max number of meter type
#define PER_DAY_SECONDS  	86400UL

#define MAX_LED_NUM         6               //max led num
#define MAC_ADDR_LEN        6               //mac address len

#define RTU_MAX_READ_REGISTER            10000 //最大寄存器数
#define RTU_MAX_WELL_NUM                 9     //最大井口数(预留一个)
#define RTU_FLASH1_REGISTER_SIZE         300   //关键配置区寄存器个数

#define RTU_CONF_FILE                    "/usr/share/config.json"  //json file for config

enum {
	THREAD_TCP_SERVER		= 0,
	THREAD_TCP_CLIENT		= 1,
	THREAD_UDP_SERVER		= 2,
	THREAD_RS485_MASTER		= 3,
	THREAD_RS485_SLAVE		= 4,
	THREAD_RS232_SERVER		= 5,
	THREAD_RS232_ZIGBEE		= 6,
	THREAD_GPIO_LED			= 7,
	THREAD_GPIO_IRQ			= 8,
	THREAD_DATA_SERVER		= 9,
	THREAD_MAX_THREAD_NUM	= 10,
};

#if 1
#define THREAD_MAX_DELAY_TIME		(20 * 60)//20min
#define THREAD_MAX_COM_DELAY_TIME	(20 * 60)//20min

#else
#define THREAD_MAX_DELAY_TIME		30//30s
#define THREAD_MAX_COM_DELAY_TIME	30//30s

#endif

typedef struct Rtu_a118_t {
	char config_str[MAX_CONFIG_LEN];                    //配置缓冲区
	int zigbee_msg_fd;                                  //zigbee消息队列描述符
	char zigbee_msg_buf[MAX_MSG_BUF_SIZE];              //zigbee消息缓冲区
	int rtu_server_msg_fd;                              //rs485 slave
	char rtu_server_msg_buf[MAX_MSG_BUF_SIZE];
	int rtu_client_msg_fd;                              //rs485 master
	char rtu_client_msg_buf[MAX_MSG_BUF_SIZE];
	int tcp_server_msg_fd;
	char tcp_server_msg_buf[MAX_MSG_BUF_SIZE];
	int tcp_client_msg_fd;
	char tcp_client_msg_buf[MAX_MSG_BUF_SIZE];
	int data_msg_fd;                                    //数据存储消息队列描述符
	char data_msg_buf[MAX_MSG_BUF_SIZE];
	int udp_server_msg_fd;
	char udp_server_msg_buf[MAX_MSG_BUF_SIZE];
	int io_fd;                                          //普通io操作描述符
	int io_irq_fd;                                      //中断io操作描述符
	Map_t map;                                          //映射表
	time_t equ_meter_para_time[RTU_MAX_WELL_NUM][MAX_NUM_METER_TYPE];
	char led_status[MAX_LED_NUM];                       //灯状态, 0: zigbee, 1: upper computer, 2: run led
	char elec_param_meter_mac[RTU_MAX_WELL_NUM][10];
	uint16_t mac_addr[MAC_ADDR_LEN];
	char time_set_status;
	char flash1_reset_status;
	char flash1_reset_button;
	int adc_fd;                                         //adc 描述符, 用于电压获取
	time_t meter_sample_time[RTU_MAX_WELL_NUM][MAX_NUM_METER_TYPE];
	sqlite3 *db;                                        //数据库, sqlite3
	time_t thread_time[THREAD_MAX_THREAD_NUM];
	time_t thread_com_time[THREAD_MAX_THREAD_NUM];
}Rtu_a118_t;

extern Rtu_a118_t rtu_a118_t; //全局属性

/* 测试打印工具宏 */
#if 0
#define printf(format, arg...) fprintf(stdout, "%s %d: "format, __FUNCTION__, __LINE__, ##arg)
#define perror(format, arg...) \
		fprintf(stderr, "%s %d: "format" ERROR %d %s\n", __FUNCTION__, __LINE__, ##arg, errno, strerror(errno))
#endif

extern int config_load_json_file(char *buf, int len, const char *path);
extern int rtu_sys_info_init(Rtu_a118_t *self);
extern void update_thread_time(int id);
extern void update_thread_com_time(int id);

#define UPDATE_THREAD_TIME(id) time(rtu_a118_t.thread_time + id)

#endif /* CONFIG_H_ */
