/*
 * zigbee.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */

#include "zigbee.h"

#include <unistd.h>

#include "../libmodbus-3.0.6/src/modbus.h"

#define RESEND_NUM					10

//extern int modbus_receive_zigbee(modbus_t *ctx, uint8_t *req);

static u8 set_at_order(modbus_t* ctx, char* order, int len,u32 delay);
static void set_panid(modbus_t* ctx, u32 panid);
static u8 set_sc_ch(modbus_t* ctx, u8 auto_or_handle_flag, u8 ch);
//static void set_api_mode_ao(modbus_t* ctx);
//static void set_api_mode_zs(modbus_t* ctx);
//static void get_api_mac_zh(modbus_t* ctx);
//static void get_api_mac_zl(modbus_t* ctx);
static void xbee_store_parameter(modbus_t* ctx);
//static void get_api_mac_zl(modbus_t* ctx);
static u8 set_password_enable(modbus_t* ctx, u8 enable_or_disable_flag);
static void set_xbee_key(modbus_t* ctx, u64 key);
static void xbee_net_restart(modbus_t* ctx);
int8 zigbee_send_packet(modbus_t* ctx, u8* mac, u8* send_data, u8 len);

int8 analysis_zigbee_rcv_data(modbus_t* ctx, u8* rec) {
	st_zighee_api_rx_packet *zigbee_rx_buff = (st_zighee_api_rx_packet *) rec;
	if (zigbee_rx_buff->rx_api_fram_id != ZIGBEE_RCV_DATA_FRAMID) {
		return -1;
	}
	analysis_pressure(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	analysis_temperature(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	analysis_torque_speed_load(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	analysis_flow_meter(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	analysis_load(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	analysis_electrical_param(ctx, &zigbee_rx_buff->rx_api_mac_addr[0],
			&zigbee_rx_buff->rx_api_data[0], zigbee_send_packet);
	return 0;
}

int8 zigbee_send_packet(modbus_t* ctx, u8* mac, u8* send_data, u8 len) {
	u8 tmp_buff[128];
	if (len > A11_ZIGBEE_SND_DATA_LEN) {
		return -1;
	}
	st_zigbee_api_tx_packet* tmp_zigbee_tx = (st_zigbee_api_tx_packet*) tmp_buff;
	tmp_zigbee_tx->tx_api_head = ZIGBEE_API_HEAD;
	tmp_zigbee_tx->tx_api_data_length[0] = HI_UINT16(
			ZIGBEE_SND_PACKET_LEN + len);
	tmp_zigbee_tx->tx_api_data_length[1] = LO_UINT16(
			ZIGBEE_SND_PACKET_LEN + len);
	tmp_zigbee_tx->tx_api_fram_id[0] = ZIGBEE_API_CLUSTERID1;
	tmp_zigbee_tx->tx_api_fram_id[1] = ZIGBEE_API_CLUSTERID0;
	memcpy(&tmp_zigbee_tx->tx_api_mac_addr[0], mac, 10);
	tmp_zigbee_tx->tx_api_mac_addr[8] = ZIGBEE_API_NETADDR0;
	tmp_zigbee_tx->tx_api_mac_addr[9] = ZIGBEE_API_NETADDR1;
	tmp_zigbee_tx->tx_api_expand[0] = ZIGBEE_API_TRANSRC;
	tmp_zigbee_tx->tx_api_expand[1] = ZIGBEE_API_TRANDES;
	tmp_zigbee_tx->tx_api_expand[2] = ZIGBEE_API_CLUSTERID0;
	tmp_zigbee_tx->tx_api_expand[3] = ZIGBEE_API_CLUSTERID1;
	tmp_zigbee_tx->tx_api_expand[4] = ZIGBEE_API_PROFILEID0;
	tmp_zigbee_tx->tx_api_expand[5] = ZIGBEE_API_PROFILEID1;
	tmp_zigbee_tx->tx_api_expand[6] = ZIGBEE_API_BROADCASTRAD;
	tmp_zigbee_tx->tx_api_expand[7] = ZIGBEE_API_SENDOPTION;
	memcpy(&tmp_zigbee_tx->tx_api_data[0], send_data, len);
	if (len == A11_ZIGBEE_SND_DATA_LEN) {
		tmp_zigbee_tx->tx_api_check_sum = calc_sum_check(
				&(tmp_zigbee_tx->tx_api_head));
	} else {
		tmp_zigbee_tx->tx_api_data[len] = calc_sum_check(
				&(tmp_zigbee_tx->tx_api_head));
	}

	if (write(ctx->s, &tmp_zigbee_tx->tx_api_head,
			(ZIGBEE_SND_PACKET_LEN + len + 4))
			< (ZIGBEE_SND_PACKET_LEN + len + 4)) {
		return -1;
	}

	return 0;

}

void set_zigbee_para(modbus_t* ctx, u8 *p_zigbee_para) {

	st_zigbee_para_option* zigbee_para = (st_zigbee_para_option*) p_zigbee_para;
	zigbee_para->panid = swap_register32(zigbee_para->panid);
	zigbee_para->key = swap_register64(zigbee_para->key);
	u8 buff[256];
	st_zigbee_send_fram* zigbee_para_replay = (st_zigbee_send_fram*) buff;
	u8 resend_num = 0;
	u8 zigbee_set_status = ZIGBEE_FAIL;
	int rc = 0;

	set_at_order(ctx,"+++",sizeof("+++")-1,2000000);
	set_at_order(ctx,"ATZS 2\r",sizeof("ATZS 2\r")-1,100000);
	set_at_order(ctx,"ATCE 1\r",sizeof("ATCE 1\r")-1,100000);
	set_at_order(ctx,"ATAP 1\r",sizeof("ATAP 1\r")-1,100000);
	set_at_order(ctx,"ATAO 1\r",sizeof("ATAO 1\r")-1,100000);
	set_at_order(ctx,"ATSM 0\r",sizeof("ATSM 0\r")-1,100000);
	set_at_order(ctx,"ATSP 100\r",sizeof("ATSO 100\r")-1,100000);
	set_at_order(ctx,"ATSN 7FFF\r",sizeof("ATSN 7FFF\r")-1,100000);
	if(-1 == set_at_order(ctx,"ATCN \r",sizeof("ATCN \r")-1,100000)){
		return;
	}

	resend_num = 0;
	zigbee_set_status = ZIGBEE_FAIL;
	//modbus_set_debug(ctx, 1);
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		set_panid(ctx, zigbee_para->panid);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'I')
				&& (zigbee_para_replay->id_name[1] == 'D')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error pandid ");
	}

	zigbee_set_status = ZIGBEE_FAIL;
	resend_num = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		set_sc_ch(ctx, zigbee_para->auto_or_handle_flag, zigbee_para->ch);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'S')
				&& (zigbee_para_replay->id_name[1] == 'C')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = FALSE;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error SC ");
	}

//	zigbee_set_status = ZIGBEE_FAIL;
//	resend_num = 0;
//	do {
//		resend_num++;
//		printf("resend_num=%d\n", resend_num);
//		set_api_mode_ao(ctx);
//		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
//		NULL);
//		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
//				&& (zigbee_para_replay->id_name[0] == 'A')
//				&& (zigbee_para_replay->id_name[1] == 'O')) {
//			zigbee_set_status = ZIGBEE_OK;
//		} else {
//			zigbee_set_status = ZIGBEE_FAIL;
//		}
//	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
//	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
//		perror("zigbee para set error AO ");
//	}

//	zigbee_set_status = ZIGBEE_FAIL;
//	resend_num = 0;
//	do {
//		resend_num++;
//		printf("resend_num=%d\n", resend_num);
//		set_api_mode_zs(ctx);
//		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
//		NULL);
//		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
//				&& (zigbee_para_replay->id_name[0] == 'Z')
//				&& (zigbee_para_replay->id_name[1] == 'S')) {
//			zigbee_set_status = ZIGBEE_OK;
//		} else {
//			zigbee_set_status = ZIGBEE_FAIL;
//		}
//	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
//	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
//		perror("zigbee para set error ZS ");
//	}

	zigbee_set_status = ZIGBEE_FAIL;
	resend_num = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		set_password_enable(ctx, zigbee_para->enable_or_disable_password_flag);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'E')
				&& (zigbee_para_replay->id_name[1] == 'E')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error EE ");
	}

	zigbee_set_status = ZIGBEE_FAIL;
	resend_num = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		set_xbee_key(ctx, zigbee_para->key);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'K')
				&& (zigbee_para_replay->id_name[1] == 'Y')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error KY ");
	}

	zigbee_set_status = ZIGBEE_FAIL;
	resend_num = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		xbee_store_parameter(ctx);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'W')
				&& (zigbee_para_replay->id_name[1] == 'R')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error WR ");
	}

	zigbee_set_status = ZIGBEE_FAIL;
	resend_num = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		xbee_net_restart(ctx);
		rc = modbus_receive_zigbee_timeout(ctx, (uint8_t *) zigbee_para_replay,
		NULL);
		if ((rc == 9) && (zigbee_para_replay->header == ZIGBEE_API_HEAD)
				&& (zigbee_para_replay->id_name[0] == 'N')
				&& (zigbee_para_replay->id_name[1] == 'R')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error NR ");
	}
}

