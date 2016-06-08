/*
 * data.c
 *
 *  Created on: Oct 1, 2014
 *      Author: abc
 */

#include <stdlib.h>
#include "../config.h"
#include "cJSON/cJSON.h"
#include "modbus-lock.h"
#include "data.h"
#include "sqlite/sqlite3.h"
#include <stdint.h>
#include "zlib.h"
#include "common/common.h"
#include "map.h"
#include <fcntl.h>
#include <time.h>

//#define HISTORY_DATA_TIMEOUT    (2 * 60)
#define DB_PATH                 "/usr/share/data.db"       //历史数据库路径

#if 1
#define HISTORY_DATA_TIMEOUT    (10 * 60)  //历史数据存储间隔
#define MAX_H_DATA_ITEM_LEN     8000
#define MAX_H_DATA_DEL_ITEM_LEN 2000
#else
#define HISTORY_DATA_TIMEOUT    5  //历史数据存储间隔
#define MAX_H_DATA_ITEM_LEN     8000//60
#define MAX_H_DATA_DEL_ITEM_LEN 2000//20
#endif

#ifndef MAX_WELL_NUM
#define MAX_WELL_NUM  8
#endif

/* 数据库初始化 */
int data_init(Rtu_a118_t *self) {
	sqlite3 *db;
	int rc;
	rc = sqlite3_open(DB_PATH, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		self->db = NULL;
		return -1;
	}
	self->db = db;
#if 0
    /* 实时数据断电恢复 */
	char* compress_buf = NULL;
	uLong blen;
	uLong tlen = self->map.x_max * MAX_WELL_NUM * sizeof(uint16_t);
	uint16_t *buf = (uint16_t *)malloc(tlen);
	char time_s[64];
	if(NULL == buf) {
		return 0;
	}
	char *sql = "select time, data from data order by id DESC limit 0, 1;";

	blen = compressBound(tlen);	/* 压缩后的长度是不会超过blen的 */
	if((compress_buf = (char*)malloc(sizeof(char) * blen)) == NULL)
	{
		printf("no enough memory!\n");
		free(buf);
		return 0;
	}
	printf("blen %d\n", blen);
	if(-1 == sqlite_data_read_bin_data(db, sql, compress_buf, &tlen, time_s)) {
		free(compress_buf);
		free(buf);
		perror("sqlite_data_read_bin_data error");
		return 0;
	}
	printf("tlen %d\n", tlen);
	if(uncompress((Bytef *)buf, &blen, compress_buf, tlen) != Z_OK)
	{
		printf("uncompress failed!\n");
		free(compress_buf);
		free(buf);
		return 0;
	}
	printf("blen %d %s\n", blen, time_s);
	memcpy(self->map.ram_x, buf, self->map.x_max * MAX_WELL_NUM * sizeof(uint16_t));
	free(compress_buf);
	free(buf);
#endif
	printf("data init ok\n");
	return 0;
}

