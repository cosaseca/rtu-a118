/*
 * pressure_meter_analysis.h
 *
 *  Created on: Oct 8, 2014
 *      Author: caspar
 */

#ifndef PRESSURE_METER_ANALYSIS_H_
#define PRESSURE_METER_ANALYSIS_H_

#include "common/common.h"

#include "meter/meter.h"

#include "libmodbus-3.0.6/src/modbus-private.h"




extern int8 analysis_pressure(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
extern int8 analysis_temperature(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
extern int8 analysis_torque_speed_load(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
extern int8 analysis_flow_meter(modbus_t* ctx, u8* p_mac, u8* p_buff,
		int8 (*func)(modbus_t*, u8*, u8*, u8));
#endif /* PRESSURE_METER_ANALYSIS_H_ */
