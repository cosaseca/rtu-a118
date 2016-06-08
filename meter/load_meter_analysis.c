/*
 * load_meter_analysis.c
 *
 *  Created on: Oct 23, 2014
 *      Author: caspar
 */

#include "load_meter_analysis.h"
#include "data/data.h"
#include "config.h"

static u8 judge_elec_param_wired_wireless(u8 well_no,u8 *p_mac);

uint16_t send_msg_load_elec_sysn(int *p_buff,int len) {
	//int rc = MSG_CONFIG_START;
	//int
	pmsg_q_send(rtu_a118_t.zigbee_msg_fd, p_buff,len, 0);
	return 0;
}

/*********************************************************************
* @fn     	analysis_load
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
int8 analysis_load(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8)) {
	u8 tmp_buff[160];
	u16 tmp_read_register = 0x00;
	u16 tmp_wr_register = 0x00;
	u16 tmp_rcv_pack_data[2] = { 0x00, 0x00 };
	float rcv_load_data[2] = { 0.0 };
	st_load_normal_packet *load_normal_data = (st_load_normal_packet *) p_buff; //常规数据
	st_load_first_packet *load_first_pack = (st_load_first_packet *) p_buff; //第一包数据
	st_load_follow_up_packet *load_follow_up_pack =
			(st_load_follow_up_packet *) p_buff; //后续包
	st_meter_param * load_param = (st_meter_param *) p_buff;

	st_equ_measure_load_packet *send_reply_packet =
			(st_equ_measure_load_packet*) tmp_buff;
	st_load_ack_packet *ack_load_diagram_data = (st_load_ack_packet*) tmp_buff;

	if ((load_normal_data->header.sensor_type
			!= swap_int16(METER_TYPE_WL_INTERGRATION_LOAD))
			|| (load_normal_data->header.protocol_type
					!= swap_int16(PROTOCOL_TYPE))) { //不是载荷数据包
		return -1;
	}
	if (load_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_ROUTINE_DATA)) { //接收到的是否未常规数据
		if (!((load_normal_data->header.sensor_group > 0)
				&& (load_normal_data->header.sensor_group < MAX_WELL_NUM + 1))) { //组号是否大于所级联的井号
			return 0;
		}
		if (load_normal_data->header.sensor_num != 1) { //编号是否为1
			return 0;
		}
		if (-1 == judge_eq_meter_para(load_normal_data->header.sensor_group,LOAD_NO)) { //请求测量
			printf("recieve load routine data\n");
			packet_a11_rtu_header(
					(u8 *) &send_reply_packet->header.protocol_type,
					load_normal_data->header.sensor_group,
					load_normal_data->header.sensor_num,
					DATA_TYPE_RTU_EQU_INTERGRATION_DIAGRAM_DATA);
			zigbee_read_registers((u16 *) &tmp_read_register, 1,
					load_normal_data->header.sensor_group, LOAD_SAMPLE_MODE_ADDR);//2010 40995
			printf("LOAD_SAMPLE_MODE_ADDR %d\n", tmp_read_register);
			if(tmp_read_register != 0x01 && tmp_read_register != 0x10){
				tmp_read_register = 0x00;
			}
			printf("LOAD_SAMPLE_MODE_ADDR %d\n", tmp_read_register);
			//send_reply_packet->data_mode = LO_UINT16(
			//		swap_int16(tmp_read_register));
			send_reply_packet->data_mode = tmp_read_register;
			printf("LOAD_SAMPLE_MODE_ADDR 3 %d\n", send_reply_packet->data_mode);
			zigbee_read_registers((u16 *) &tmp_read_register, 1,
					load_normal_data->header.sensor_group, LOAD_SET_POINTS_ADDR);
			if (tmp_read_register > 250) {
				tmp_read_register = 200;
			}
			send_reply_packet->sample_points = swap_int16(tmp_read_register); //功圖點數
			send_reply_packet->synchron_time = 0x00;
			send_reply_packet->sample_interval = 0x00;
			zigbee_read_registers((u16 *) &tmp_read_register, 1,
					load_normal_data->header.sensor_group,  LOAD_INTERVAL_ADDR);// 功图采集间隔
			if(tmp_read_register > 60){
				tmp_read_register = 10;
			}
			send_reply_packet->sample_time = LO_UINT16(tmp_read_register);
			printf("LOAD_SAMPLE_MODE_ADDR 1 %d\n", tmp_read_register);
			func(ctx, p_mac, (u8*) &(send_reply_packet->header.protocol_type),
					sizeof(st_equ_measure_load_packet));
			printf("LOAD_SAMPLE_MODE_ADDR 2 %d\n", tmp_read_register);
		} else { //请求仪表数据
			printf("recieve load routine data & req meter param\n");
			packet_a11_rtu_header(
					(u8 *) &send_reply_packet->header.protocol_type,
					load_normal_data->header.sensor_group,
					load_normal_data->header.sensor_num,
					DATA_TYPE_RTU_EQU_METER_PARA);
			func(ctx, p_mac, (u8*) &(send_reply_packet->header.protocol_type),
					10);
		}
		time(&rtu_a118_t.meter_sample_time[load_normal_data->header.sensor_group - 1][LOAD_NO]);

		save_meter_info((u8 *) &load_normal_data->header.protocol_type,
				load_normal_data->header.sensor_group, LOAD_METER_INFO_ADDR); // 存储仪表信息
		rcv_load_data[0] = swap_rcv_float_seq(load_normal_data->load_value);
		rcv_load_data[1] = swap_rcv_float_seq(load_normal_data->accele_value);
		rcv_load_data[0] = swap_stor_float_seq(rcv_load_data[0]);
		rcv_load_data[1] = swap_stor_float_seq(rcv_load_data[1]);
#if 1
		zigbee_write_registers((uint16_t *) &rcv_load_data, 4,
				load_normal_data->header.sensor_group, LOAD_LOAD_RTDATA_ADDR);
#endif
	} else if (load_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_METER_PARA)
#if 1 //load meter bug
			|| load_normal_data->header.data_typed == swap_int16(0x0100)
#endif
			) {
		printf("\nrecieve load meter param but not available\n");
		if (!((load_param->header.sensor_group > 0)
				&& (load_param->header.sensor_group < MAX_WELL_NUM + 1))) {
			return 0;
		}
		if (load_param->header.sensor_num != 1) {
			return 0;
		}
		printf("recieve load meter param\n");
		zigbee_write_registers((u16*) &load_param,
				sizeof(st_meter_param) / 2, load_param->header.sensor_group,
				LOAD_METER_PARA_ADDR);//仪表参数
	} else if ((load_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_INTERGRATION_LOAD_DIAGRAM))
			&& (load_first_pack->packet_index == 0x00)) { //处理接收功图第一包数据
		if (!((load_first_pack->header.sensor_group > 0)
				&& (load_first_pack->header.sensor_group < MAX_WELL_NUM + 1))) { //组号是否大于所级联的井号
			return 0;
		}
		if (load_first_pack->header.sensor_num != 1) { //编号是否为1
			return 0;
		}
//		//周期数据，同步电参数据
#if 0
		load_first_pack->synchron_time = 10;// swap_int16(load_first_pack->synchron_time);
		load_first_pack->cycle_time = 2000;//1500;//swap_int16(load_first_pack->cycle_time);
		load_first_pack->sample_points = 200;//swap_int16(load_first_pack->sample_points);
#else
		load_first_pack->synchron_time = swap_int16(load_first_pack->synchron_time);
		load_first_pack->cycle_time = swap_int16(load_first_pack->cycle_time);
		load_first_pack->sample_points = swap_int16(load_first_pack->sample_points);
#endif
		uint16_t buf[6];
		int *msg_cmd = (int *)buf;
		*msg_cmd = MSG_LOAD_ELEC_SYSN;
		buf[2] = load_first_pack->synchron_time;
		buf[3] = load_first_pack->cycle_time;
		buf[4] = load_first_pack->sample_points;
		buf[5] = load_first_pack->header.sensor_group;

		printf("MSG_LOAD_ELEC_SYSN %hu %hu %hu %hu\n", buf[2], buf[3], buf[4], buf[5]);

		int rc = judge_elec_param_wired_wireless(
				load_first_pack->header.sensor_group,
				(u8*)rtu_a118_t.elec_param_meter_mac[load_first_pack->header.sensor_group - 1]);
		printf("judge_elec_param_wired_wireless begin %d\n", rc);
		//rc = 0;
		if (1 == rc){
			pmsg_q_send(rtu_a118_t.zigbee_msg_fd, buf, sizeof(buf), 0);
			printf("judge_elec_param_wired_wireless begin group num %d\n", buf[5]);
			log_msg("wireless elec begin %hu %hu %hu %hu\n", buf[2], buf[3], buf[4], buf[5]);
		}
		else{
			pmsg_q_send(rtu_a118_t.rtu_client_msg_fd, buf, sizeof(buf), 0);
			printf("######################################################\n");
			printf("#### judge_elec_param_wired_wireless begin wired #####\n");
			printf("######################################################\n");
			log_msg("wired elec begin %hu %hu %hu %hu\n", buf[2], buf[3], buf[4], buf[5]);
		}

		printf("recieve load first packet data\n");
		tmp_wr_register = load_first_pack->sample_points;
		zigbee_write_registers((u16*) &tmp_wr_register, 1,
				load_normal_data->header.sensor_group, LOAD_SAMPLE_POINTS_ADDR );//984 实际点数100

		tmp_rcv_pack_data[0] = load_first_pack->cycle_time;
		tmp_rcv_pack_data[1] = swap_int16(load_first_pack->travel_length);
		rcv_load_data[0] = (float) (6000.0 / tmp_rcv_pack_data[0]);
		rcv_load_data[1] = (float) (tmp_rcv_pack_data[1] * 0.001);
		rcv_load_data[0]  = swap_stor_float_seq(rcv_load_data[0]);
		rcv_load_data[1]  = swap_stor_float_seq(rcv_load_data[1]);
		zigbee_write_registers((u16*) &rcv_load_data, 4,
				load_first_pack->header.sensor_group, LOAD_TRAVEL_CNT_ADDR ); //991 存储冲刺，冲程101
//		zigbee_write_registers((u16*) &rcv_load_data, 4,
//				load_first_pack->header.sensor_group, 420);

		packet_a11_rtu_header(
				(u8 *) &ack_load_diagram_data->header.protocol_type,
				load_first_pack->header.sensor_group,
				load_first_pack->header.sensor_num,
				DATA_TYPE_RTU_ACK_INTERGRATION_DIAGRAM_DATA);
		ack_load_diagram_data->packet_index = load_first_pack->packet_index;
		func(ctx, p_mac, (u8*) &(ack_load_diagram_data->header.protocol_type),
				sizeof(st_load_ack_packet));
	} else if ((load_normal_data->header.data_typed
			== swap_int16(DATA_TYPE_INTERGRATION_LOAD_DIAGRAM))
			&& (load_follow_up_pack->packet_index > 0)
			&& (load_follow_up_pack->packet_index < 15)) {
		printf("recieve load follow up packet data num = %d\n",
				load_follow_up_pack->packet_index);
		u8 tmp_index = load_follow_up_pack->packet_index;
		u8 cnt = 0;
		for (cnt = 0; cnt < 30; cnt++) {
			load_follow_up_pack->pos_load[cnt] = swap_int16(
					load_follow_up_pack->pos_load[cnt]);
		}
		for (cnt = 0; cnt < 15; cnt++) {
			load_follow_up_pack->pos_load[cnt] = load_follow_up_pack->pos_load[cnt]
					/ 10;
		}
		zigbee_write_registers((u16*) &load_follow_up_pack->pos_load[0], 15,
				load_follow_up_pack->header.sensor_group,
				LOAD_CYCLE_POS_ADDR  + (tmp_index - 1) * 15);//位移250点
		zigbee_write_registers((u16*) &load_follow_up_pack->pos_load[15], 15,
				load_follow_up_pack->header.sensor_group,
				LOAD_CYCLE_LOAD_ADDR  + (tmp_index - 1) * 15);//载荷250点
		packet_a11_rtu_header(
				(u8 *) &ack_load_diagram_data->header.protocol_type,
				load_follow_up_pack->header.sensor_group,
				load_follow_up_pack->header.sensor_num,
				DATA_TYPE_RTU_ACK_INTERGRATION_DIAGRAM_DATA);
		ack_load_diagram_data->packet_index = load_follow_up_pack->packet_index;
		func(ctx, p_mac, (u8*) &(ack_load_diagram_data->header.protocol_type),
				sizeof(st_load_ack_packet));
		if (tmp_index == 14) {
			save_time_register(load_follow_up_pack->header.sensor_group, LOAD_CYCLE_TIME_ADDR );//1242
		}
	} else {
		return 0;
	}

	return 1;
}

/*********************************************************************
* @fn     	judge_elec_param_wired_wireless
*
* @brief  	解析有关电参数数据
*
* @param  	u8 well_no
*
* @return 	0: wired
* 			1: wireless
*/
static u8 judge_elec_param_wired_wireless(u8 well_no,u8 *p_mac){
	u8 cnt = 0;
	for(cnt = 0; cnt < 10; cnt ++){
		if(p_mac[cnt] != 0){
			return 1;
		}
	}
	return 0;
}
