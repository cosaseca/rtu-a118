/*
 * wired_actuator.c
 *
 *  Created on: Nov 18, 2014
 *      Author: caspar
 */

#include <unistd.h>
#include "wired_actuator.h"

//#define printf(format, arg...)

static u16 packet_sum_check(u8* p_buff, u8 len);
static void rtu_equ_actuator_data(modbus_t* ctx);
static void rtu_set_actuator_param(modbus_t* ctx);

static int deal_wired_actuator1(modbus_t* ctx){
	u8 resend_num = 0;
	u8 buff[256];
	int rc = 0;
	u8 actuator_status = 0x00;
	float tmp_float_data = 0.0;
	st_actuator_para_ack_rtu *actuator_para_reply = (st_actuator_para_ack_rtu *)buff;
	st_actuator_ack_rtu *actuator_data_reply = (st_actuator_ack_rtu *)buff;

	uint16_t actuator_well_no;
	zigbee_read_registers((uint16_t *) &actuator_well_no, 1, 1,
		ACTUATOR_WELL_NO_ADDR);

	//油管
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		ctx->slave = 0x01;	//SLAVE_NO_ACTUATOR;
		rtu_set_actuator_param(ctx);
		rc = modbus_receive_actuator_timeout(ctx, (u8 *)actuator_para_reply, NULL);
		if ((rc == sizeof(st_actuator_para_ack_rtu))
				&& (0x01 == actuator_para_reply->set_status)
				&&(ctx->slave == actuator_para_reply->addr)
				&&(0x02 == actuator_para_reply->fun_code)) {

			printf("set actuator para success\n");
			actuator_status = 0x01;
		} else {
			printf("set actuator para fail\n");
			actuator_status = 0x00;
		}
	} while ((resend_num < RESEND_ACT_NUM) && (actuator_status == 0x00));

	//if(resend_num >= RESEND_ACT_NUM) {
	//	return -1;
	//}
	if(actuator_status == 0x00) {
		return -1;
	}
	
	resend_num = 0x00;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		ctx->slave = 0x01;	//SLAVE_NO_ACTUATOR;
		rtu_equ_actuator_data(ctx);
		rc = modbus_receive_actuator_timeout(ctx, (u8 *) actuator_data_reply,
				NULL);
		if ((rc == sizeof(st_actuator_ack_rtu))
				&& (0x01 == actuator_data_reply->fun_code)
				&& (0x1e == swap_int16(actuator_data_reply->len))
				&&(ctx->slave == actuator_data_reply->addr)) {
			printf("read actuator data success\n");
			time(&rtu_a118_t.meter_sample_time[actuator_well_no - 1][ACTUATOR_NO1]);
			printf("before convert actuator_data_reply->total_flow %llu\n", actuator_data_reply->total_flow);
			actuator_data_reply->total_flow = swap_int64(actuator_data_reply->total_flow);
			printf("after convert actuator_data_reply->total_flow %llu\n", actuator_data_reply->total_flow);
			tmp_float_data = (float)actuator_data_reply->total_flow/1000.0;
			tmp_float_data = swap_stor_float_seq(tmp_float_data);
			zigbee_write_registers((u16*) &tmp_float_data, 2,actuator_well_no,ACTUATOR_TOTAL_FLOW_ADDR);//累计注入量
			actuator_data_reply->flow_rate = swap_int32(actuator_data_reply->flow_rate);;
			tmp_float_data = (float)actuator_data_reply->flow_rate/1000.0;
			tmp_float_data = swap_stor_float_seq(tmp_float_data);
			zigbee_write_registers((u16*) &tmp_float_data, 2,actuator_well_no,ACTUATOR_FLOW_RATE_ADDR);//流速
			uint16_t temp_value;
			temp_value = actuator_data_reply->flowmeter_comm_status;
			zigbee_write_registers((u16*) &temp_value, 1,actuator_well_no,ACTUATOR_FLOWER_STATE_ADDR);//流量计状态
			temp_value = actuator_data_reply->current_argle;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_ANGLE_ADDR);
			temp_value = swap_int16(actuator_data_reply->adjust_times_total);
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_ADJUST_CNT_ADDR);
			temp_value = actuator_data_reply->auto_flag;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_ADJUST_MODE_ADDR);
			temp_value = actuator_data_reply->flowmeter_type;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_FLOWER_MODEL_ADDR);
			temp_value = actuator_data_reply->version[2];
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_SOFTWARE_VERSION_ADDR);
			temp_value = actuator_data_reply->version[3];
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR_HARDWARE_VERSION_ADDR);
			actuator_status = 0x01;
		} else {
			printf("read actuator data fail\n");
			actuator_status = 0x00;
		}
	} while ((resend_num < RESEND_ACT_NUM) && (actuator_status == 0x00));
	if(actuator_status == 0x00) {
		return -1;
	}
	return 0;
}

