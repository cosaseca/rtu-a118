/*
 * pressure_meter_analysis.c
 *
 *  Created on: Oct 8, 2014
 *      Author: caspar
 */

#include "pressure_meter_analysis.h"
#include "data/data.h"



/*********************************************************************
* @fn     	analysis_pressure
*
* @brief  	解析有关压力数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_mac 需要回复的mac地址；
* 			int8 (*func)(modbus_t*, u8*, u8*, u8) 回调发送函数
*
* @return 	-1:非本仪表数据
* 			0：仪表数据格式或者组号编号有误
* 			1：正常完成解析
*/
int8 analysis_pressure(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	float tmp_meter_val = 0.0;
	u16 tmp_sleep_time = 10;
	u8 tmp_buff[160];
	st_sensor_normal_packet *sensor_normal_data =
			(st_sensor_normal_packet *) p_buff;
	st_rtu_ack_sensor_normal_packet *send_reply_packet =
			(st_rtu_ack_sensor_normal_packet*) tmp_buff;

	if ((sensor_normal_data->sensor_type != swap_int16(METER_TYPE_WL_PRESSURE))
			|| (sensor_normal_data->protocol_type != swap_int16(PROTOCOL_TYPE))) {
		return -1;
	}
	if (sensor_normal_data->data_typed == swap_int16(DATA_TYPE_ROUTINE_DATA)) {//解析常规数据
		if (!((sensor_normal_data->sensor_group > 0)
				&& (sensor_normal_data->sensor_group < MAX_WELL_NUM + 1))) {
			return 0;
		}
		if ((sensor_normal_data->sensor_num == 0)
				|| (sensor_normal_data->sensor_num > 3)) {
			return 0;
		}
		if (-1 == judge_eq_meter_para(sensor_normal_data->sensor_group,sensor_normal_data->sensor_num-1)) {//仪表常规数据正常回复
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_ACK_ROUTINE);
			zigbee_read_registers((u16 *) &tmp_sleep_time, 1,
					sensor_normal_data->sensor_group,
					(OIL_PRESSURE_INTERVAL_ADDR + (sensor_normal_data->sensor_num-1)*OIL_PRESSURE_SIZE));// 1150
			if(tmp_sleep_time > 1800){//容错
				tmp_sleep_time = 1800;
			}

			send_reply_packet->sleep_time = swap_int16(tmp_sleep_time);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 12);
		} else {//判断是否请求仪表参数
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 10);
		}
		//保存仪表信息
		save_meter_info((u8 *) &sensor_normal_data->protocol_type,
				sensor_normal_data->sensor_group,
				(OIL_PRESSURE_METER_INFO_ADDR + (sensor_normal_data->sensor_num - 1) * OIL_PRESSURE_SIZE)); // 1300
		time(&rtu_a118_t.meter_sample_time[sensor_normal_data->sensor_group - 1][sensor_normal_data->sensor_num - 1]);
		//保存仪表数据
		tmp_meter_val = swap_rcv_float_seq(sensor_normal_data->sensor_valve);
		//printf("tmp_meter_val = %f",tmp_meter_val);
		tmp_meter_val = swap_stor_float_seq(tmp_meter_val);
		zigbee_write_registers((u16*) &tmp_meter_val, 2,
				sensor_normal_data->sensor_group,
				(OIL_PRESSURE_RTDATA_ADDR + (sensor_normal_data->sensor_num - 1) * OIL_PRESSURE_SIZE));//保存实时数据0
		save_time_register(sensor_normal_data->sensor_group,
				(OIL_PRESSURE_TIME_ADDR + (sensor_normal_data->sensor_num - 1) * OIL_PRESSURE_SIZE));//保存采集时间 1200

	}
	else if (sensor_normal_data->data_typed
			== swap_int16(DATA_TYPE_METER_PARA)) {//仪表参数
		st_meter_param *sensor_param = (st_meter_param *) p_buff;
		if (!((sensor_param->header.sensor_group > 0)
				&& (sensor_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return 0;
		}
		if ((sensor_normal_data->sensor_num == 0)
				|| (sensor_normal_data->sensor_num > 3)) {
			return 0;
		}
		//保存仪表参数
		zigbee_write_registers((u16*) &sensor_param,
				sizeof(st_meter_param) / 2, sensor_param->header.sensor_group,
				(OIL_PRESSURE_METER_PARA_ADDR  + (sensor_normal_data->sensor_num - 1)
								* OIL_PRESSURE_SIZE));// 保存仪表参数1500
	} else {
		return 0;
	}
	return 1;
}


/*********************************************************************
* @fn     	analysis_temperature
*
* @brief  	解析有关温度数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_mac 需要回复的mac地址；
* 			int8 (*func)(modbus_t*, u8*, u8*, u8) 回调发送函数
*
* @return 	-1:非本仪表数据
* 			0：仪表数据格式或者组号编号有误
* 			1：正常完成解析
*/
int8 analysis_temperature(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	float tmp_meter_val = 0.0;
	u16 tmp_sleep_time = 10;
	u8 tmp_buff[160];
	st_sensor_normal_packet *sensor_normal_data =
			(st_sensor_normal_packet *) p_buff;
	st_rtu_ack_sensor_normal_packet *send_reply_packet =
			(st_rtu_ack_sensor_normal_packet*) tmp_buff;
	//判断非温度仪表数据
	if ((sensor_normal_data->sensor_type
			!= swap_int16(METER_TYPE_WL_TEMPERATURE))
			|| (sensor_normal_data->protocol_type != swap_int16(PROTOCOL_TYPE))) {
		return -1;
	}
	if (sensor_normal_data->data_typed == swap_int16(DATA_TYPE_ROUTINE_DATA)) {//温度常规数据
		if (-1 == judge_eq_meter_para(sensor_normal_data->sensor_group,TEMPERATURE_NO)) {//仪表常规数据正常回复
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_ACK_ROUTINE);
			zigbee_read_registers((u16 *) &tmp_sleep_time, 1,
					sensor_normal_data->sensor_group, TEMPERATURE_INTERVAL_ADDR);// 温度采集间隔 1153
			if(tmp_sleep_time > 1800){
				tmp_sleep_time = 1800;
			}
			send_reply_packet->sleep_time = swap_int16(tmp_sleep_time);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 12);
		} else {//请求仪表参数
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 10);
		}
		if (!((sensor_normal_data->sensor_group > 0)
				&& (sensor_normal_data->sensor_group < MAX_WELL_NUM + 1))) {
			return -1;
		}
		if (sensor_normal_data->sensor_num != 1) {
			return -1;
		}
		time(&rtu_a118_t.meter_sample_time[sensor_normal_data->sensor_group - 1][TEMPERATURE_NO]);
		//保存温度仪表信息
		save_meter_info((u8 *) &sensor_normal_data->protocol_type,
				sensor_normal_data->sensor_group, TEMPERATURE_METER_INFO_ADDR);// 仪表信息 1360
		tmp_meter_val = swap_rcv_float_seq(sensor_normal_data->sensor_valve);
		tmp_meter_val = swap_stor_float_seq(tmp_meter_val);
		//保存温度实时数据
		zigbee_write_registers((u16*) &tmp_meter_val, 2,
				sensor_normal_data->sensor_group, TEMPERATURE_RTDATA_ADDR);// 6井口油温值
		//保存温度采集时间
		save_time_register(sensor_normal_data->sensor_group,TEMPERATURE_TIME_ADDR );//1218

	} else if (sensor_normal_data->data_typed
			== swap_int16(DATA_TYPE_METER_PARA)) {//仪表参数
		st_meter_param *sensor_param = (st_meter_param *) p_buff;
		if (!((sensor_param->header.sensor_group > 0)
				&& (sensor_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return -1;
		}
		//保存温度仪表参数
		zigbee_write_registers((u16*) &sensor_param,
				sizeof(st_meter_param) / 2, sensor_param->header.sensor_group,
				TEMPERATURE_METER_PARA_ADDR );// 1611温度仪表参数
	} else {
		return -1;
	}
	return 0;
}


/*********************************************************************
* @fn     	analysis_flow_meter
*
* @brief  	解析有关流量计数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_mac 需要回复的mac地址；
* 			int8 (*func)(modbus_t*, u8*, u8*, u8) 回调发送函数
*
* @return 	-1:非本仪表数据
* 			0：仪表数据格式或者组号编号有误
* 			1：正常完成解析
*/
int8 analysis_flow_meter(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	float tmp_meter_val = 0.0;
	u16 tmp_sleep_time = 10;
	u8 tmp_buff[160];
	st_sensor_flow_packet *sensor_normal_data =
			(st_sensor_flow_packet *) p_buff;
	st_rtu_ack_sensor_normal_packet *send_reply_packet =
			(st_rtu_ack_sensor_normal_packet*) tmp_buff;
	//判断非流量计数据
	if ((sensor_normal_data->sensor_type
			!= swap_int16(METER_TYPE_WL_FLOW_METER))
			|| (sensor_normal_data->protocol_type != swap_int16(PROTOCOL_TYPE))) {
		return -1;
	}
	if (sensor_normal_data->data_typed == swap_int16(DATA_TYPE_ROUTINE_DATA)) {//流量计常规数据
		if (-1 == judge_eq_meter_para(sensor_normal_data->sensor_group,WL_FLOW_METER_NO)) {//仪表常规数据正常回复
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_ACK_ROUTINE);
			zigbee_read_registers((u16 *) &tmp_sleep_time, 1,
					sensor_normal_data->sensor_group, ACTUATOR_INTERVAL_ADDR);// 流量计（与执行器共用）采集间隔 1153
			if(tmp_sleep_time > 1800){
				tmp_sleep_time = 1800;
			}
			send_reply_packet->sleep_time = swap_int16(tmp_sleep_time);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 12);
            printf("flow normal data\n");
		} else {//请求仪表参数
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 10);
            printf("flow arg data\n");
		}
		if (!((sensor_normal_data->sensor_group > 0)
				&& (sensor_normal_data->sensor_group < MAX_WELL_NUM + 1))) {
            printf("flow sensor_group error %d\n", sensor_normal_data->sensor_group);
			return -1;
		}
		if (sensor_normal_data->sensor_num != 1 && sensor_normal_data->sensor_num != 2) {
            printf("flow sensor_num error %d\n", sensor_normal_data->sensor_num);
			return -1;
		}
		time(&rtu_a118_t.meter_sample_time[sensor_normal_data->sensor_group - 1][WL_FLOW_METER_NO]);
		//保存流量计仪表信息