/**
 *
 */
u8 calc_sum_check(u8* p_order_frame) {
	u16 i = 0, temp = 0, len = 0;
	u8 *buff = p_order_frame;
	len = BUILD_UINT16(*(buff + 2), *(buff + 1));
	if (len == 0)
		return 0;
	for (i = 0; i < len; i++) {
		temp += (*(buff + i + 3));
	}
	return (0xFF - (u8) (temp));
}

/**
 *
 */
static void set_panid(modbus_t* ctx, u32 panid) //7E 00 0C 08 01 49 44 00 00 00 00 00 00 00 00 69
{
	st_zigbee_send_fram zigbee_send_fram;
	u64 use_panid = 0;
	use_panid = 0x0000000000000000 | panid;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x0c;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x02;
	zigbee_send_fram.id_name[0] = 'I'; //0x49;
	zigbee_send_fram.id_name[1] = 'D'; //0x44;
	use_panid = swap_int64(use_panid);
	memcpy((void *) (&zigbee_send_fram.data[0]), (void *) (&use_panid), 8);
	zigbee_send_fram.check_sum = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
}


// 设置通道，自动或手动，0自动，ch无效，1 手动，ch :0x0b~0x19；
static u8 set_sc_ch(modbus_t* ctx, u8 auto_or_handle_flag, u8 ch) {
	st_zigbee_send_fram zigbee_send_fram;
	u16 temp = 0;
	if (auto_or_handle_flag == 0) {
		zigbee_send_fram.header = 0x7e;
		zigbee_send_fram.len[0] = 0;
		zigbee_send_fram.len[1] = 0x06;
		zigbee_send_fram.fram_id = 0x08;
		zigbee_send_fram.fram_id_flag = 0x0b;
		zigbee_send_fram.id_name[0] = 0x53;
		zigbee_send_fram.id_name[1] = 0x43;
		zigbee_send_fram.data[0] = 0x7f;
		zigbee_send_fram.data[1] = 0xff;
		zigbee_send_fram.data[2] = calc_sum_check(&(zigbee_send_fram.header));
		write(ctx->s, &zigbee_send_fram.header,
		BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
		return 1;
	} else {
		if ((ch < 0x0b) || (ch > 0x19))
			return 0;
		temp = (1 << (ch - 0x0b));

		zigbee_send_fram.header = 0x7e;
		zigbee_send_fram.len[0] = 0;
		zigbee_send_fram.len[1] = 0x06;
		zigbee_send_fram.fram_id = 0x08;
		zigbee_send_fram.fram_id_flag = 0x0b;
		zigbee_send_fram.id_name[0] = 0x53;
		zigbee_send_fram.id_name[1] = 0x43;
		zigbee_send_fram.data[0] = temp >> 8;
		zigbee_send_fram.data[1] = (u8) temp;
		zigbee_send_fram.data[2] = calc_sum_check(&(zigbee_send_fram.header));
		write(ctx->s, &zigbee_send_fram.header,
		BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
		return 1;
	}
}
//static void set_api_mode_ao(modbus_t* ctx) {
//	st_zigbee_send_fram zigbee_send_fram;
//	zigbee_send_fram.header = 0x7e;
//	zigbee_send_fram.len[0] = 0;
//	zigbee_send_fram.len[1] = 0x05;
//	zigbee_send_fram.fram_id = 0x08;
//	zigbee_send_fram.fram_id_flag = 0x02;
//	zigbee_send_fram.id_name[0] = 0x41;
//	zigbee_send_fram.id_name[1] = 0x4f;
//	zigbee_send_fram.data[0] = 0x01;
//	zigbee_send_fram.data[1] = calc_sum_check(&(zigbee_send_fram.header));
//	write(ctx->s, &zigbee_send_fram.header,
//	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
//}
//static void set_api_mode_zs(modbus_t* ctx) {
//	st_zigbee_send_fram zigbee_send_fram;
//	zigbee_send_fram.header = 0x7e;
//	zigbee_send_fram.len[0] = 0;
//	zigbee_send_fram.len[1] = 0x05;
//	zigbee_send_fram.fram_id = 0x08;
//	zigbee_send_fram.fram_id_flag = 0x0d;
//	zigbee_send_fram.id_name[0] = 0x5a;
//	zigbee_send_fram.id_name[1] = 0x53;
//	zigbee_send_fram.data[0] = 0x02;
//	zigbee_send_fram.data[1] = calc_sum_check(&(zigbee_send_fram.header));
//	write(ctx->s, &zigbee_send_fram.header,
//	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
//}
#if 0
static void get_api_mac_zh(modbus_t* ctx) {
	st_zigbee_send_fram zigbee_send_fram;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x04;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x06;
	zigbee_send_fram.id_name[0] = 0x53;
	zigbee_send_fram.id_name[1] = 0x48;
	zigbee_send_fram.data[0] = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
			BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
}

static void get_api_mac_zl(modbus_t* ctx) {
	st_zigbee_send_fram zigbee_send_fram;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x04;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x07;
	zigbee_send_fram.id_name[0] = 0x53;
	zigbee_send_fram.id_name[1] = 0x4c;
	zigbee_send_fram.data[0] = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
			BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
}
#endif

//设置加密项，使能失能加密项0失能，1 使能；store_zigbee_send_fram.enable_or_disable_password_flag
static u8 set_password_enable(modbus_t* ctx, u8 enable_or_disable_flag) {
	st_zigbee_send_fram zigbee_send_fram;
	if (enable_or_disable_flag > 1)
		return 0;
	if (enable_or_disable_flag == 0) {
		zigbee_send_fram.header = 0x7e;
		zigbee_send_fram.len[0] = 0;
		zigbee_send_fram.len[1] = 0x05;
		zigbee_send_fram.fram_id = 0x08;
		zigbee_send_fram.fram_id_flag = 0x01;
		zigbee_send_fram.id_name[0] = 0x45;
		zigbee_send_fram.id_name[1] = 0x45;
		zigbee_send_fram.data[0] = 0x00;
		zigbee_send_fram.data[1] = calc_sum_check(&(zigbee_send_fram.header));
		write(ctx->s, &zigbee_send_fram.header,
		BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);

		return 1;
	} else {
		zigbee_send_fram.header = 0x7e;
		zigbee_send_fram.len[0] = 0;
		zigbee_send_fram.len[1] = 0x05;
		zigbee_send_fram.fram_id = 0x08;
		zigbee_send_fram.fram_id_flag = 0x01;
		zigbee_send_fram.id_name[0] = 0x45;
		zigbee_send_fram.id_name[1] = 0x45;
		zigbee_send_fram.data[0] = 0x01;
		zigbee_send_fram.data[1] = calc_sum_check(&(zigbee_send_fram.header));
		write(ctx->s, &zigbee_send_fram.header,
		BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
		return 1;
	}
}

static void set_xbee_key(modbus_t* ctx, u64 key) {
	st_zigbee_send_fram zigbee_send_fram;
	u64 use_key = 0;
	use_key = key;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x0C;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x04;
	zigbee_send_fram.id_name[0] = 0x4b;
	zigbee_send_fram.id_name[1] = 0x59;
	use_key = swap_int64(use_key);
	memcpy(zigbee_send_fram.data, (u8 *) (&use_key), 8);
	zigbee_send_fram.check_sum = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
	return;
}

static void xbee_store_parameter(modbus_t* ctx) {
	st_zigbee_send_fram zigbee_send_fram;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x04;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x01;
	zigbee_send_fram.id_name[0] = 0x57;
	zigbee_send_fram.id_name[1] = 0x52;
	zigbee_send_fram.data[0] = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
}
//7E000408044E5253   网络重启
static void xbee_net_restart(modbus_t* ctx) {
	st_zigbee_send_fram zigbee_send_fram;
	zigbee_send_fram.header = 0x7e;
	zigbee_send_fram.len[0] = 0;
	zigbee_send_fram.len[1] = 0x04;
	zigbee_send_fram.fram_id = 0x08;
	zigbee_send_fram.fram_id_flag = 0x04;
	zigbee_send_fram.id_name[0] = 0x4e;
	zigbee_send_fram.id_name[1] = 0x52;
	zigbee_send_fram.data[0] = calc_sum_check(&(zigbee_send_fram.header));
	write(ctx->s, &zigbee_send_fram.header,
	BUILD_UINT16(zigbee_send_fram.len[1],zigbee_send_fram.len[0]) + 4);
}


static u8 set_at_order(modbus_t* ctx, char* order, int len,u32 delay){

	u8 resend_num = 0;
	u8 zigbee_set_status = ZIGBEE_FAIL;
	int rc = 0;
	u8 at_reply[25];
	u8 rc_cnt = 0;
	u8 rc_read_cnt = 0;

	if(len <= 0){
		return -1;
	}

	memset(at_reply, 0, sizeof(at_reply));
	resend_num = 0;
	zigbee_set_status = ZIGBEE_FAIL;
	rc_cnt = 0;
	rc_read_cnt = 0;
	do {
		resend_num++;
		printf("resend_num=%d\n", resend_num);
		write(ctx->s, order, len);
		usleep(delay);
		do {
			rc = read(ctx->s, at_reply + rc_cnt, 3 - rc_cnt);
			rc_read_cnt++;
			if (rc > 0) {
				rc_cnt += rc;
			}
		} while ((rc_cnt < 3) && (rc_read_cnt < 3));
		if ((at_reply[0] == 'O' && at_reply[1] == 'K')
				&& (at_reply[2] == '\r')) {
			zigbee_set_status = ZIGBEE_OK;
		} else {
			zigbee_set_status = ZIGBEE_FAIL;
		}
	} while ((resend_num < RESEND_NUM) && (zigbee_set_status == ZIGBEE_FAIL));
	if ((resend_num >= RESEND_NUM) || (zigbee_set_status == ZIGBEE_FAIL)) {
		perror("zigbee para set error at %s",order);
		return -1;
	}
	return 1;
}

