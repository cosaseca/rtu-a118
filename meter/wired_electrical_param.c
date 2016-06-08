/*
 * wired_electrical_param.c
 *
 *  Created on: Nov 18, 2014
 *      Author: caspar
 */

#include <unistd.h>
#include "wired_electrical_param.h"
#include "modbus_485/modbus_485.h"
#include "config.h"


static u8 buff_backward(int16 * buff,u16 len);

int time_handle_fun(modbus_t *ctx, uint16_t *syn_along_info){

	printf("----------------time_handle_fun---------------begin\n");
	log_msg("%hu %hu %hu", syn_along_info[0], syn_along_info[1], syn_along_info[2]);
	uint16_t electrical_well_no;
	zigbee_read_registers((uint16_t *) &electrical_well_no, 1, 1,
		ELEC_WELL_NO_ADDR);
	modbus_set_slave(ctx,electrical_well_no);
	struct timeval syn_t0;
//	uint16_t syn_along_info[3] = {900, 3600, 200};
	gettimeofday(&syn_t0, NULL);
	uint16_t tm_first = (syn_along_info[1]/2 + syn_along_info[0]);//%(syn_along_info[1]);
	syn_t0.tv_sec += tm_first/100;
	syn_t0.tv_usec += tm_first * 10 % 1000 * 1000;
	printf("----------------time_handle_fun %d---------------begin\n", syn_along_info[1]);
	if(syn_along_info[1] < 1000){//10S
		log_msg("time < 1000\n");
		return -1;
	}
	//log_msg("%d %d", tm_first);
	if(syn_along_info[1] > 10000){//100S
		log_msg("time > 10000\n");
		return -1;
	}
	uint16_t n = syn_along_info[2];
	uint16_t m = n;
	if(n > 250) {
		n = 250;
		m = n;
		log_msg("n > 250\n");
	}
	uint16_t step_tm = syn_along_info[1]*10/n;
	if(step_tm < 100) {
		n = syn_along_info[1]*10/100;
		step_tm = 100;
		log_msg("step_tm < 100\n");
	}
	//printf("time %u %u %u %u\n", syn_t0.tv_sec, syn_t0.tv_usec, time(NULL), n);
	//log_msg("wait\n");
	timer_sleep(&syn_t0);
	int i;
	int rc = 0;
	short syn_data[250 * 2];
	memset(syn_data,0xFFFF,250*4);
	u8 read_cnt = 0;
	int i_n = 0;
	struct timeval syn_t1 = {syn_t0.tv_sec, syn_t0.tv_usec};
	syn_t1.tv_sec += syn_along_info[1]/100;
	syn_t1.tv_usec += syn_along_info[1] * 10 % 1000 * 1000;

	struct timeval syn_t3;
	printf("----------------time_handle_fun n %d---------------begin\n", n);
	log_msg("begin %hu %hu %hu %hu\n", syn_along_info[0], syn_along_info[1], syn_along_info[2], n);
	ctx->response_timeout.tv_usec = 70000;
	for(i = 0;i < n;) {
		gettimeofday(&syn_t3, NULL);
		if(syn_t3.tv_sec * 1000 + syn_t3.tv_usec / 1000
				> syn_t1.tv_sec * 1000 + syn_t1.tv_usec / 1000) {
			n = i;
			if(n < m/2) {
				log_msg("n < m/2\n");
				return -1;
			}
			log_msg("time out\n");
			break;
		}
		//function
		//printf("time %u %016x %u %u %d\n", syn_t0.tv_sec, syn_t0.tv_usec, time(NULL), n, step_tm);
		u16 tmp_electrical_quantity[4];
		//do {
		rc = modbus_read_registers(ctx, ADDR_ELECTRICAL_PARAM_SYNC,
					NUM_ELECTRICAL_PARAM_SYNC, tmp_electrical_quantity);
		if(-1 == rc) {
			read_cnt++;
			//usleep(30000);
			if(read_cnt > 5) {
				log_msg("read time out\n");
				return -1;
			}
			continue;
		} else {
			read_cnt = 0;
		}
		//} while ((-1 == rc) && (read_cnt < 3));

		syn_data[i%250] = tmp_electrical_quantity[0];
		syn_data[i%250 + 250] = tmp_electrical_quantity[3];
		syn_t0.tv_sec += step_tm/1000;
		syn_t0.tv_usec += step_tm%1000*1000;
		++i;
		printf("step %d\n", i);
		//log_msg("step %d\n", i);
		timer_sleep(&syn_t0);
		//usleep(70);
	}
	//ctx->response_timeout.tv_usec = 500000;
	i_n = i;
	if(n < m){
		uint16_t insert_step = m / (m - n);
		uint16_t move_pos = insert_step - 1;
		for(i = 0; i < (m - n);i++){
			buff_backward((int16 *)&syn_data[move_pos%250],(m - move_pos));
			buff_backward((int16 *)&syn_data[move_pos%250 + 250],(m - move_pos));
			if(move_pos == (m - 1)){
				syn_data[move_pos%250] = (syn_data[move_pos%250 - 1] + syn_data[0])/2;
				syn_data[move_pos%250 + 250] = (syn_data[move_pos%250 + 250- 1] + syn_data[250])/2;
			}
			else{
				syn_data[move_pos%250] = (syn_data[move_pos%250 - 1] + syn_data[move_pos%250 + 1])/2;
				syn_data[move_pos%250 + 250] = (syn_data[move_pos%250 + 250- 1] + syn_data[move_pos%250 + 250 + 1])/2;
			}

			move_pos += insert_step;
		}
	}
	zigbee_write_registers((uint16_t *)syn_data, sizeof(syn_data)/sizeof(uint16_t), electrical_well_no, ELEC_CYCLE_CURRENT_ADDR);
	save_time_register(electrical_well_no, ELEC_CYCLE_TIME_ADDR);
	log_msg("end step %d\n", i_n);
	return 0;
}

