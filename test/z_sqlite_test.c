/*
 * z_sqlite_test.c
 *
 *  Created on: Nov 22, 2014
 *      Author: ygz
 */
#if 0
#include <stdlib.h>
#include "sqlite/sqlite3.h"
#include "config.h"
#include <zlib.h>

int sqlite_data_read_bin_data(sqlite3 *db, char *data, int *len) {
	char *zErrMsg = 0;
	int rc;

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare(db, "select data from data order by id DESC;", -1, &stmt, 0);
//	printf("rc %d\n", rc);
	rc = sqlite3_step(stmt);
	printf("rc %d\n", rc);
	char *data_v = (char *)sqlite3_column_blob(stmt,0);//得到纪录中的BLOB字段
	int len_v = sqlite3_column_bytes(stmt, 0);//得到字段中数据的长度
	memcpy(data, data_v, len_v);
//	printf("%d\n", len_v);
	if (rc != SQLITE_ROW) {
		fprintf(stderr, "SQL error sqlite3_step\n");
//		sqlite3_free(zErrMsg);
	}
	rc = sqlite3_finalize(stmt);
	return 0;
}

int main(int argc, char *argv[]) {
	char *db_path = "data.db";
	sqlite3 *db;
	char *zErrMsg = 0;
	if (db_path == NULL) {
		fprintf(stderr, "Usage: DATABASE SQL-STATEMENT\n");
		return (void *)(1);
	}
	int rc = sqlite3_open(db_path, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return (void *)(1);
	}
//	sqlite3_soft_heap_limit64(1024 * 1024 * 4);
	char* compress_buf = NULL;
	uLong blen;

	uint16_t buf[RTU_MAX_REAL_REGISTER * RTU_MAX_WELL_NUM];
	uLong tlen = sizeof(buf);

	blen = compressBound(tlen);	/* 压缩后的长度是不会超过blen的 */
	if((compress_buf = (char*)malloc(sizeof(char) * blen)) == NULL)
	{
		printf("no enough memory!\n");
		return -1;
	}
	printf("blen %d\n", blen);
	sqlite_data_read_bin_data(db, compress_buf, &tlen);

//	/* 压缩 */
//	if(compress(compress_buf, &blen, buf, tlen) != Z_OK)
//	{
//		printf("compress failed!\n");
//		free(compress_buf);
//		continue;
//	}
//	printf("blen %d\n", blen);

	if(uncompress(buf, &blen, compress_buf, tlen) != Z_OK)
	{
		printf("uncompress failed!\n");
		return -1;
	}
	printf("blen %d %d %d %d %d\n", blen, *(buf + 1150), *(buf + 1151), *(buf + 1152), *(buf + 1153));
	sqlite3_close(db);
	return 0;
}
#endif
