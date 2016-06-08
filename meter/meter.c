/*
 * meter.c
 *
 *  Created on: Nov 9, 2014
 *      Author: caspar
 */


#include "data/data.h"
#include "common/common.h"
#include "meter/meter.h"
#include <time.h>
#include <sys/sysinfo.h>

/*********************************************************************
* @fn     clear_meter_rtdata
*
* @brief  清仪表实时数据
*
* @param  void
*
* @return void
*/
void clear_meter_rtdata(void){
	u8 i = 0;
	u16 tmp_sleep_time = 0;
	u16 meter_sample_interval[RTU_MAX_WELL_NUM][9];
	time_t current_time;
	time(&current_time);
	for(i = 0; i < RTU_MAX_WELL_NUM; i ++){
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,(OIL_PRESSURE_INTERVAL_ADDR));
		meter_sample_interval[i][OIL_PRESSURE_NO] = tmp_sleep_time; //OIL_PRESSURE_SIZE
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(OIL_PRESSURE_INTERVAL_ADDR + OIL_PRESSURE_SIZE));
		meter_sample_interval[i][CASING_PRESSURE_NO] = tmp_sleep_time;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(OIL_PRESSURE_INTERVAL_ADDR + OIL_PRESSURE_SIZE*2));
		meter_sample_interval[i][BACK_PRESSURE_NO] = tmp_sleep_time;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(TEMPERATURE_INTERVAL_ADDR));
		meter_sample_interval[i][TEMPERATURE_NO] = tmp_sleep_time;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(LOAD_INTERVAL_ADDR));
		meter_sample_interval[i][LOAD_NO] = tmp_sleep_time*60;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(ELEC_INTERVAL_ADDR));
		meter_sample_interval[i][ELECTRICAL_PARAMETER_NO] = tmp_sleep_time;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(STL_INTERVAL_ADDR));
		meter_sample_interval[i][STL_NO] = tmp_sleep_time;
		zigbee_read_registers((u16 *) &tmp_sleep_time, 1,i+1,
				(ACTUATOR_INTERVAL_ADDR));
		meter_sample_interval[i][ACTUATOR_NO1] = tmp_sleep_time;
		meter_sample_interval[i][ACTUATOR_NO2] = tmp_sleep_time;
	}
	for (i = 0; i < RTU_MAX_WELL_NUM; i++) {
		if (current_time
				> meter_sample_interval[i][OIL_PRESSURE_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][OIL_PRESSURE_NO]) { //油压
			rtu_a118_t.meter_sample_time[i][OIL_PRESSURE_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,2, i+1,OIL_PRESSURE_RTDATA_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][CASING_PRESSURE_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][CASING_PRESSURE_NO]) { //套压
			rtu_a118_t.meter_sample_time[i][CASING_PRESSURE_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,2, i+1,OIL_PRESSURE_RTDATA_ADDR + OIL_PRESSURE_SIZE);
		}
		if (current_time
				> meter_sample_interval[i][BACK_PRESSURE_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][BACK_PRESSURE_NO]) { //回压
			rtu_a118_t.meter_sample_time[i][BACK_PRESSURE_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,2, i+1,OIL_PRESSURE_RTDATA_ADDR + 2*OIL_PRESSURE_SIZE);
		}
		if (current_time
				> meter_sample_interval[i][TEMPERATURE_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][TEMPERATURE_NO]) { //温度
			rtu_a118_t.meter_sample_time[i][TEMPERATURE_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,2, i+1,TEMPERATURE_RTDATA_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][LOAD_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][LOAD_NO]) { //载荷
			rtu_a118_t.meter_sample_time[i][LOAD_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,4, i+1,LOAD_LOAD_RTDATA_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][ELECTRICAL_PARAMETER_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][ELECTRICAL_PARAMETER_NO]) { //电参
			rtu_a118_t.meter_sample_time[i][ELECTRICAL_PARAMETER_NO] =
					current_time;
			zigbee_write_registers_mset(0xFFFF,24, i+1,ELEC_CURRENT_RTDATA_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][STL_NO]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][STL_NO]) { //一体化载荷
			rtu_a118_t.meter_sample_time[i][STL_NO] = current_time;
			zigbee_write_registers_mset(0xFFFF,6, i+1,STL_SPEED_RTDATA_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][ACTUATOR_NO1]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][ACTUATOR_NO1]) { //执行器1
			rtu_a118_t.meter_sample_time[i][ACTUATOR_NO1] = current_time;
			zigbee_write_registers_mset(0xFFFF,11, i+1,ACTUATOR_TOTAL_FLOW_ADDR);
		}
		if (current_time
				> meter_sample_interval[i][ACTUATOR_NO2]*CLEAR_RTDATA_CNT
						+ rtu_a118_t.meter_sample_time[i][ACTUATOR_NO2]) { //温度
			rtu_a118_t.meter_sample_time[i][ACTUATOR_NO2] = current_time;
			zigbee_write_registers_mset(0xFFFF,11, i+1,ACTUATOR2_TOTAL_FLOW_ADDR);
		}

	}


}

