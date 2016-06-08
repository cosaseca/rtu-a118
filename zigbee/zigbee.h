/*
 * zigbee.h
 *
 *  Created on: Sep 27, 2014
 *      Author: caspar
 */

#ifndef ZIGBEE_H_
#define ZIGBEE_H_

#include <stdio.h>
#include <string.h>
#include "common/common.h"

#include "meter/pressure_meter_analysis.h"

#include "libmodbus-3.0.6/src/modbus-private.h"

#define ZIGBEE_API_HEAD 					0x7E
#define ZIGBEE_API_FRAMID1					0x00
#define ZIGBEE_API_NETADDR0					0xFF
#define ZIGBEE_API_NETADDR1					0xFE
#define ZIGBEE_API_TRANSRC					0xE8
#define ZIGBEE_API_TRANDES					0xE8
#define ZIGBEE_API_CLUSTERID0				0x00
#define ZIGBEE_API_CLUSTERID1				0x11
#define ZIGBEE_API_PROFILEID0				0x18
#define ZIGBEE_API_PROFILEID1				0x57
#define ZIGBEE_API_BROADCASTRAD				0x00
#define ZIGBEE_API_SENDOPTION				0x60

#define ZIGBEE_SET_S2B_FRAMID 				0x08
#define ZIGBEE_SET_S2C_FRAMID 				0x09
#define ZIGBEE_SET_ACK_FRAMID 				0x88
#define ZIGBEE_RCV_DATA_FRAMID 				0x91
#define ZIGBEE_SEND_DATA_FRAMID				0x11

#define A11_ZIGBEE_RCV_DATA_LEN				(80)
#define A11_ZIGBEE_SND_DATA_LEN				(80)
#define ZIGBEE_SND_PACKET_LEN				(20)

#define ZIGBEE_OK							0x01
#define ZIGBEE_FAIL							0x00

typedef struct st_zigbee_para_option{
	u32 panid;
	u16 auto_or_handle_flag;
	u16 ch;
	u16 enable_or_disable_password_flag;
	u64 key;
} __attribute__ ((packed)) st_zigbee_para_option;

typedef struct st_zigbee_send_fram{
	u8 header;
	u8 len[2];
	u8 fram_id;
	u8 fram_id_flag;
	u8 id_name[2];
	u8 data[8];
	u8 check_sum;
} __attribute__ ((packed)) st_zigbee_send_fram;

typedef struct st_zighee_api_rx_packet{
	u8 rx_api_head;						//协议头
	u8 rx_api_data_length[2];			//总长度
	u8 rx_api_fram_id;					//帧ID，0x91,接收
	u8 rx_api_mac_addr[10];				//8个mac地址2个网络地址
	u8 rx_api_expand[7];				//协议规定
	u8 rx_api_data[A11_ZIGBEE_RCV_DATA_LEN];	//字节数据
	u8 rx_api_check_sum;				//校验和
} __attribute__ ((packed)) st_zighee_api_rx_packet;

typedef struct
{
	u8 tx_api_head;								//协议头0x7e
	u8 tx_api_data_length[2];					//总长度
	u8 tx_api_fram_id[2];						//帧ID，0x10,0x01,表示发送
	u8 tx_api_mac_addr[10];						//8个mac地址2个网络地址
	u8 tx_api_expand[8];						//协议规定，0x00,0x00
	u8 tx_api_data[A11_ZIGBEE_SND_DATA_LEN];	//字节数据
	u8 tx_api_check_sum;						//校验和
}__attribute__ ((packed)) st_zigbee_api_tx_packet;

u8 calc_sum_check(u8* p_order_frame);
void set_zigbee_para(modbus_t* ctx, u8 *p_zigbee_para);
int8 analysis_zigbee_rcv_data(modbus_t* ctx, u8* rec);
int8 zigbee_send_packet(modbus_t* ctx, u8* mac, u8* send_data, u8 len);

extern void *thread_zigbee_tty_server(void *arg);
extern void send_sysn_electrical_param(modbus_t *ctx, u16 *syn_along_info);
extern int8 zigbee_send_packet(modbus_t* ctx, u8* mac, u8* send_data, u8 len);

#endif /* ZIGBEE_H_ */