static int deal_wired_actuator2(modbus_t* ctx){
	u8 resend_num = 0;
	u8 buff[256];
	int rc = 0;
	u8 actuator_status = 0x00;
	float tmp_float_data = 0.0;
	st_actuator_para_ack_rtu *actuator_para_reply = (st_actuator_para_ack_rtu *)buff;
	st_actuator_ack_rtu *actuator_data_reply = (st_actuator_ack_rtu *)buff;

	uint16_t actuator_well_no;
	zigbee_read_registers((uint16_t *) &actuator_well_no, 1, 1,
	ACTUATOR_WELL_NO_ADDR);

	//套管
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		ctx->slave = 0x02;	//SLAVE_NO_ACTUATOR;
		rtu_set_actuator_param(ctx);
		rc = modbus_receive_actuator_timeout(ctx, (u8 *)actuator_para_reply, NULL);
		if ((rc == sizeof(st_actuator_para_ack_rtu))
				&& (0x01 == actuator_para_reply->set_status)
				&&(ctx->slave == actuator_para_reply->addr)
				&&(0x02 == actuator_para_reply->fun_code)) {

			printf("set actuator para success\n");
			actuator_status = 0x01;
		} else {
			printf("set actuator para fail\n");
			actuator_status = 0x00;
		}
	} while ((resend_num < RESEND_ACT_NUM) && (actuator_status == 0x00));

	//if(resend_num >= RESEND_ACT_NUM) {
	//	return;
	//}

	if(actuator_status == 0x00) {
		return -1;
	}
	
	resend_num = 0x00;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		ctx->slave = 0x02;	//SLAVE_NO_ACTUATOR;
		rtu_equ_actuator_data(ctx);
		rc = modbus_receive_actuator_timeout(ctx, (u8 *) actuator_data_reply,
				NULL);
		if ((rc == sizeof(st_actuator_ack_rtu))
				&& (0x01 == actuator_data_reply->fun_code)
				&& (0x1e == swap_int16(actuator_data_reply->len))
				&&(ctx->slave == actuator_data_reply->addr)) {
			printf("read actuator data success\n");
			time(&rtu_a118_t.meter_sample_time[actuator_well_no - 1][ACTUATOR_NO2]);
			actuator_data_reply->total_flow = swap_int64(actuator_data_reply->total_flow);;
			tmp_float_data = (float)actuator_data_reply->total_flow/1000.0;
			tmp_float_data = swap_stor_float_seq(tmp_float_data);
			zigbee_write_registers((u16*) &tmp_float_data, 2,actuator_well_no,ACTUATOR2_TOTAL_FLOW_ADDR);//累计注入量
			actuator_data_reply->flow_rate = swap_int32(actuator_data_reply->flow_rate);
			tmp_float_data = (float)actuator_data_reply->flow_rate/1000.0;
			tmp_float_data = swap_stor_float_seq(tmp_float_data);
			zigbee_write_registers((u16*) &tmp_float_data, 2,actuator_well_no,ACTUATOR2_FLOW_RATE_ADDR);//流速
			uint16_t temp_value;
			temp_value = actuator_data_reply->flowmeter_comm_status;
			zigbee_write_registers((u16*) &temp_value, 1,actuator_well_no,ACTUATOR2_FLOWER_STATE_ADDR);//流量计状态
			temp_value = actuator_data_reply->current_argle;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_ANGLE_ADDR);
			temp_value = swap_int16(actuator_data_reply->adjust_times_total);
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_ADJUST_CNT_ADDR);
			temp_value = actuator_data_reply->auto_flag;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_ADJUST_MODE_ADDR);
			temp_value = actuator_data_reply->flowmeter_type;
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_FLOWER_MODEL_ADDR);
			temp_value = actuator_data_reply->version[2];
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_SOFTWARE_VERSION_ADDR);
			temp_value = actuator_data_reply->version[3];
			zigbee_write_registers((u16*) &temp_value,1,actuator_well_no,ACTUATOR2_HARDWARE_VERSION_ADDR);
			actuator_status = 0x01;
		} else {
			printf("read actuator data fail\n");
			actuator_status = 0x00;
		}
	} while ((resend_num < RESEND_ACT_NUM) && (actuator_status == 0x00));
	if(actuator_status == 0x00) {
		return -1;
	}
	return 0;
}



/*********************************************************************
 * @fn     	deal_wired_actuator
 *
 * @brief  	有线仪表数据--执行器
 *
 * @param  	modbus_t* ctx 串口句柄；
 *
 * @return	void
 */
int deal_wired_actuator(modbus_t* ctx){
	int rc0 = -1;
	int rc1 = -1;
	rc0 = deal_wired_actuator1(ctx);
	rc1 = deal_wired_actuator2(ctx);
	if(-1 == rc0 && -1 == rc1) {
		return -1;
	}
	return 0;
}



/*********************************************************************
 * @fn     	rtu_equ_actuator_data
 *
 * @brief  	下发时间，请求仪表数据
 *
 * @param  	modbus_t* ctx
 *
 * @return	void //CC 01 01 AA E6 77 54 00 00 00 00 00 00 29 03 EE
 *
 */