/*********************************************************************
* @fn     	packet_a11_rtu_header
*
* @brief  	发送打包a11头
*
* @param	u8* a11_header A11协议头
* 			u8 group	组号
* 			u8 no		编号
* 			u16 data_type	协议类型
*
* @return void
*/
void packet_a11_rtu_header(u8* a11_header, u8 group, u8 no,u16 data_type) {
	st_rtu_ack_sensor_normal_packet * send_ack_header =
			(st_rtu_ack_sensor_normal_packet *) a11_header;
	send_ack_header->protocol_type = swap_int16(PROTOCOL_TYPE);
	send_ack_header->vender_no = swap_int16(VENDER_CODE_TJWZ);
	send_ack_header->sensor_type = swap_int16(METER_TYPE_WL_RTU);
	send_ack_header->sensor_group = group;
	send_ack_header->sensor_num = no;
	send_ack_header->data_typed = swap_int16(data_type);
}


/*********************************************************************
* @fn     	save_time_register
*
* @brief  	保存当前时间到井号为well_no，地址为addr
*
* @param	u16 well_no 井号
* 			u16 addr 当前地址
*
* @return void
*/
void save_time_register(u16 well_no, u16 addr) {
	u16 time_register[6];
	time_t a;
	time(&a);
	struct tm* st_time;
	st_time = localtime(&a);
	time_register[0] = int16_to_bcd(st_time->tm_year + 1900);
	time_register[1] = int16_to_bcd(st_time->tm_mon + 1);
	time_register[2] = int16_to_bcd(st_time->tm_mday);
	time_register[3] = int16_to_bcd(st_time->tm_hour);
	time_register[4] = int16_to_bcd(st_time->tm_min);
	time_register[5] = int16_to_bcd(st_time->tm_sec);
	zigbee_write_registers((u16 *) &time_register, 6, well_no, addr);
}


/*********************************************************************
* @fn
*
* @brief  	判断是否请求仪表参数（开机时间小于180S及每天的12点0-10分钟请求）
*
* @param	void
*
* @return 	0: 请求仪表参数
* 			-1：不请求
*/

/*********************************************************************
* @fn     	judge_eq_meter_para
*
* @brief  	判断是否请求仪表参数（开机时间小于180S及每天的12点0-10分钟请求）
*
* @param	void
*
* @return 	0: 请求仪表参数
* 			-1：不请求
*/

int8 judge_eq_meter_para(int16 well_no,u8 meter_type) {
	time_t tmp_time;
	//struct sysinfo tmp_sysinfo;
	time(&tmp_time);
	//struct tm* st_time;
	//st_time = localtime(&tmp_time);
	//sysinfo(&tmp_sysinfo);
//	if (tmp_sysinfo.uptime < 180) {
//		printf("systerm start less 180\n");
//		return 0;
//	}
	if((well_no >= RTU_MAX_WELL_NUM - 1) || (well_no < 1)){
		return -1;
	}
	if(meter_type >= MAX_NUM_METER_TYPE){
		return -1;
	}

	if (tmp_time - rtu_a118_t.equ_meter_para_time[well_no - 1][meter_type] > PER_DAY_SECONDS) {
		rtu_a118_t.equ_meter_para_time[well_no - 1][meter_type] = tmp_time;
		return 0;
	} else {
		return -1;
	}
}


/*********************************************************************
* @fn     	save_meter_info
*
* @brief  	保存仪表信息
*
* @param	u8* a11_header A11协议头
* 			u16 well_no		井号
* 			u16 addr		地址
*
* @return void
*/
void save_meter_info(u8* a11_header, u16 well_no, u16 addr) {
	st_sensor_normal_packet *tmp_meter_normal =
			(st_sensor_normal_packet *) a11_header;
	u16 tmp_meter_info_buff[10];
	tmp_meter_info_buff[0] = swap_int16(tmp_meter_normal->vender_no);
	tmp_meter_info_buff[1] = swap_int16(tmp_meter_normal->sensor_type);
	tmp_meter_info_buff[2] = tmp_meter_normal->sensor_group;
	tmp_meter_info_buff[3] = tmp_meter_normal->sensor_num;
	tmp_meter_info_buff[4] = tmp_meter_normal->comm_quality;
	tmp_meter_info_buff[5] = tmp_meter_normal->bat_value;
	tmp_meter_info_buff[6] = swap_int16(tmp_meter_normal->collect_interval);
	tmp_meter_info_buff[7] = swap_int16(tmp_meter_normal->meter_satus);
	tmp_meter_info_buff[8] = 0;
	tmp_meter_info_buff[9] = 0;
	//
	zigbee_write_registers((u16 *) &tmp_meter_info_buff[0], 10, well_no, addr);
}

