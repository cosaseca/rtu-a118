/*
 * config_utils.c
 *
 *  Created on: Oct 10, 2014
 *      Author: ygz
 */

#include <unistd.h>
#include <stdio.h>

/**
 * 加载配置文件, json
 * buf out 缓冲区
 * len in 缓冲区大小
 * path in 配置文件的路径
 * 返回值 {-1: 失败, n>0: 成功}
 */
int config_load_json_file(char *buf, int len, const char *path) {
	FILE *file = fopen(path, "r");
	int rc = -1;
	if(NULL == file) {
		return -1;
	}
	rc = fread(buf, len, 1, file);
	fclose(file);
	return rc;
}

