/*
 * load_meter_analysis.h
 *
 *  Created on: Oct 23, 2014
 *      Author: caspar
 */

#ifndef LOAD_METER_ANALYSIS_H_
#define LOAD_METER_ANALYSIS_H_

#include "common/common.h"
#include "meter/meter.h"
#include "libmodbus-3.0.6/src/modbus-private.h"

//载荷请求包
typedef struct {
	st_a11_packet_header header; 	//帧头
	u8 comm_quality;       		//通信质量：xx%只对常规数据进行通信效率统计。初时值为100%
	u8 bat_value;         			//电池电量
	u16 collect_interval;    		//采集间隔0~1800秒，实际没用。
	u16 meter_satus;      			//仪表状态
	fp32 accele_value;  			//加速度，单位为g
	fp32 load_value;    			//载荷数据，单位为KN
}__attribute__ ((packed)) st_load_normal_packet;

typedef struct {
	st_a11_packet_header header; 	//帧头
	u16 sleep_time;					//休眠时间
}__attribute__ ((packed)) st_rtu_ack_load_normal_packet;

//主机请求载荷测量
typedef struct {
	st_a11_packet_header header; 	//帧头
	u8 data_mode;   				//OX0实际功图 0x1功图原始数据 0x10-仿真数据
	u16 sample_points;  			//功图采集的点数
	u16 synchron_time; 				//同步时间,时间单位为10ms
	u16 sample_interval;  			//功图点采集间隔
	u8 sample_time;      			//休眠采集间隔 min
	u8 moc[9];						//厂家自定义
}__attribute__ ((packed)) st_equ_measure_load_packet;

//载荷15包数据首包
typedef struct {
	st_a11_packet_header header; 	//帧头
	u8 packet_index;       		//顺序包号00
	u8 reserve[3];    			//预留字节
	u16 synchron_time;      		//同步时间
	u16 time_mark;        			//时间标记
	u16 sample_points;      		//功图点数
	u16 travel_length;      		//冲程 ,无符号整型,隐含3位小数传感器计算的冲程
	u16 cycle_time;          		//周期 ,传感器计算的冲程周期单位10ms
	u8 moc[20];					//预留20字节
}__attribute__ ((packed)) st_load_first_packet;

// 载荷数据包 后续包
typedef struct {
	st_a11_packet_header header; 		//帧头
	u8 packet_index;       			//顺序包号
	u8 reserve;       				//预留字节
	u16 pos_load[30];						//位移，载荷数据   总共15*2 个，60字节
}__attribute__ ((packed)) st_load_follow_up_packet;

// 载荷数据包 回复
typedef struct {
	st_a11_packet_header header; 		//帧头
	u8 packet_index;       			//顺序包号
}__attribute__ ((packed)) st_load_ack_packet;

#endif /* LOAD_METER_ANALYSIS_H_ */
