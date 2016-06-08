/*
 * test.c
 *
 *  Created on: Oct 11, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <string.h>
#include "sqlite/sqlite3.h"
#include "config.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int sqlite_data_read_bin_data(sqlite3 *db, const char *sql, char *data, int *len, char *s_tm) {
	//char *zErrMsg = 0;
	int rc;

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare(db, sql, -1, &stmt, 0);
//	printf("rc %d\n", rc);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW && SQLITE_DONE != rc) {
		fprintf(stderr, "SQL error sqlite3_step %d\n", rc);
		rc = sqlite3_finalize(stmt);
		return -1;
	}
//	printf("rc %d\n", rc);
	if(NULL != s_tm) {
		const unsigned char *s_time = sqlite3_column_text(stmt, 0);
		if(NULL == s_time) {
			rc = sqlite3_finalize(stmt);
			fprintf(stderr, "SQL error sqlite3_step\n");
			return -1;
		}
		strcpy(s_tm, (const char *)s_time);
	}
//	printf("%s\n", s_time);
	if(NULL != data) {
		char *data_v = (char *)sqlite3_column_blob(stmt, 1);//得到纪录中的BLOB字段
		if(NULL == data_v) {
			rc = sqlite3_finalize(stmt);
			fprintf(stderr, "SQL error sqlite3_step\n");
			return -1;
		}
		int len_v = sqlite3_column_bytes(stmt, 1);//得到字段中数据的长度
		memcpy(data, data_v, len_v);
	}
//	*len = len_v;
//	printf("%d\n", len_v);
	if (rc != SQLITE_ROW && SQLITE_DONE != rc) {
		fprintf(stderr, "SQL error sqlite3_step\n");
//		sqlite3_free(zErrMsg);
	}
	rc = sqlite3_finalize(stmt);
	return 0;
}

int sqlite_data_read_bin(const char *db_name, char *data, int *len) {
	sqlite3 *db;
	//char *zErrMsg = 0;
	int rc;

	if (db_name == NULL) {
		fprintf(stderr, "Usage: DATABASE SQL-STATEMENT\n");
		return (1);
	}
	rc = sqlite3_open(db_name, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return (1);
	}

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare(db, "select data from data where time='2014-11-11 11:11:43';", -1, &stmt, 0);
//	printf("rc %d\n", rc);
	rc = sqlite3_step(stmt);
//	printf("rc %d\n", rc);
	char *data_v = (char *)sqlite3_column_blob(stmt,0);//得到纪录中的BLOB字段
	int len_v = sqlite3_column_bytes(stmt, 0);//得到字段中数据的长度
	memcpy(data, data_v, len_v);
//	printf("%d\n", len_v);
	if (rc != SQLITE_ROW) {
		fprintf(stderr, "SQL error sqlite3_step\n");
//		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	return 0;
}

int sqlite_data_store_bin_data(sqlite3 *db, const char *data, int len) {
	int rc;
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare(db,"insert into data (time, data) values (datetime('now', 'localtime'),?);",-1,&stmt,NULL);
//	printf("rc %d\n", rc);
	rc = sqlite3_bind_blob(stmt,1,data,len,NULL);
//	printf("rc %d\n", rc);
	rc = sqlite3_step(stmt);
//	printf("rc %d\n", rc);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "SQL error sqlite3_step\n");
//		sqlite3_free(zErrMsg);
	}
	rc = sqlite3_finalize(stmt);
	printf("sqlite3_finalize %d\n", rc);
	return 0;
}

int sqlite_data_store_bin(const char *db_name, const char *data, int len) {
	sqlite3 *db;
	//char *zErrMsg = 0;
	int rc;

	if (db_name == NULL) {
		fprintf(stderr, "Usage: DATABASE SQL-STATEMENT\n");
		return (1);
	}
	rc = sqlite3_open(db_name, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return (1);
	}

	sqlite3_stmt *stmt;
	rc = sqlite3_prepare(db,"insert into data (time, data) values (datetime('now', 'localtime'),?);",-1,&stmt,NULL);
//	printf("rc %d\n", rc);
	rc = sqlite3_bind_blob(stmt,1,data,len,NULL);
//	printf("rc %d\n", rc);
	rc = sqlite3_step(stmt);
//	printf("rc %d\n", rc);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "SQL error sqlite3_step\n");
//		sqlite3_free(zErrMsg);
	}
	rc = sqlite3_finalize(stmt);
	printf("sqlite3_finalize %d\n", rc);
	sqlite3_close(db);
	return 0;
}

int sqlite_data_store_sql(const char *db_name, const char *sql) {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	if (db_name == NULL) {
		fprintf(stderr, "Usage: DATABASE SQL-STATEMENT\n");
		return (1);
	}
	rc = sqlite3_open(db_name, &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return (1);
	}
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	return 0;
}

