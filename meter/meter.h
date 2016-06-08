/*
 * meter.h
 *
 *  Created on: Oct 8, 2014
 *      Author: caspar
 */


#ifndef METER_H_
#define METER_H_
#include "config.h"

//内存地址
#define ELEC_WELL_NO_ADDR								(17)
#define ACTUATOR_WELL_NO_ADDR							(18)
//压力仪表地址
#define OIL_PRESSURE_BASE_ADDR							300
#define OIL_PRESSURE_SIZE								72
#define OIL_PRESSURE_RTDATA_ADDR						(OIL_PRESSURE_BASE_ADDR)
#define OIL_PRESSURE_METER_INFO_ADDR					(OIL_PRESSURE_BASE_ADDR + 2)
#define OIL_PRESSURE_INTERVAL_ADDR						(OIL_PRESSURE_BASE_ADDR + 22)
#define OIL_PRESSURE_TIME_ADDR							(OIL_PRESSURE_BASE_ADDR + 23)
#define OIL_PRESSURE_METER_PARA_ADDR					(OIL_PRESSURE_BASE_ADDR + 29)
//温度仪表地址
#define TEMPERATURE_BASE_ADDR							516
#define TEMPERATURE_SIZE								72
#define TEMPERATURE_RTDATA_ADDR							(TEMPERATURE_BASE_ADDR)
#define TEMPERATURE_METER_INFO_ADDR						(TEMPERATURE_BASE_ADDR + 2)
#define TEMPERATURE_INTERVAL_ADDR						(TEMPERATURE_BASE_ADDR + 22)
#define TEMPERATURE_TIME_ADDR							(TEMPERATURE_BASE_ADDR + 23)
#define TEMPERATURE_METER_PARA_ADDR						(TEMPERATURE_BASE_ADDR + 29)

//扭矩转速载荷一体仪表地址
#define STL_BASE_ADDR									588
#define STL_SIZE										76
#define STL_SPEED_RTDATA_ADDR							(STL_BASE_ADDR)
#define STL_TORQUE_RTDATA_ADDR							(STL_BASE_ADDR+2)
#define STL_LOAD_RTDATA_ADDR							(STL_BASE_ADDR+4)
#define STL_METER_INFO_ADDR								(STL_BASE_ADDR + 6)
#define STL_INTERVAL_ADDR								(STL_BASE_ADDR + 26)
#define STL_TIME_ADDR									(STL_BASE_ADDR + 27)
#define STL_METER_PARA_ADDR								(STL_BASE_ADDR + 33)

//载荷仪表地址
#define LOAD_BASE_ADDR									664
#define LOAD_SIZE										588
#define LOAD_LOAD_RTDATA_ADDR							(LOAD_BASE_ADDR)
#define LOAD_ACC_RTDATA_ADDR							(LOAD_BASE_ADDR + 2)
#define LOAD_METER_INFO_ADDR							(LOAD_BASE_ADDR + 4)
#define LOAD_INTERVAL_ADDR								(LOAD_BASE_ADDR + 24)
#define LOAD_DIAGRAM_ORDER_ADDR							(LOAD_BASE_ADDR + 25)
#define LOAD_SET_POINTS_ADDR							(LOAD_BASE_ADDR + 26)
#define LOAD_SAMPLE_POINTS_ADDR							(LOAD_BASE_ADDR + 27)
#define LOAD_SAMPLE_MODE_ADDR							(LOAD_BASE_ADDR + 28)
#define LOAD_TRAVEL_CNT_ADDR							(LOAD_BASE_ADDR + 29)
#define LOAD_TRAVEL_LEN_ADDR 							(LOAD_BASE_ADDR + 31)
#define LOAD_CYCLE_POS_ADDR								(LOAD_BASE_ADDR + 33)
#define LOAD_CYCLE_LOAD_ADDR							(LOAD_BASE_ADDR + 283)
#define LOAD_TIME_ADDR									(LOAD_BASE_ADDR + 533)
#define LOAD_CYCLE_TIME_ADDR							(LOAD_BASE_ADDR + 539)
#define LOAD_METER_PARA_ADDR							(LOAD_BASE_ADDR + 545)

