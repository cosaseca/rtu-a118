/*
 * rtu_log.c
 *
 *  Created on: Jan 6, 2015
 *      Author: ygz
 */
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <error.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

extern unsigned long get_file_size(const char *path);
extern int flash_map(const char *path, int len, void **flash);

void log_printf(const char* format, ...);
//#define printf(format, arg...) fprintf(stdout, "%s %d: "format, __FUNCTION__, __LINE__, ##arg)
#define perror(format, arg...) \
		fprintf(stderr, "%s %d: "format" ERROR %d %s\n", __FUNCTION__, __LINE__, ##arg, errno, strerror(errno))

#define printf(format, arg...) log_printf("%s %d: "format, __FUNCTION__, __LINE__, ##arg)

#define RTU_LOGFILE_PATH        "/var/www/rtu_log"
#define RTU_INFO_PATH           "/var/www/rtu_info"
//#define RTU_LOGFILE_PATH       "./rtu_log.txt"

#define RTU_LOG_MAX_BUF_SIZE   1024
#define RTU_LOG_MAX_FILE_SIZE  (1024 * 1024)

typedef struct Log_t {
	pthread_mutex_t mutex;
	char buf[RTU_LOG_MAX_BUF_SIZE];
	char *flash;
	int flen;
	pthread_mutex_t info_mutex;
	char info_buf[RTU_LOG_MAX_BUF_SIZE];
	FILE *info_file;
}Log_t;

Log_t log_t = {
		.mutex = PTHREAD_MUTEX_INITIALIZER,
		.flen = 0,
		.flash = MAP_FAILED,
};

#if 0
unsigned long get_file_size(const char *path)
{
	unsigned long filesize = -1;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

/*
 * 创建flash
 * path in 路径
 * len in 长度
 * flash_is_new out flash首次初始化标识
 */
int flash_create(const char *path, int len, char *flash_is_new) {
	void *flash_init_buf;
	FILE *flash_file;
	int flash_fd;
	if(0 != access(path, 0)
			|| (len != get_file_size(path))) {
		*flash_is_new = 1;
		flash_init_buf = malloc(len);
		if(NULL == flash_init_buf) {
			perror("flash_init_buf=NULL");
			return -1;
		}
		memset(flash_init_buf, -1, len);
		flash_file = fopen(path, "w");
		if(NULL == flash_file) {
			perror("flash_file=NULL");
			return -1;
		}
		if(1 != fwrite(flash_init_buf, len, 1, flash_file)) {
			perror("flash fwrite error");
			return -1;
		}
		fclose(flash_file);
		free(flash_init_buf);
		printf("create flash %s\n", path);
	}
	return 0;
}

/*
 * flash 内存映射
 */
int flash_map(const char *path, int len, void **flash) {
	int fd = open(path, O_RDWR | O_SYNC);
	if(-1 == fd) {
		perror("flash_map fd=-1");
		return -1;
	}
	*flash = mmap(NULL, len ,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(MAP_FAILED == *flash) {
		perror("flash map error");
		return -1;
	}
	close(fd);
	return 0;
};

int ithread_start(void *(run)(void *), void *arg) {
	pthread_t threadId;
	pthread_attr_t threadAttr;
	int rc = -1;
	memset(&threadAttr,0,sizeof(pthread_attr_t));
	rc = pthread_attr_init(&threadAttr);
	if(0 != rc) {
		perror("pthread_attr_init");
		return -1;
	}
	rc = pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
	if(0 != rc) {
		perror("pthread_attr_setdetachstate");
		rc = pthread_attr_destroy(&threadAttr);
		if(0 != rc) {
			perror("pthread_attr_destroy");
			return -1;
		}
		return -1;
	}
	rc = pthread_create(&threadId, &threadAttr, run, arg);
	if(0 != rc) {
		perror("pthread_create");
		rc = pthread_attr_destroy(&threadAttr);
		if(0 != rc) {
			perror("pthread_attr_destroy");
			return -1;
		}
		return -1;
	}
	rc = pthread_attr_destroy(&threadAttr);
	if(0 != rc) {
		perror("pthread_attr_destroy");
		return -1;
	}
	printf("thread id %lx\n", threadId);
	return threadId;
}
#endif

void log_printf(const char* format, ...) {
	if(NULL == format)
	{
		return;
	}
	int rc = 0;
	time_t tm = time(NULL);
	if(0 != pthread_mutex_lock(&log_t.mutex)) {
		perror("pthread_mutex_lock");
		return;
	}

	if(MAP_FAILED != log_t.flash) {
		va_list list;
		va_start(list, format);
		rc = sprintf(log_t.buf, "@%s ", ctime(&tm));
		rc += vsnprintf(log_t.buf + rc, RTU_LOG_MAX_BUF_SIZE - rc - 1, format, list);
		if(rc > 0) {
			if(log_t.flen + rc >= RTU_LOG_MAX_FILE_SIZE) {
				log_t.flen = 0;
			}
			memcpy(log_t.flash + log_t.flen, log_t.buf, rc);
			log_t.flen += rc;
		}
		va_end(list);
	}

	pthread_mutex_unlock(&log_t.mutex);
}

void info_printf(const char* format, ...) {
	if(NULL == format)
	{
		return;
	}
	int rc = 0;
	if(0 != pthread_mutex_lock(&log_t.info_mutex)) {
		perror("pthread_mutex_lock");
		return;
	}

	if(NULL != log_t.info_file) {
		va_list list;
		va_start(list, format);
		rc = vsnprintf(log_t.info_buf, RTU_LOG_MAX_BUF_SIZE - 1, format, list);
		if(rc > 0) {
			fprintf(log_t.info_file, "%s\r\n", log_t.info_buf);
			fflush(log_t.info_file);
		}
		va_end(list);
	}

	pthread_mutex_unlock(&log_t.info_mutex);
}

static int flash_create_with_value(const char *path, int len, char *flash_is_new, char v) {
	void *flash_init_buf;
	FILE *flash_file;
	if(0 != access(path, 0)
			|| (len != get_file_size(path))) {
		*flash_is_new = 1;
		flash_init_buf = malloc(len);
		if(NULL == flash_init_buf) {
			perror("flash_init_buf=NULL");
			return -1;
		}
		memset(flash_init_buf, v, len);
		flash_file = fopen(path, "w");
		if(NULL == flash_file) {
			perror("flash_file=NULL");
			return -1;
		}
		if(1 != fwrite(flash_init_buf, len, 1, flash_file)) {
			perror("flash fwrite error");
			return -1;
		}
		fclose(flash_file);
		free(flash_init_buf);
		printf("create flash %s\n", path);
	}
	return 0;
}

int log_init() {
	char flash1_is_new;
	if(-1 == flash_create_with_value(RTU_LOGFILE_PATH,
			RTU_LOG_MAX_FILE_SIZE * sizeof(char),
			&flash1_is_new, 'F')) {
		perror("log flash create error %s\n", RTU_LOGFILE_PATH);
		return -1;
	}

	if(-1 == flash_map(RTU_LOGFILE_PATH, RTU_LOG_MAX_FILE_SIZE * sizeof(char),
			(void **)&log_t.flash)) {
		perror("log flash map error %s\n", RTU_LOGFILE_PATH);
		return -1;
	}
	log_t.info_file = fopen(RTU_INFO_PATH, "w");
	if(NULL == log_t.info_file) {
		return -1;
	}
	return 0;
}

void *log_test(void *arg) {
	int a = (int)(arg);
	while(1) {
		log_printf("hello world %d\n", a);
		usleep(100000);
	}
	return NULL;
}

#if 0
int main(int argc, char *argv[]) {
	if(-1 == log_init()) {
		return -1;
	}
	ithread_start(log_test, (void *)1);
	ithread_start(log_test, (void *)2);
	ithread_start(log_test, (void *)3);
	ithread_start(log_test, (void *)4);
	ithread_start(log_test, (void *)5);
	ithread_start(log_test, (void *)6);
	ithread_start(log_test, (void *)7);
	ithread_start(log_test, (void *)8);
	ithread_start(log_test, (void *)9);
	while(1) {
		log_printf("hello world\n");
		usleep(100000);
	}
	return 0;
}
#endif