/* 数据存储线程 */
void *thread_data(void *arg) {
    /* 加载配置 */
	char *name = "null";
	char *db_path = "./data.db";

	cJSON *config = (cJSON *)arg;
	if(NULL == config) {
		printf("cJSON_Parse error\n");
		return NULL;
	}
	cJSON *leaf = cJSON_GetObjectItem(config, "name");
	if(NULL != leaf && cJSON_String == leaf->type) {
		name = leaf->valuestring;
	} else {
		printf("no name config\n");
		return NULL;
	}

	leaf = cJSON_GetObjectItem(config, "db_path");
	if(NULL != leaf && cJSON_String == leaf->type) {
		db_path = leaf->valuestring;
	} else {
		printf("no da_path config\n");
	}
	if(0 == strcmp("data", name)) {

	} else {
		printf("name is not data\n");
		return NULL;
	}

	printf("[thread] start %s %s\n", name, db_path);

	int rc = 0;
	int i = 0;
	int time_count = 0;
	Rtu_a118_t *self = &rtu_a118_t;
	sqlite3 *db;
	int msg_fd = self->data_msg_fd;
	fd_set fdset;
	fd_set fdset_a;
	FD_ZERO(&fdset);
	if(msg_fd > 0) {
		FD_SET(msg_fd, &fdset);
	} else {
		return (void *)(-1);
	}

	struct timeval timeout = {1, 0};
	struct timeval timeout_a = timeout;
	time_t time0 = time(NULL);
	//char *zErrMsg = 0;

	uint16_t first_data_time[6];

	if (db_path == NULL) {
		fprintf(stderr, "Usage: DATABASE SQL-STATEMENT\n");
		return (void *)(1);
	}
#if 0
	rc = sqlite3_open(db_path, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return (void *)(1);
	}
#endif
	if(NULL == self->db) {
		return (void *)(1);
	}
	db = self->db;

	char* compress_buf = NULL;
	uLong blen;
	uLong tlen = self->map.x_max * MAX_WELL_NUM * sizeof(uint16_t);
	uint16_t *buf = (uint16_t *)malloc(tlen);
	if(NULL == buf) {
		return (void *)(-1);
	}

	char *sql_first = "select time from data order by id ASC limit 0, 1;";
	char time_s_first[64];
	if(-1 == sqlite_data_read_bin_data(db, sql_first, NULL, NULL, time_s_first)) {
		//return (void *)(-1);
		sprintf(time_s_first, "%hd-%hd-%hd %hd:%hd:%hd", 2015, 1, 1, 0, 0, 0);
	}

	sscanf(time_s_first, "%hd-%hd-%hd %hd:%hd:%hd"
			,first_data_time
			,first_data_time + 1
			,first_data_time + 2
			,first_data_time + 3
			,first_data_time + 4
			,first_data_time + 5);
	*first_data_time = int16_to_bcd(*first_data_time);
	*(first_data_time + 1) = int16_to_bcd(*(first_data_time + 1));
	*(first_data_time + 2) = int16_to_bcd(*(first_data_time + 2));
	*(first_data_time + 3) = int16_to_bcd(*(first_data_time + 3));
	*(first_data_time + 4) = int16_to_bcd(*(first_data_time + 4));

	char *sql_h_data_len = "select count(*) from data;";
	char h_data_len_first[64] = "";
	int h_data_len = 0;
	if(-1 == sqlite_data_read_bin_data(db, sql_h_data_len, NULL, NULL, h_data_len_first)) {
		return (void *)(-1);
	}
	h_data_len = atoi(h_data_len_first);
	printf("current history data len %d\n", h_data_len);

    /* 消息处理函数 
     * fd in 消息队列描述符
     * msg in 消息体
     */
	int msg_handler(int fd, char *msg) {
		if(MSG_REQUEST_H_DATA == *(int *)msg) {
            /* 查询数据库, 解压数据 */
			blen = compressBound(tlen);	/* 压缩后的长度是不会超过blen的 */
			if((compress_buf = (char*)malloc(sizeof(char) * blen)) == NULL)
			{
				printf("no enough memory!\n");
				return -1;
			}
			int well_num = *(int *)(msg + 4);
			printf("blen %lu well_num %d\n", blen, well_num);
			char sql[128];
			uint16_t r_date_time[6];
			printf("well_num %d\n", *(int *)(msg + 4));
			map_modbus_write_registers(first_data_time, 5, well_num, 7006, 0);
			map_modbus_read_registers(r_date_time, sizeof(r_date_time), well_num, 7000 - 1, 0);
			sprintf(sql, "select time, data from data where time>'%04hd-%02hd-%02hd %02hd:%02hd:00' order by id ASC limit %hd, 1;"
				,bcd_to_int16(r_date_time[0])
				,bcd_to_int16(r_date_time[1])
				,bcd_to_int16(r_date_time[2])
				,bcd_to_int16(r_date_time[3])
				,bcd_to_int16(r_date_time[4])
				,r_date_time[5]);
			printf("sql %s\n", sql);
			log_msg("sql %s\n", sql);
			char time_s[64];
			if(-1 == sqlite_data_read_bin_data(db, sql, compress_buf, (int *)&tlen, time_s)) {
				free(compress_buf);
				return -1;
			}
			uint16_t time_u16[6];
			printf("%s\n", time_s);
			sscanf(time_s, "%hd-%hd-%hd %hd:%hd:%hd"
					,time_u16
					,time_u16 + 1
					,time_u16 + 2
					,time_u16 + 3
					,time_u16 + 4
					,time_u16 + 5);
			*time_u16 = int16_to_bcd(*time_u16);
			*(time_u16 + 1) = int16_to_bcd(*(time_u16 + 1));
			*(time_u16 + 2) = int16_to_bcd(*(time_u16 + 2));
			*(time_u16 + 3) = int16_to_bcd(*(time_u16 + 3));
			*(time_u16 + 4) = int16_to_bcd(*(time_u16 + 4));
			*(time_u16 + 5) = int16_to_bcd(*(time_u16 + 5));

			printf("year %hx\n", *time_u16);
			log_msg("year %04hx-%02hx-%02hx %02hx:%02hx:%02hx well %d %d\n", 
				*time_u16, 
				*(time_u16 + 1), 
				*(time_u16 + 2), 
				*(time_u16 + 3), 
				*(time_u16 + 4), 
				*(time_u16 + 5), 
				*(int *)(msg + 4),
				h_data_len);

			if(uncompress((Bytef *)buf, &blen, (const Bytef *)compress_buf, tlen) != Z_OK)
			{
				printf("uncompress failed!\n");
				return -1;
			}
			free(compress_buf);

			well_num = *(int *)(msg + 4);
			printf("well_num %d %d\n", well_num, *(int *)(msg + 4));
#if 0
			first_data_time[0] = int16_to_bcd(2014);
			first_data_time[1] = int16_to_bcd(12);
			first_data_time[2] = int16_to_bcd(12);
			first_data_time[3] = int16_to_bcd(12);
			first_data_time[4] = int16_to_bcd(12);
#endif

            //填写寄存器
			map_modbus_write_registers(time_u16, 5, well_num, 7014, 0);
			if(-1 == map_modbus_write_data_registers(buf + self->map.x_max * (well_num - 1), 
				self->map.x_max, well_num, 7019, 0)) {
				printf("self->map.x_max %d\n", self->map.x_max);
			}
		}
		return 0;
	}

	while(1) {
		update_thread_time(THREAD_DATA_SERVER);
		fdset_a = fdset;
		timeout_a = timeout;
		rc = select(msg_fd + 1, &fdset_a, NULL, NULL, &timeout_a);
		if(rc > 0) {
			if(FD_ISSET(msg_fd, &fdset_a)) {
				pmsg_q_receive(msg_fd, self->data_msg_buf, 0, NULL);
				msg_handler(msg_fd, self->data_msg_buf);
			}
		}
		if(time(NULL) - HISTORY_DATA_TIMEOUT < time0 && time_count < HISTORY_DATA_TIMEOUT) {
			++time_count;
			continue;
		}
        //存储时间已到
		time_count = 0;
		time(&time0);
		printf("insert data to database\n");
		memcpy(buf, self->map.ram_x, tlen);
		++i;
        //压缩数据并存储
		blen = compressBound(tlen);	/* 压缩后的长度是不会超过blen的 */
		if((compress_buf = (char*)malloc(sizeof(char) * blen)) == NULL)
		{
			printf("no enough memory!\n");
			continue;
		}
		printf("blen %lu\n", blen);

		/* 压缩 */
		if(compress((Bytef *)compress_buf, &blen, (Bytef *)buf, tlen) != Z_OK)
		{
			printf("compress failed!\n");
			free(compress_buf);
			continue;
		}
		printf("blen %lu\n", blen);

//		if(uncompress(buf, &tlen, compress_buf, blen) != Z_OK)
//		{
//			printf("uncompress failed!\n");
//			return -1;
//		}
//		printf("tlen %d\n", tlen);
//		data_compress(buf, RTU_MAX_WELL_NUM, bufout, len);
//		sqlite_data_store_bin(db_path, buf, sizeof(buf));
		if(-1 == sqlite_data_store_bin_data(db, compress_buf, blen)) {
			
		} else {
			update_thread_com_time(THREAD_DATA_SERVER);
		}
		free(compress_buf);
		++h_data_len;
		printf("current history data len %d\n", h_data_len);
		log_msg("count of data %d\n", h_data_len);
#if 1
		if(h_data_len > MAX_H_DATA_ITEM_LEN) {
			char del_sql_buf[512];
			sprintf(del_sql_buf, "delete from data where id in (select id from data order by id limit 0,%d);",
				MAX_H_DATA_DEL_ITEM_LEN);
			printf("delete sql %s\n", del_sql_buf);
			if(-1 == sqlite_data_read_bin_data(db, del_sql_buf, NULL, NULL, NULL)) {
				//return (void *)(-1);
			} else {
				h_data_len -= MAX_H_DATA_DEL_ITEM_LEN;
				printf("delete ok\n");
				if(-1 == sqlite_data_read_bin_data(db, sql_first, NULL, NULL, time_s_first)) {
					//return (void *)(-1);
					sprintf(time_s_first, "%hd-%hd-%hd %hd:%hd:%hd", 2015, 1, 1, 0, 0, 0);
				}

				sscanf(time_s_first, "%hd-%hd-%hd %hd:%hd:%hd"
						,first_data_time
						,first_data_time + 1
						,first_data_time + 2
						,first_data_time + 3
						,first_data_time + 4
						,first_data_time + 5);
				*first_data_time = int16_to_bcd(*first_data_time);
				*(first_data_time + 1) = int16_to_bcd(*(first_data_time + 1));
				*(first_data_time + 2) = int16_to_bcd(*(first_data_time + 2));
				*(first_data_time + 3) = int16_to_bcd(*(first_data_time + 3));
				*(first_data_time + 4) = int16_to_bcd(*(first_data_time + 4));
				*(first_data_time + 5) = int16_to_bcd(*(first_data_time + 5));
				log_msg("first data %hx-%hx-%hx %hx:%hx:%hx %d\n", 
					*first_data_time,
					*(first_data_time + 1),
					*(first_data_time + 2),
					*(first_data_time + 3),
					*(first_data_time + 4),
					*(first_data_time + 5),
					h_data_len);
			}
		}
#endif
		printf("current history data len %d\n", h_data_len);
//		sqlite_data_store_sql(db_path,
//				"insert into data (time, data) values "
//				"(datetime('now', 'localtime'), 'hello world');");
//		int len = 0;
//		sqlite_data_read_bin(db_path, buf, &len);
	}
//	sqlite3_close(db);
	return 0;
}