//电参仪表地址
#define ELEC_BASE_ADDR									1252
#define ELEC_SIZE										602
#define ELEC_CURRENT_RTDATA_ADDR						(ELEC_BASE_ADDR)
#define ELEC_VOLTAGE_RTDATA_ADDR						(ELEC_BASE_ADDR + 6)
#define ELEC_ACTIVE_POWER_RTDATA_ADDR					(ELEC_BASE_ADDR + 12)
#define ELEC_REACTIVE_POWER_RTDATA_ADDR					(ELEC_BASE_ADDR + 14)
#define ELEC_ACTIVE_POWER_RATE_RTDATA_ADDR				(ELEC_BASE_ADDR + 16)
#define ELEC_REACTIVE_POWER_RATE_RTDATA_ADDR			(ELEC_BASE_ADDR + 18)
#define ELEC_REVERSE_POWER_RATE_RTDATA_ADDR				(ELEC_BASE_ADDR + 20)
#define ELEC_POWER_FACTOR_RTDATA_ADDR					(ELEC_BASE_ADDR + 22)
#define ELEC_CYCLE_CURRENT_ADDR							(ELEC_BASE_ADDR + 24)
#define ELEC_CYCLE_POWER_ADDR							(ELEC_BASE_ADDR + 274)
#define ELEC_METER_INFO_ADDR							(ELEC_BASE_ADDR + 524)
#define ELEC_INTERVAL_ADDR								(ELEC_BASE_ADDR + 544)
#define ELEC_VOLTAGE_RATE_ADDR							(ELEC_BASE_ADDR + 545)
#define ELEC_CURRENT_RATE_ADDR							(ELEC_BASE_ADDR + 546)
#define ELEC_TIME_ADDR									(ELEC_BASE_ADDR + 547)
#define ELEC_CYCLE_TIME_ADDR							(ELEC_BASE_ADDR + 553)
#define ELEC_METER_PARA_ADDR							(ELEC_BASE_ADDR + 559)



//执行器仪表地址
#define ACTUATOR_BASE_ADDR								1870
#define ACTUATOR_SIZE									31
#define ACTUATOR_INTERVAL_ADDR							(ACTUATOR_BASE_ADDR)
#define ACTUATOR_INJEC_PERDAY_ADDR						(ACTUATOR_BASE_ADDR + 1)
#define ACTUATOR_ADJUST_ACCURENT_ADDR					(ACTUATOR_BASE_ADDR + 3)
#define ACTUATOR_AUTO_FLAG_ADDR							(ACTUATOR_BASE_ADDR + 4)
#define ACTUATOR_TOTAL_FLOW_ADDR						(ACTUATOR_BASE_ADDR + 5)
#define ACTUATOR_FLOW_RATE_ADDR							(ACTUATOR_BASE_ADDR + 7)
#define ACTUATOR_FLOWER_STATE_ADDR						(ACTUATOR_BASE_ADDR + 9)
#define ACTUATOR_ANGLE_ADDR								(ACTUATOR_BASE_ADDR + 10)
#define ACTUATOR_ADJUST_CNT_ADDR						(ACTUATOR_BASE_ADDR + 11)
#define ACTUATOR_ADJUST_MODE_ADDR						(ACTUATOR_BASE_ADDR + 12)
#define ACTUATOR_FLOWER_MODEL_ADDR						(ACTUATOR_BASE_ADDR + 13)
#define ACTUATOR_SOFTWARE_VERSION_ADDR					(ACTUATOR_BASE_ADDR + 14)
#define ACTUATOR_HARDWARE_VERSION_ADDR					(ACTUATOR_BASE_ADDR + 15)
#define ACTUATOR2_BASE_ADDR								1885
#define ACTUATOR2_INJEC_PERDAY_ADDR						(ACTUATOR2_BASE_ADDR + 1)
#define ACTUATOR2_ADJUST_ACCURENT_ADDR					(ACTUATOR2_BASE_ADDR + 3)
#define ACTUATOR2_AUTO_FLAG_ADDR						(ACTUATOR2_BASE_ADDR + 4)
#define ACTUATOR2_TOTAL_FLOW_ADDR						(ACTUATOR2_BASE_ADDR + 5)
#define ACTUATOR2_FLOW_RATE_ADDR						(ACTUATOR2_BASE_ADDR + 7)
#define ACTUATOR2_FLOWER_STATE_ADDR						(ACTUATOR2_BASE_ADDR + 9)
#define ACTUATOR2_ANGLE_ADDR							(ACTUATOR2_BASE_ADDR + 10)
#define ACTUATOR2_ADJUST_CNT_ADDR						(ACTUATOR2_BASE_ADDR + 11)
#define ACTUATOR2_ADJUST_MODE_ADDR						(ACTUATOR2_BASE_ADDR + 12)
#define ACTUATOR2_FLOWER_MODEL_ADDR						(ACTUATOR2_BASE_ADDR + 13)
#define ACTUATOR2_SOFTWARE_VERSION_ADDR					(ACTUATOR2_BASE_ADDR + 14)
#define ACTUATOR2_HARDWARE_VERSION_ADDR					(ACTUATOR2_BASE_ADDR + 15)