///*********************************************************************
// * @fn     	deal_wired_electrical_sync
// *
// * @brief  	请求有线仪表数据
// *
// * @param  	modbus_t* ctx 串口句柄；
// *
// * @return	void
// */
//void deal_wired_electrical_sync(modbus_t* ctx) {
////	static struct timeval time_start = {0,0};
//	int rc = 0;
//	u16 tmp_electrical_quantity[4];
//	u8 read_cnt = 0;
//	do {
//		rc = modbus_read_registers(ctx, ADDR_ELECTRICAL_PARAM_SYNC,
//				NUM_ELECTRICAL_PARAM_SYNC, tmp_electrical_quantity);
//		read_cnt++;
//	} while ((-1 == rc) && (read_cnt < 3));
//
//}


/*********************************************************************
 * @fn     	deal_wired_electrical
 *
 * @brief  	请求有线仪表数据
 *
 * @param  	modbus_t* ctx 串口句柄；
 *
 * @return	u8
 */
int deal_wired_electrical(modbus_t* ctx) {
	uint16_t electrical_well_no;
	zigbee_read_registers((uint16_t *) &electrical_well_no, 1, 1,
		ELEC_WELL_NO_ADDR);

	modbus_set_slave(ctx,electrical_well_no);
	if(ctx->slave >= RTU_MAX_WELL_NUM || ctx->slave < 1){
		return -1;
	}

	short int tmp_electrical_param[10];
	u16 tmp_electrical_quantity[4];
	float electrical_param_result[12];
	u16 elec_i0 = 5;
	u16 elec_u0 = 250;
	u16 elec_ipp = 40;
	u16 elec_upp = 1;
	u8 read_cnt = 0x00;
	//printf("aaaaaaaaaaaaaaaaa\n");
	int rc = 0;
	do {//读取电参测量范围
		rc = modbus_read_registers(ctx,  ADDR_ELECTRICAL_SET_SCALE ,
				NUM_ELECTRICAL_SET_SCALE , tmp_electrical_quantity);
		read_cnt++;
		usleep(80000);
	} while ((-1 == rc) && (read_cnt < 3));
	if (-1 == rc) {
		return -1; //读取失败
		printf("read electrical scale fail\n");
	}
	elec_u0 = tmp_electrical_quantity[0]; //电参模块的电压量程 250V
	elec_i0 = tmp_electrical_quantity[1]; // /10 电流量程 5A对应50
	elec_i0 /= 10;
	printf("elec_u0 = %d,elec_i0 = %d\n",elec_u0,elec_i0);

	zigbee_read_registers((u16 *) &elec_upp, 1, electrical_well_no, ELEC_VOLTAGE_RATE_ADDR);
	zigbee_read_registers((u16 *) &elec_ipp, 1, electrical_well_no, ELEC_CURRENT_RATE_ADDR);
	if (elec_upp > 5000) {
		elec_upp = 1;
	}
	if (elec_ipp > 1000) {
		elec_ipp = 1;
	}

	read_cnt = 0;
	rc = 0;
	tmp_electrical_quantity[0] = elec_upp;
	tmp_electrical_quantity[1] = elec_ipp;
	do {//读取电参测量范围
		rc = modbus_write_registers(ctx, ADDR_ELECTRICAL_SET_RATIO,
				NUM_ELECTRICAL_SET_RATIO, tmp_electrical_quantity);
		read_cnt++;
		usleep(80000);
	} while ((-1 == rc) && (read_cnt < 3));
	if (-1 == rc) {
		return -1; //读取失败
		printf("write electrical ratio fail\n");
	}

	read_cnt = 0;
	rc = 0;
	do {
		rc = modbus_read_registers(ctx, ADDR_ELECTRICAL_QUANTITY,
		NUM_ELECTRICAL_QUANTITY, tmp_electrical_quantity);
		read_cnt++;
		usleep(80000);
	} while ((-1 == rc) && (read_cnt < 3));
	if (-1 == rc) {
		return -1; //读取失败
		printf("read electrical quantity fail\n");
	}

	read_cnt = 0;
	do {
		rc = modbus_read_registers(ctx, ADDR_ELECTRICAL_PARAM,
				NUM_ELECTRICAL_PARAM, (uint16_t *)tmp_electrical_param);
		read_cnt++;
		usleep(80000);
	} while ((-1 == rc) && (read_cnt < 3));
	if (-1 == rc) {
		return -1; //读取失败
		printf("read electrical param fail\n");
	}

	time(&rtu_a118_t.meter_sample_time[electrical_well_no - 1][ELECTRICAL_PARAMETER_NO]);
	u16 cnt = 0;
	for (cnt = 0; cnt < 3; cnt++) {
		electrical_param_result[cnt] = tmp_electrical_param[cnt + 3];
	}
	for (cnt = 0; cnt < 3; cnt++) {
		electrical_param_result[cnt+3] = tmp_electrical_param[cnt];
	}
	for (cnt = 0; cnt < 4; cnt++) {
		electrical_param_result[8 + cnt] = tmp_electrical_param[6 + cnt];
	}
	electrical_param_result[0] = electrical_param_result[0] * elec_i0 * elec_ipp
			/ 10000.0;	//A相电流
	electrical_param_result[1] = electrical_param_result[1] * elec_i0 * elec_ipp
			/ 10000.0;	//B相电流
	electrical_param_result[2] = electrical_param_result[2] * elec_i0 * elec_ipp
			/ 10000.0;	//C相电流
	electrical_param_result[3] = electrical_param_result[3] * elec_u0 * elec_upp
			/ 10000.0;	//A相电压
	electrical_param_result[4] = electrical_param_result[4] * elec_u0 * elec_upp
			/ 10000.0;	//B相电压
	electrical_param_result[5] = electrical_param_result[5] * elec_u0 * elec_upp
			/ 10000.0;	//C相电压
	electrical_param_result[6] = BUILD_UINT322(tmp_electrical_quantity[1],
			tmp_electrical_quantity[0]);	//有功电能
	electrical_param_result[7] = BUILD_UINT322(tmp_electrical_quantity[3],
			tmp_electrical_quantity[2]);	//无功电能
	electrical_param_result[6] = electrical_param_result[6] * elec_u0 * elec_upp
			* elec_i0 * elec_ipp / 10800000.0;
	electrical_param_result[7] = electrical_param_result[7] * elec_u0 * elec_upp
			* elec_i0 * elec_ipp / 10800000.0;
	electrical_param_result[8] = electrical_param_result[8] * elec_u0 * elec_upp
			* elec_i0 * elec_ipp * 3 / 10000.0;
	electrical_param_result[9] = electrical_param_result[9] * elec_u0 * elec_upp
			* elec_i0 * elec_ipp * 3 / 10000.0;
	electrical_param_result[10] = electrical_param_result[10] * elec_u0
			* elec_upp * elec_i0 * elec_ipp * 3 / 10000.0;
	electrical_param_result[8] = electrical_param_result[8] / 1000.0;	//有功功率
	electrical_param_result[9] = electrical_param_result[9] / 1000.0;	//无功功率
	electrical_param_result[10] = electrical_param_result[10] / 1000.0;	//视在功率，反向功率
	electrical_param_result[11] = electrical_param_result[11] / 10000.0; //功率因数

	for (cnt = 0; cnt < 12; cnt++) {
		electrical_param_result[cnt] = swap_stor_float_seq(
				electrical_param_result[cnt]);
	}
	printf("electrical_param_result = %f\n",electrical_param_result[0]);
	zigbee_write_registers((u16*) electrical_param_result, 24, electrical_well_no, ELEC_CURRENT_RTDATA_ADDR);
	save_time_register(electrical_well_no, ELEC_TIME_ADDR);//保存电数据时间
	memset(rtu_a118_t.elec_param_meter_mac[ctx->slave - 1],0,10);
	return rc;
}

/*********************************************************************
* @fn     	buff_backward
*
* @brief  	把缓冲区向后移动一个字节
*
* @param
*
* @return	1:成功
* 			0:失败
*/
static u8 buff_backward(int16 * buff,u16 len)
{
    u16 i;
    if(len <= 1){
		return 0;
    }
	for(i = len; i > 0;i --)
	{
		buff[i] = buff[i-1];
	}
	return 1;
}
