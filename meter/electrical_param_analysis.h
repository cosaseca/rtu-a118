/*
 * electrical_param_analysis.h
 *
 *  Created on: Nov 7, 2014
 *      Author: caspar
 */

#ifndef ELECTRICAL_PARAM_ANALYSIS_H_
#define ELECTRICAL_PARAM_ANALYSIS_H_

#include "common/common.h"
#include "meter/meter.h"
#include "libmodbus-3.0.6/src/modbus-private.h"

typedef struct {
	st_a11_packet_header header; 	//帧头
	u8 comm_quality;       		//通信质量：xx%只对常规数据进行通信效率统计。初时值为100%
	u8 bat_value;         		//电池电量
	u16 sleep_time;    			//采集间隔0~1800秒，实际没用。
	u16 meter_satus;      		//仪表状态
	fp32 ia;  //A相电流
	fp32 ib;  //B相电流
	fp32 ic;  //C相电流
	fp32 va;  //A相电压
	fp32 vb;  //A相电压
	fp32 vc;  //A相电压
	fp32 power_factor;  		//功率因数
	fp32 active_power;  		//有功功率
	fp32 reactive_power;  		//无功功率
	fp32 quantity_elect;		//总电量
	fp32 reserve;				//保留
}__attribute__ ((packed)) st_elect_normal_packet;

typedef struct {
	st_a11_packet_header header; 	//帧头
	u16 sleep_time;					//休眠时间
}__attribute__ ((packed)) st_rtu_ack_elect_normal_packet;

typedef struct {
	st_a11_packet_header header;//帧头
	u8 current_chart_mode;    	//功图模式
	u16 sample_points;     		//点数
	u16 synchron_time;    		//同步时间
	u16 sample_interval;   		//点与点之间的时间 ms
	u8 arithmetic;    			//算法
	u8 reserve[10];
}__attribute__ ((packed)) st_rtu_equ_elect_measure_packet;

typedef struct {
	st_a11_packet_header header; //帧头
	u8 packet_index;       	//顺序包号00
	u8 reserve;    			//预留字节
	u8 diagram_mode;      		//功图模式
	u8 arithmetic;        		//算法
	u16 synchron_time;      	//同步时间
	u16 time_mark;      		//时间标记
	u16 sample_points;          //采样点数 与功图点数相同
}__attribute__ ((packed)) st_elect_first_packet;


typedef struct
{
	st_a11_packet_header header; 	//帧头
    u8  packet_index;       		//顺序包号
    u8  reserve;       				//预留字节
    u16 rcv_data[30];				//载荷数据    30 个，60字节
}__attribute__ ((packed)) st_elect_follow_up_packet;

// 电参数据包 回复
typedef struct {
	st_a11_packet_header header; 		//帧头
	u8 packet_index;       			//顺序包号
}__attribute__ ((packed)) st_elect_ack_packet;

extern int8 analysis_electrical_param(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
extern void send_sysn_electrical_param(modbus_t *ctx, u16 *syn_along_info);

#endif /* ELECTRICAL_PARAM_ANALYSIS_H_ */