//协议类型
#define PROTOCOL_TYPE 									0x0000
//厂家代码
#define VENDER_CODE_BJAK 								0x0001
#define VENDER_CODE_BJBZJH 								0x0002
#define VENDER_CODE_AHBBRY 								0x0003
#define VENDER_CODE_GZHTKS 								0x0004
#define VENDER_CODE_BJZHYX 								0x0005
#define VENDER_CODE_TJWZ 								0x0006
#define VENDER_CODE_HTKGGX 								0x0007
#define VENDER_CODE_HBCY 								0x0008
#define VENDER_CODE_CQYT 								0x0009
#define VENDER_CODE_RESERVED 							0x8001
//仪表类型
#define METER_TYPE_WL_INTERGRATION_LOAD 				0x0001
#define METER_TYPE_WL_PRESSURE			 				0x0002
#define METER_TYPE_WL_TEMPERATURE		 				0x0003
#define METER_TYPE_WL_ELECTRICITY		 				0x0004
#define METER_TYPE_WL_ANGULAR_ROTATION	 				0x0005
#define METER_TYPE_WL_LOAD 								0x0006
#define METER_TYPE_WL_TORQUE							0x0007
#define METER_TYPE_WL_DYNAMIC_LIQUID_LEVEL				0x0008
#define METER_TYPE_WL_INTERGRATION_SPEED_TORQUE 		0x0009
#define METER_TYPE_WL_FLOW_METER		 				0x000A
#define METER_TYPE_WL_RTU 								0x1F10
#define METER_TYPE_WL_MANIPULATOR 						0x1F11
#define METER_TYPE_RESERVED 							0x2000
//数据类型
#define DATA_TYPE_ROUTINE_DATA 							0x0000
#define DATA_TYPE_METER_PARA 							0x0010
#define DATA_TYPE_INTERGRATION_LOAD_DIAGRAM				0x0020
#define DATA_TYPE_WL_LOAD_READ_DIAGRAM					0x0021
#define DATA_TYPE_WL_ANGULAR_ROTATION_READ_DIAGRAM		0x0022
#define DATA_TYPE_ELECTRICITY_DIAGRAM					0x0030
#define DATA_TYPE_SPECIAL_DATA							0x0080
#define DATA_TYPE_RTU_ACK_ROUTINE						0x0100
#define DATA_TYPE_RTU_EQU_METER_PARA					0x0101
#define DATA_TYPE_RTU_EQU_INTERGRATION_DIAGRAM_DATA		0x0200
#define DATA_TYPE_RTU_ACK_INTERGRATION_DIAGRAM_DATA		0x0201
#define DATA_TYPE_RTU_EQU_INTERGRATION_RECORD_DIAGRAM	0x0202
#define DATA_TYPE_RTU_EQU_WL_LOAD_DIAGRAM_DATA			0x0204
#define DATA_TYPE_RTU_ACK_WL_LOAD_DIAGRAM_DATA			0x0205
#define DATA_TYPE_RTU_EQU_WL_LOAD_RECORD_DIAGRAM		0x0206
#define DATA_TYPE_RTU_EQU_WL_ROTATION_DIAGRAM_DATA		0x0207
#define DATA_TYPE_RTU_ACK_WL_ROTATION_DIAGRAM_DATA		0x0208
#define DATA_TYPE_RTU_EQU_WL_ROTATION_RECORD_DIAGRAM	0x0209
#define DATA_TYPE_RTU_EQU_WL_ELECTRICITY_DIAGRAM_DATA	0x0210
#define DATA_TYPE_RTU_ACK_WL_ELECTRICITY_DIAGRAM_DATA	0x0211
#define DATA_TYPE_RTU_EQU_WL_ELECTRICITY_RECORD_DIAGRAM	0x0212
#define DATA_TYPE_RTU_EQU_SPECIAL_DIAGRAM_DATA			0x0230
#define DATA_TYPE_RTU_ACK_SPECIAL_DIAGRAM_DATA			0x0231
#define DATA_TYPE_RTU_EQU_SPECIAL_RECORD_DIAGRAM		0x0232
#define DATA_TYPE_RTU_CONTROL_METER						0x0300

#define MAX_WELL_NUM									(RTU_MAX_WELL_NUM-1)

