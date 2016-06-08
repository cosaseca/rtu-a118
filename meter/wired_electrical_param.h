/*
 * wired_electrical_param.h
 *
 *  Created on: Nov 18, 2014
 *      Author: caspar
 */

#ifndef WIRED_ELECTRICAL_PARAM_H_
#define WIRED_ELECTRICAL_PARAM_H_

#include "wired_meter.h"

//力创 万众 有线电参寄存器数组

#define ADDR_ELECTRICAL_SET_SCALE								0x02	//电压及电流变比 2
#define NUM_ELECTRICAL_SET_SCALE								2
#define ADDR_ELECTRICAL_SET_RATIO								0x05	//电压及电流变比 2
#define NUM_ELECTRICAL_SET_RATIO								2
#define ADDR_ELECTRICAL_QUANTITY								0x0C	//总电量 4个寄存器 //正向 反向
#define NUM_ELECTRICAL_QUANTITY									4
#define ADDR_ELECTRICAL_PARAM									0x40	//除电量参数之外的所有参数10个寄存器
#define NUM_ELECTRICAL_PARAM									10
#define ADDR_ELECTRICAL_PARAM_SYNC								0x43	//除电量参数之外的所有参数10个寄存器
#define NUM_ELECTRICAL_PARAM_SYNC								4

extern int deal_wired_electrical(modbus_t* ctx);

#endif /* WIRED_ELECTRICAL_PARAM_H_ */
