/*
 * wired_actuator.h
 *
 *  Created on: Nov 18, 2014
 *      Author: caspar
 */

#ifndef WIRED_ACTUATOR_H_
#define WIRED_ACTUATOR_H_

#include "wired_meter.h"

#define RESEND_ACT_NUM					3

//RTU 下发时间
typedef struct
{
	u8	header; 	//头
	u8 	addr; 		//设备号
	u8 	fun_code; 	//功能码  01读取执行器的值，并下发时间
	u32 date_time; 	//日期时间---1970年的秒
	u8 	reserved[6];	//预留
	u16 checksum;		//和校验
	u8 	eof; //结束符
}__attribute__ ((packed)) st_set_actuator_time;

//下发时间回复 采集数据
typedef struct
{
	u8 	header;		//头
	u8 	addr;		//地址
	u8 	fun_code;	//功能码
	u16 len;		//数据域长度
	u64 total_flow;	//总流量---升
	u32 flow_rate;	//流速---升/小时
	u8 	adjust_times_oneway;	//单向连续调节次数
	u8 	flowmeter_comm_status;	//流量计通信状态
	u8 	flowmeter_type;			//流量计型号 1-一诺 2-驸马  3-大港1 4-大港2 5-蓝天 6-蓝天modbus (sDISSETPAGE.flower_modle+1)
	u8 	current_argle;			//当前角度
	u8 	version[4];				//版本号
	u16 adjust_times_total;		//当前累计调节次数
	u8 	auto_flag;				//自动调节标志
	u8 	valve_off_flag;			//阀门关闭状态标志 这个是状态而不是设置命令 1--关闭  0--调节
	u8 	adjust_angle_oneway;	//单向连续调节角度
	u32 sys_clock;				//系统时钟
	u8 	un_use;
	u16 checkdsum;
	u8 	eof1;					//结束符
	u8 	eof2;					//结束符
}__attribute__ ((packed)) st_actuator_ack_rtu;

//RTU下发参数
typedef struct {
	u8 	header; 	//头
	u8 	addr; 		//地址
	u8 	fun_code; 	//02 下发参数 ，
	u32 injec_alloc_perday;	//日配注---升
	u8 	adjust_accuracy_percent;	//调节精度---百分比
	u8 	auto_flag; 	//自动标志
	u8 	start_time_day;		//当日流量起始计算时间 一天中的几点钟
	u16 checksum;
	u8 	eof; 		//结束符
}__attribute__ ((packed)) st_set_actuator_para;

//下发参数回复
typedef struct
{
	u8 	header;		//头
	u8 	addr;		//地址
	u8 	fun_code;	//功能码
	u16 len;		//数据域长度
	u8	set_status;
	u16 checksum;
	u8 	eof1;					//结束符
	u8 	eof2;					//结束符
}__attribute__ ((packed)) st_actuator_para_ack_rtu;

extern int deal_wired_actuator(modbus_t* ctx);

#endif /* WIRED_ACTUATOR_H_ */