#define OIL_PRESSURE_NO									0x00
#define CASING_PRESSURE_NO								0x01
#define BACK_PRESSURE_NO								0x02
#define TEMPERATURE_NO									0x03
#define LOAD_NO											0x04
#define ELECTRICAL_PARAMETER_NO							0x05
#define STL_NO											0x06
#define ACTUATOR_NO1									0x07
#define ACTUATOR_NO2									0x08
#define WL_FLOW_METER_NO								0x09

#define CLEAR_RTDATA_CNT								10


//压力与温度
typedef struct {
	u16 protocol_type;    	//协议类型
	u16 vender_no;      	//厂商代码
	u16 sensor_type;  		//仪表类型：压力变送器默认为0x0002
	u8 sensor_group;        //仪表组号
	u8 sensor_num;			//仪表编号
	u16 data_typed;			//命令类型 数据类型

	u8 comm_quality;       	//通信质量：xx%只对常规数据进行通信效率统计。初时值为100%
	u8 bat_value;         	//电池电量
	u16 collect_interval;   //采集间隔0~1800秒，实际没用。
	u16 meter_satus;      	//仪表状态
	fp32 sensor_valve;  	//实时数据
} __attribute__ ((packed)) st_sensor_normal_packet;

typedef struct {
	u16 protocol_type;    	//协议类型
	u16 vender_no;      	//厂商代码
	u16 sensor_type;  		//仪表类型：压力变送器默认为0x0002
	u8 sensor_group;        //仪表组号
	u8 sensor_num;			//仪表编号
	u16 data_typed;			//数据类型
	u16 sleep_time;			//休眠时间
} __attribute__ ((packed)) st_rtu_ack_sensor_normal_packet;

typedef struct {
	u16 protocol_type;    	//协议类型
	u16 vender_no;      	//厂商代码
	u16 sensor_type;  		//仪表类型
	u8 sensor_group;        //仪表组号
	u8 sensor_num;			//仪表编号
	u16 data_typed;			//命令类型 数据类型
} __attribute__ ((packed)) st_a11_packet_header;

typedef struct {
	st_a11_packet_header header; //帧头
	//u16 data_typed;
	u8 typed_code[8];
	u8 serial_number[16];
	u16 hardware_version;
	u16 sorftware_version;
	u16 range_down;
	u16 range_up;
	u16 precision;
	u8 bakup[12];
	u8 protect_level[8];
	u8 anti_level[12];
	u8 meter_explain[10];
} __attribute__ ((packed)) st_meter_param;

//一体化扭矩载荷转速
typedef struct {
	u16 protocol_type;    	//协议类型
	u16 vender_no;      	//厂商代码
	u16 sensor_type;  		//仪表类型：压力变送器默认为0x0002
	u8 sensor_group;        //仪表组号
	u8 sensor_num;			//仪表编号
	u16 data_typed;			//命令类型 数据类型

	u8 comm_quality;       	//通信质量：xx%只对常规数据进行通信效率统计。初时值为100%
	u8 bat_value;         	//电池电量
	u16 collect_interval;   //采集间隔0~1800秒，实际没用。
	u16 meter_satus;      	//仪表状态
	fp32 torque_valve;  	//实时数据 扭矩
	fp32 load_valve;  		//实时数据 载荷
	fp32 speed_valve;  		//实时数据 转速
} __attribute__ ((packed)) st_torque_speed_load_normal_packet;

//无线流量计
typedef struct {
	u16 protocol_type;    	//协议类型
	u16 vender_no;      	//厂商代码
	u16 sensor_type;  		//仪表类型：压力变送器默认为0x0002
	u8 sensor_group;        //仪表组号
	u8 sensor_num;			//仪表编号
	u16 data_typed;			//命令类型 数据类型

	u8 comm_quality;       	//通信质量：xx%只对常规数据进行通信效率统计。初时值为100%
	u8 bat_value;         	//电池电量
	u16 collect_interval;   //采集间隔0~1800秒，实际没用。
	u16 meter_satus;      	//仪表状态
	fp32 total_flow_valve;  	//实时数据
	fp32 current_flow_valve;  	//实时数据
} __attribute__ ((packed)) st_sensor_flow_packet;

void meter_param_init(void);
void packet_a11_rtu_header(u8* a11_header, u8 group, u8 no,u16 data_type);
void save_time_register(u16 well_no, u16 addr);
void save_meter_info(u8* a11_header, u16 well_no, u16 addr);
int8 judge_eq_meter_para(int16 well_no,u8 meter_type);
void clear_meter_rtdata(void);
extern int8 analysis_load(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
extern int8 analysis_electrical_param(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));

extern int time_handle_fun(modbus_t *ctx, uint16_t *syn_along_info);
extern int deal_wired_actuator(modbus_t* ctx);

#endif /* METER_H_ */
