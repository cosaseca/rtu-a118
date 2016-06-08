/*
 * electrical_param_analysis.c
 *
 *  Created on: Nov 7, 2014
 *      Author: caspar
 */

#include "electrical_param_analysis.h"
#include "data/data.h"
#include "config.h"
#include "zigbee/zigbee.h"


/*********************************************************************
* @fn     	send_sysn_electrical_param
*
* @brief  	发送电参同步命令
*
* @param  	modbus_t* ctx 串口句柄；
* 			u16 *syn_along_info 同步信息
*
* @return 	void
*/
void send_sysn_electrical_param(modbus_t *ctx, u16 *syn_along_info){
	printf("begin send_sysn_electrical_param %d\n", syn_along_info[3]);
	u8 buff[128];
	u16 tmp_read_register;
	if(syn_along_info[3] >= RTU_MAX_WELL_NUM || syn_along_info[3] < 1){
		return;
	}
	st_rtu_equ_elect_measure_packet *equ_elec_sysn =
			(st_rtu_equ_elect_measure_packet *) buff;
	packet_a11_rtu_header((u8 *) &equ_elec_sysn->header.protocol_type,
			syn_along_info[3], 1, DATA_TYPE_RTU_EQU_WL_ELECTRICITY_DIAGRAM_DATA);
	zigbee_read_registers((u16 *) &tmp_read_register, 1,
			syn_along_info[3], LOAD_SAMPLE_MODE_ADDR);//
	equ_elec_sysn->current_chart_mode = swap_int16(tmp_read_register);
	equ_elec_sysn->sample_points = swap_int16(syn_along_info[2]);
	uint16_t sync_time = syn_along_info[0];
	sync_time = sync_time>12?sync_time-12:sync_time;
	equ_elec_sysn->synchron_time = swap_int16(sync_time);
	equ_elec_sysn->sample_interval = swap_int16(syn_along_info[1]*10/(syn_along_info[2]-1));
	log_msg("equ_elec_sysn --- %d, %hu, %hu, %hu\n", 
		swap_int16(equ_elec_sysn->current_chart_mode),
		swap_int16(equ_elec_sysn->sample_points),
		swap_int16(equ_elec_sysn->synchron_time),
		swap_int16(equ_elec_sysn->sample_interval));
	equ_elec_sysn->arithmetic = 0x00;
	u8 *mac_a = (u8 *)&(rtu_a118_t.elec_param_meter_mac[syn_along_info[3] - 1]);
	printf("before zigbee_send_packet mac %hx %hx %hx %hx %hx %hx %hx %hx %d\n", *mac_a, *(mac_a + 1),
		*(mac_a + 2), *(mac_a + 3),
		*(mac_a + 4), *(mac_a + 5),
		*(mac_a + 6), *(mac_a + 7),
		syn_along_info[3]);
	zigbee_send_packet(ctx, (u8*)rtu_a118_t.elec_param_meter_mac[syn_along_info[3] - 1],
			(u8*)&equ_elec_sysn->header.protocol_type,
			(u8)sizeof(st_rtu_equ_elect_measure_packet));
	printf("after zigbee_send_packet\n");
}