static void rtu_equ_actuator_data(modbus_t* ctx){
	time_t time_current;
	time(&time_current);
	st_set_actuator_time equ_actuator_para;
	if(ctx->slave > 0x02){
		return;
	}
	equ_actuator_para.header = 0xCC;
	equ_actuator_para.addr = ctx->slave;
	equ_actuator_para.fun_code = 0x01;
	equ_actuator_para.date_time = swap_int32(time_current);
	memset(equ_actuator_para.reserved,0,6);
	equ_actuator_para.checksum = packet_sum_check((u8 *)&equ_actuator_para.header,13);
	equ_actuator_para.checksum = swap_int16(equ_actuator_para.checksum);
	equ_actuator_para.eof = 0xEE;
	write(ctx->s, &equ_actuator_para.header,sizeof(st_set_actuator_time));
}


/*********************************************************************
 * @fn     	rtu_set_actuator_param
 *
 * @brief  	下发时间，请求仪表数据
 *
 * @param  	modbus_t* ctx
 *
 * @return	void
 */
static void rtu_set_actuator_param(modbus_t* ctx){
	st_set_actuator_para set_actuator_para;
	set_actuator_para.header = 0xCC;
	set_actuator_para.addr = ctx->slave;
	set_actuator_para.fun_code = 0x02;
	uint16_t actuator_well_no;
	zigbee_read_registers((uint16_t *) &actuator_well_no, 1, 1,
	ACTUATOR_WELL_NO_ADDR);
	uint16_t tmp_value;
	if(0x01 == ctx->slave){
		zigbee_read_registers((u16 *) &set_actuator_para.injec_alloc_perday, 2,actuator_well_no,ACTUATOR_INJEC_PERDAY_ADDR);
		zigbee_read_registers((u16 *) &tmp_value, 1,actuator_well_no,ACTUATOR_ADJUST_ACCURENT_ADDR);
		set_actuator_para.adjust_accuracy_percent = tmp_value;
		zigbee_read_registers((u16 *) &tmp_value, 1,actuator_well_no,ACTUATOR_AUTO_FLAG_ADDR);
		set_actuator_para.auto_flag = tmp_value;
	}
	else if(0x02 == ctx->slave){
		zigbee_read_registers((u16 *) &set_actuator_para.injec_alloc_perday, 2,actuator_well_no,ACTUATOR2_INJEC_PERDAY_ADDR);
		zigbee_read_registers((u16 *) &tmp_value, 1,actuator_well_no,ACTUATOR2_ADJUST_ACCURENT_ADDR);
		set_actuator_para.adjust_accuracy_percent = tmp_value;
		zigbee_read_registers((u16 *) &tmp_value, 1,actuator_well_no,ACTUATOR2_AUTO_FLAG_ADDR);
		set_actuator_para.auto_flag = tmp_value;
	}
	else{
		return;
	}
//	printf("set_actuator_para 1: %08X, 2: %d, 3: %d, 4: %d\n",
//			set_actuator_para.injec_alloc_perday,
//			set_actuator_para.adjust_accuracy_percent,
//			set_actuator_para.auto_flag,
//			actuator_well_no);
	set_actuator_para.injec_alloc_perday = swap_register32(set_actuator_para.injec_alloc_perday);
	//printf("injec_alloc_perday = %08X\n",set_actuator_para.injec_alloc_perday);
#if 0 //float
	float tmp_injec_alloc_perday = 0;
	memcpy(&tmp_injec_alloc_perday, &set_actuator_para.injec_alloc_perday, sizeof(float));
	set_actuator_para.injec_alloc_perday = 1000*tmp_injec_alloc_perday;
	printf("injec_alloc_perday = %d\n",set_actuator_para.injec_alloc_perday);
	set_actuator_para.injec_alloc_perday = swap_int32(set_actuator_para.injec_alloc_perday);
	printf("injec_alloc_perday = %d\n",set_actuator_para.injec_alloc_perday);
#else //int32
	set_actuator_para.injec_alloc_perday = 1000*set_actuator_para.injec_alloc_perday;
	set_actuator_para.injec_alloc_perday = swap_int32(set_actuator_para.injec_alloc_perday);
	//printf("injec_alloc_perday = %d\n",set_actuator_para.injec_alloc_perday);
#endif
	set_actuator_para.start_time_day = 0x00;
	set_actuator_para.checksum = packet_sum_check((u8 *)&set_actuator_para.header,10);
	set_actuator_para.checksum = swap_int16(set_actuator_para.checksum);
	set_actuator_para.eof = 0xEE;
	write(ctx->s, &set_actuator_para.header,sizeof(st_set_actuator_para));
}


/*********************************************************************
 * @fn     	packet_sum_check
 *
 * @brief  	数据包长度为len的和校验
 *
 * @param  	u8* p_buff, 数据起始地址
 * 			u8 len，数据包长度
 *
 * @return	void
 */
static u16 packet_sum_check(u8* p_buff, u8 len) {
	u16 i = 0, temp = 0;
	u8 *buff = p_buff;
	if (len == 0)
		return 0;
	for (i = 0; i < len; i++) {
		temp += (*(buff + i));
	}
	return temp;
}
