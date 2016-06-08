/*
 * wired_meter.h
 *
 *  Created on: Nov 25, 2014
 *      Author: caspar
 */

#ifndef WIRED_METER_H_
#define WIRED_METER_H_

#include "libmodbus-3.0.6/src/modbus-private.h"
#include "common/common.h"
#include "meter.h"


#define	SLAVE_NO_ELECTRICAL										0x01
#define	SLAVE_NO_ACTUATOR										0x02

#define MASTER_485_CTRL_DEFAULT										0x00
#define MASTER_485_CTRL_ELECTRICAL									0x01
#define MASTER_485_CTRL_ACTUATOR									0x02
#define MASTER_485_CTRL_ELECTRICAL_SYNC								0x03

extern int deal_master_485_packet(modbus_t *ctx ,u8 *p_ctrl, void *arg);


#endif /* WIRED_METER_H_ */