//		save_meter_info((u8 *) &sensor_normal_data->protocol_type,
//				sensor_normal_data->sensor_group, TEMPERATURE_METER_INFO_ADDR);// 仪表信息 1360
		tmp_meter_val = swap_rcv_float_seq(sensor_normal_data->total_flow_valve);
		tmp_meter_val = swap_stor_float_seq(tmp_meter_val);
		//保存流量计累计流量实时数据
        printf("store flow total data\n");
		zigbee_write_registers((u16*) &tmp_meter_val, 2,
				sensor_normal_data->sensor_group, ACTUATOR_TOTAL_FLOW_ADDR+ 15 *(sensor_normal_data->sensor_num - 1));// 6井口油温值

		tmp_meter_val = swap_rcv_float_seq(
				sensor_normal_data->current_flow_valve);
		tmp_meter_val = swap_stor_float_seq(tmp_meter_val);
		//保存流量计瞬时流量实时数据
        printf("store flow cur data\n");
		zigbee_write_registers((u16*) &tmp_meter_val, 2,
				sensor_normal_data->sensor_group,
				ACTUATOR_FLOW_RATE_ADDR
						+ 15 * (sensor_normal_data->sensor_num - 1));// 瞬时流量值
		//保存流量计采集时间
		//save_time_register(sensor_normal_data->sensor_group,TEMPERATURE_TIME_ADDR );//1218

	} else if (sensor_normal_data->data_typed
			== swap_int16(DATA_TYPE_METER_PARA)) {		//仪表参数
		st_meter_param *sensor_param = (st_meter_param *) p_buff;
		if (!((sensor_param->header.sensor_group > 0)
				&& (sensor_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return -1;
		}
		//保存流量计仪表参数
//		zigbee_write_registers((u16*) &sensor_param,
//				sizeof(st_meter_param) / 2, sensor_param->header.sensor_group,
//				TEMPERATURE_METER_PARA_ADDR );// 1611温度仪表参数
	} else {
		return -1;
	}
	return 0;
}


/*********************************************************************
* @fn     	analysis_torque_speed_load
*
* @brief  	解析有关一体化扭矩转速载荷数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_mac 需要回复的mac地址；
* 			int8 (*func)(modbus_t*, u8*, u8*, u8) 回调发送函数
*
* @return 	-1:非本仪表数据
* 			0：仪表数据格式或者组号编号有误
* 			1：正常完成解析
*/
int8 analysis_torque_speed_load(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	float tmp_meter_val[3] = {0.0};
	u16 tmp_sleep_time = 10;
	u8 tmp_buff[160];
	st_torque_speed_load_normal_packet *sensor_normal_data =
			(st_torque_speed_load_normal_packet *) p_buff;
	st_rtu_ack_sensor_normal_packet *send_reply_packet =
			(st_rtu_ack_sensor_normal_packet*) tmp_buff;
	//判断非一体化扭矩转速载荷仪表数据
	if ((sensor_normal_data->sensor_type
			!= swap_int16(METER_TYPE_WL_INTERGRATION_SPEED_TORQUE))
			|| (sensor_normal_data->protocol_type != swap_int16(PROTOCOL_TYPE))) {
		return -1;
	}
	if (sensor_normal_data->data_typed == swap_int16(DATA_TYPE_ROUTINE_DATA)) {//常规数据
		if (-1 == judge_eq_meter_para(sensor_normal_data->sensor_group,STL_NO)) {//仪表常规数据正常回复
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_ACK_ROUTINE);
			zigbee_read_registers((u16 *) &tmp_sleep_time, 1,
					sensor_normal_data->sensor_group, STL_INTERVAL_ADDR);// 一体化STL采集时间1154
			if(tmp_sleep_time > 1800){
				tmp_sleep_time = 1800;
			}
			send_reply_packet->sleep_time = swap_int16(tmp_sleep_time);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 12);
		} else {//请求仪表参数
			packet_a11_rtu_header((u8 *) &send_reply_packet->protocol_type,
					sensor_normal_data->sensor_group,
					sensor_normal_data->sensor_num,DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->protocol_type), 10);
		}
		if (!((sensor_normal_data->sensor_group > 0)
				&& (sensor_normal_data->sensor_group < MAX_WELL_NUM + 1))) {
			return -1;
		}
		if (sensor_normal_data->sensor_num != 1) {
			return -1;
		}
		time(&rtu_a118_t.meter_sample_time[sensor_normal_data->sensor_group - 1][STL_NO]);
		//保存一体化扭矩转速载荷仪表信息
		save_meter_info((u8 *) &sensor_normal_data->protocol_type,
				sensor_normal_data->sensor_group, STL_METER_INFO_ADDR);// 1420
		tmp_meter_val[0] = swap_rcv_float_seq(sensor_normal_data->speed_valve);
		tmp_meter_val[0] = swap_stor_float_seq(tmp_meter_val[0]);
		tmp_meter_val[1] = swap_rcv_float_seq(sensor_normal_data->torque_valve);
		tmp_meter_val[1] = swap_stor_float_seq(tmp_meter_val[1]);
		tmp_meter_val[2] = swap_rcv_float_seq(sensor_normal_data->load_valve);
		tmp_meter_val[2] = swap_stor_float_seq(tmp_meter_val[2]);
		//保存一体化扭矩转速载荷仪表实时数据
		zigbee_write_registers((u16*)tmp_meter_val, 6,
				sensor_normal_data->sensor_group, STL_SPEED_RTDATA_ADDR );//12
		//保存一体化扭矩转速载荷仪表采集时间
		save_time_register(sensor_normal_data->sensor_group, STL_TIME_ADDR);// 1224

	} else if (sensor_normal_data->data_typed
			== swap_int16(DATA_TYPE_METER_PARA)) {//仪表参数
		st_meter_param *sensor_param = (st_meter_param *) p_buff;
		if (!((sensor_param->header.sensor_group > 0)
				&& (sensor_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return -1;
		}
		//保存一体化扭矩转速载荷仪表参数
		zigbee_write_registers((u16*) &sensor_param,
				sizeof(st_meter_param) / 2, sensor_param->header.sensor_group,
				STL_METER_PARA_ADDR);//2644
	} else {
		return -1;
	}
	return 0;
}



