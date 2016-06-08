/*
 * pmsg_q.h
 *
 *  Created on: Oct 14, 2014
 *      Author: ygz
 */

#ifndef PMSG_Q_H_
#define PMSG_Q_H_

/* 消息队列路径 */
#define ZIGBEE_MSG_PATH       "/zigbee"
#define RTU_SERVER_MSG_PATH   "/rtu_server"
#define RTU_CLIENT_MSG_PATH   "/rtu_client"
#define TCP_SERVER_MSG_PATH   "/tcp_server"
#define TCP_CLIENT_MSG_PATH   "/tcp_client"
#define DATA_MSG_PATH         "/data"
#define UDP_SERVER_MSG_PATH   "/udp_server"

/* 消息类型 */
enum {
	MSG_NULL,               //空类型
	MSG_CONFIG_START,       //开始配置, 30寄存器写1
	MSG_NETWORK_SET_IP,
	MSG_CONFIG2_START,      //flash2写配置
	MSG_REQUEST_H_DATA,     //请求历史数据
	MSG_LOAD_ELEC_SYSN,     //caspar, 仪表同步
};
/* 消息队列消息结构(未使用) */
typedef struct MsgData {
//  long int type;
	int cmd;               //消息类型
	int len;               //消息长度
//	body; //数据体
}__attribute__ ((packed)) MsgData;

extern int pmsg_q_open(const char *path);
extern int pmsg_q_close(int mfd);
extern int pmsg_q_rm(const char *path);
extern int pmsg_q_receive(int mfd, void *msg, size_t len, unsigned int *prio);
extern int pmsg_q_send(int mfd, void *msg, size_t len, unsigned int prio);


#endif /* PMSG_Q_H_ */
