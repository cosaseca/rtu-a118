/*
 * lock.c
 *
 *  Created on: Oct 2, 2014
 *      Author: abc
 */


#include <pthread.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int lock_modbus_rtu_lock(int addr, int nb, int mode) {
	//pthread_mutex_lock(&mutex);
	return 0;
}

int lock_modbus_rtu_unlock(int addr, int nb, int mode) {
	//pthread_mutex_unlock(&mutex);
	return 0;
}
