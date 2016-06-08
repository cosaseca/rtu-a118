/*
 * data.h
 *
 *  Created on: Oct 7, 2014
 *      Author: abc
 */

#ifndef DATA_H_
#define DATA_H_
//#include "unqlite/unqlite.h"
#include "config.h"
#include <stdint.h>

//extern int zigbee_read_registers(uint16_t *buf, int len, int well_num, int addr);
//extern int zigbee_write_registers(uint16_t *buf, int len, int well_num, int addr);
extern void *thread_data(void *arg);
//extern int data_store_action(const char *db_name, const char *key, void *data, unqlite_int64 len);
extern int sqlite_data_store_sql(const char *db_name, const char *sql);

extern int sqlite_data_store_bin(const char *db_name, const char *data, int len);

//extern int data_map_init(Rtu_a118_t *self);

//extern int data_map_map(Rtu_a118_t *self);

extern int data_init(Rtu_a118_t *self);
extern int sqlite_data_read_bin_data(sqlite3 *db, const char *sql, char *data, int *len, char *s_tm);
extern int sqlite_data_store_bin_data(sqlite3 *db, const char *data, int len);
#endif /* DATA_H_ */
