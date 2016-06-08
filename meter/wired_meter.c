/*
 * wired_meter.c
 *
 *  Created on: Nov 25, 2014
 *      Author: caspar
 */

#include "wired_meter.h"
#include "wired_electrical_param.h"



/*********************************************************************
* @fn     	deal_master_485_packet
*
* @brief  	请求有线仪表数据
*
* @param  	modbus_t* ctx 串口句柄；
* 			u8* p_ctrl 主485控制权
*
* @return	void
*/
int deal_master_485_packet(modbus_t *ctx ,u8 *p_ctrl, void *arg){
	int rc = -1;
	//log_msg("%d", *p_ctrl);
	switch(*p_ctrl){
	case MASTER_485_CTRL_ELECTRICAL_SYNC:{
//		deal_wired_electrical_sync(ctx);
		rc = time_handle_fun(ctx, arg);
		if(MASTER_485_CTRL_ELECTRICAL_SYNC == *p_ctrl) {
			*p_ctrl = MASTER_485_CTRL_DEFAULT;
		}
		break;
	}
	case MASTER_485_CTRL_ELECTRICAL:{
		rc = deal_wired_electrical(ctx);
		if(MASTER_485_CTRL_ELECTRICAL == *p_ctrl) {
			*p_ctrl = MASTER_485_CTRL_DEFAULT;
		}
		break;
	}
	case MASTER_485_CTRL_ACTUATOR:{
		rc = deal_wired_actuator(ctx);
		if(MASTER_485_CTRL_ACTUATOR == *p_ctrl) {
			*p_ctrl = MASTER_485_CTRL_DEFAULT;
		}
		break;
	}
	default :
		*p_ctrl = MASTER_485_CTRL_DEFAULT;
		break;

	}
	//log_msg("%d", *p_ctrl);
	return rc;
}



