/*
 * modbus_485.h
 *
 *  Created on: Oct 8, 2014
 *      Author: abc
 */

#ifndef MODBUS_485_H_
#define MODBUS_485_H_


extern void *thread_modbus485_slave(void *arg);
extern void *thread_modbus485_master(void *arg);

extern int init_port_info(uint16_t *port_info, int *baud, char *parity,
		int *data_bit, int *stop_bit);
extern int timer_sleep(struct timeval *t1);


#endif /* MODBUS_485_H_ */