/*********************************************************************
* @fn     	analysis_electrical_param
*
* @brief  	解析有关电参数数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_mac 需要回复的mac地址；
* 			int8 (*func)(modbus_t*, u8*, u8*, u8) 回调发送函数
*
* @return 	-1:非本仪表数据
* 			0：仪表数据格式或者组号编号有误
* 			1：正常完成解析
*/
int8 analysis_electrical_param(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	u8 tmp_buff[160];
	u8 cnt_cycle = 0x00;
	u16 tmp_read_register = 0x00;
	//u16 tmp_wr_register = 0x00;
	//u16 tmp_rcv_pack_data[2] = { 0x00, 0x00 };
	float rcv_elect_data[11] = { 0.0 };
	st_elect_normal_packet *elect_normal_data = (st_elect_normal_packet *) p_buff; //常规数据
	st_elect_first_packet *elect_first_pack = (st_elect_first_packet *) p_buff; //第一包数据
	st_elect_follow_up_packet *elect_follow_up_pack =
			(st_elect_follow_up_packet *) p_buff; //后续包
	st_meter_param * elect_param = (st_meter_param *) p_buff;

	st_rtu_ack_elect_normal_packet *elect_normal_ack =
			(st_rtu_ack_elect_normal_packet *) tmp_buff;

	st_rtu_equ_elect_measure_packet *send_reply_packet =
			(st_rtu_equ_elect_measure_packet*) tmp_buff;
	st_elect_ack_packet *ack_elect_diagram_data = (st_elect_ack_packet*) tmp_buff;

	if ((elect_normal_data->header.sensor_type
			!= swap_int16(METER_TYPE_WL_ELECTRICITY))
			|| (elect_normal_data->header.protocol_type
					!= swap_int16(PROTOCOL_TYPE))) { //不是电参数据包
		return -1;
	}
	if (elect_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_ROUTINE_DATA)) { //接收到的是否为常规数据
		if (!((elect_normal_data->header.sensor_group > 0)
				&& (elect_normal_data->header.sensor_group < MAX_WELL_NUM + 1))) { //组号是否大于所级联的井号
			return 0;
		}
		if (elect_normal_data->header.sensor_num != 1) { //编号是否为1
			return 0;
		}
		if (-1 == judge_eq_meter_para(elect_normal_data->header.sensor_group,ELECTRICAL_PARAMETER_NO)) { //常规回复
			printf("recieve elect routine data\n");
			packet_a11_rtu_header(
					(u8 *) &elect_normal_ack->header.protocol_type,
					elect_normal_data->header.sensor_group,
					elect_normal_data->header.sensor_num,
					DATA_TYPE_RTU_ACK_ROUTINE);
			zigbee_read_registers((u16 *) &tmp_read_register, 1,
					elect_normal_data->header.sensor_group, ELEC_INTERVAL_ADDR);// 1155采集间隔
			if(tmp_read_register > 1800){
				tmp_read_register = 1800;
			}
			memcpy(&rtu_a118_t.elec_param_meter_mac[elect_normal_data->header.sensor_group - 1], p_mac, 10);
			elect_normal_ack->sleep_time = swap_int16(tmp_read_register);
			func(ctx, p_mac, (u8*) &(send_reply_packet->header.protocol_type),
					sizeof(st_rtu_ack_elect_normal_packet));
		} else { //请求仪表数据
			printf("recieve elect routine data & req meter param\n");
			packet_a11_rtu_header(
					(u8 *) &send_reply_packet->header.protocol_type,
					elect_normal_data->header.sensor_group,
					elect_normal_data->header.sensor_num,
					DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->header.protocol_type),
					10);
		}

		time(&rtu_a118_t.meter_sample_time[elect_normal_data->header.sensor_group - 1][ELECTRICAL_PARAMETER_NO]);
		save_meter_info((u8 *) &elect_normal_data->header.protocol_type,
				elect_normal_data->header.sensor_group, ELEC_METER_INFO_ADDR); // 存储电参仪表信息1400
		rcv_elect_data[0] = swap_rcv_float_seq(elect_normal_data->ia);
		rcv_elect_data[1] = swap_rcv_float_seq(elect_normal_data->ib);
		rcv_elect_data[2] = swap_rcv_float_seq(elect_normal_data->ic);
		rcv_elect_data[3] = swap_rcv_float_seq(elect_normal_data->va);
		rcv_elect_data[4] = swap_rcv_float_seq(elect_normal_data->vb);
		rcv_elect_data[5] = swap_rcv_float_seq(elect_normal_data->vc);
		rcv_elect_data[6] = swap_rcv_float_seq(elect_normal_data->power_factor);
		rcv_elect_data[7] = swap_rcv_float_seq(elect_normal_data->active_power);
		rcv_elect_data[8] = swap_rcv_float_seq(elect_normal_data->reactive_power);
		rcv_elect_data[9] = swap_rcv_float_seq(elect_normal_data->quantity_elect);
		rcv_elect_data[10] = swap_rcv_float_seq(elect_normal_data->reserve);
		for (cnt_cycle = 0x00; cnt_cycle < 10; cnt_cycle++) {
			rcv_elect_data[cnt_cycle] = swap_stor_float_seq(
					rcv_elect_data[cnt_cycle]);
		}
		zigbee_write_registers((u16*) rcv_elect_data, 12,
				elect_normal_data->header.sensor_group, ELEC_CURRENT_RTDATA_ADDR);// ia ib ic va vb vc 18
		zigbee_write_registers((u16*) &rcv_elect_data[6], 2,
						elect_normal_data->header.sensor_group, ELEC_POWER_FACTOR_RTDATA_ADDR);// 功率因数40
		zigbee_write_registers((u16*) &rcv_elect_data[7], 2,
						elect_normal_data->header.sensor_group, ELEC_ACTIVE_POWER_RATE_RTDATA_ADDR);// 有功功率34
		zigbee_write_registers((u16*) &rcv_elect_data[8], 2,
						elect_normal_data->header.sensor_group, ELEC_REACTIVE_POWER_RATE_RTDATA_ADDR);// 无功功率36
		rcv_elect_data[9] += rcv_elect_data[10];
		zigbee_write_registers((u16*) &rcv_elect_data[9], 2,
						elect_normal_data->header.sensor_group, ELEC_ACTIVE_POWER_RTDATA_ADDR);// 有功功耗
		zigbee_write_registers((u16*) &rcv_elect_data[10], 2,
						elect_normal_data->header.sensor_group, ELEC_REACTIVE_POWER_RTDATA_ADDR);// 无功功耗
		save_time_register(elect_normal_data->header.sensor_group, ELEC_TIME_ADDR);
	}
	else if (elect_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_METER_PARA)) {//电参仪表参数
		if (!((elect_param->header.sensor_group > 0)
				&& (elect_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return 0;
		}
		if (elect_param->header.sensor_num != 1) {
			return 0;
		}
		printf("recieve elect meter param\n");
		//存储仪表参数
		zigbee_write_registers((u16*) &elect_param,
				sizeof(st_meter_param) / 2, elect_param->header.sensor_group,
				ELEC_METER_PARA_ADDR );//1685
	}else if ((elect_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_ELECTRICITY_DIAGRAM))
			&& (elect_first_pack->packet_index == 0x00)) { //处理接收电流图第一包数据
		if (!((elect_first_pack->header.sensor_group > 0)
				&& (elect_first_pack->header.sensor_group < MAX_WELL_NUM + 1))) { //组号是否大于所级联的井号
			return 0;
		}
		if (elect_first_pack->header.sensor_num != 1) { //编号是否为1
			return 0;
		}
		//周期数据，同步电参数据
		printf("recieve elect first packet data\n");

		packet_a11_rtu_header(
				(u8 *) &ack_elect_diagram_data->header.protocol_type,
				elect_first_pack->header.sensor_group,
				elect_first_pack->header.sensor_num,
				DATA_TYPE_RTU_ACK_WL_ELECTRICITY_DIAGRAM_DATA);
		ack_elect_diagram_data->packet_index = elect_first_pack->packet_index;
		func(ctx, p_mac, (u8*) &(ack_elect_diagram_data->header.protocol_type),
				sizeof(st_elect_ack_packet));
	}else if ((elect_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_ELECTRICITY_DIAGRAM))
			&& (elect_follow_up_pack->packet_index > 0)
			&& (elect_follow_up_pack->packet_index < 15)) {//电参后续包
		printf("recieve elect follow up packet data num = %d\n",
				elect_follow_up_pack->packet_index);
		u8 tmp_index = elect_follow_up_pack->packet_index;
		u8 cnt = 0;
		for (cnt = 0; cnt < 30; cnt++) {
			elect_follow_up_pack->rcv_data[cnt] = swap_int16(
					elect_follow_up_pack->rcv_data[cnt]);
		}
		//保存电流图数据
		zigbee_write_registers((u16*) &elect_follow_up_pack->rcv_data[0], 15,
				elect_follow_up_pack->header.sensor_group,
				ELEC_CYCLE_CURRENT_ADDR  + (tmp_index - 1) * 15);//605
		//保存功率图数据
		zigbee_write_registers((u16*) &elect_follow_up_pack->rcv_data[15], 15,
				elect_follow_up_pack->header.sensor_group,
				ELEC_CYCLE_POWER_ADDR  + (tmp_index - 1) * 15);//855
		packet_a11_rtu_header(
				(u8 *) &ack_elect_diagram_data->header.protocol_type,
				elect_follow_up_pack->header.sensor_group,
				elect_follow_up_pack->header.sensor_num,
				DATA_TYPE_RTU_ACK_WL_ELECTRICITY_DIAGRAM_DATA);
		ack_elect_diagram_data->packet_index = elect_follow_up_pack->packet_index;
		func(ctx, p_mac, (u8*) &(ack_elect_diagram_data->header.protocol_type),
				sizeof(st_elect_ack_packet));
		log_msg("pack index %u", tmp_index);
		if (tmp_index == 14) {//保存周期数据时间
			save_time_register(elect_follow_up_pack->header.sensor_group, ELEC_CYCLE_TIME_ADDR);//
		}
	} else {
		return 0;
	}

	return 1;
}

