/*
 * modbus-lock.h
 *
 *  Created on: Oct 9, 2014
 *      Author: ygz
 */

#ifndef MODBUS_LOCK_H_
#define MODBUS_LOCK_H_

enum {
	DATA_PROCESS_MODE_READ = 1,
	DATA_PROCESS_MODE_WRITE = 2,
};

extern int lock_modbus_rtu_lock(int addr, int nb, int mode);
extern int lock_modbus_rtu_unlock(int addr, int nb, int mode);

#endif /* MODBUS_LOCK_H_ */
