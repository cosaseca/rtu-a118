/**	
   @file thread.c
   @brief 线程公用函数
   @note
   author: yangguozheng
   date: 2013-09-16
   mcdify record: creat this file.
   author: yangguozheng
   date: 2013-09-17
   modify record: add a function
 */
#include "common/thread.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

/**
   @brief 创建线程
   @author yangguozheng 
   @param[in] run 线程入口
   @return 失败: -1，成功: 线程标识
   @note
 */
#if 1
int thread_start(void *(run)(void *)) {
	int result = ithread_start(run, NULL);
	return result;
}
#endif

/**
   @brief 创建线程底层新封装
   @author yangguozheng 
   @param[in] run 线程入口
   @param[in] arg 线程参数
   @return 失败: -1，成功: 线程标识
   @note
 */
#if 0
int ithread_start(void *(run)(void *), void *arg) {
	pthread_t threadId;
	pthread_attr_t threadAttr;
	int rc = -1;
	memset(&threadAttr,0,sizeof(pthread_attr_t));
	rc = pthread_attr_init(&threadAttr);
	printf("pthread_attr_init %d\n", rc);
	rc = pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
	printf("pthread_attr_setdetachstate %d\n", rc);
	int result = pthread_create(&threadId, &threadAttr, run, arg);
	printf("pthread_create %d\n", result);
	if(0 != result) {
		perror("pthread_create");
	}
	rc = pthread_attr_destroy(&threadAttr);
	printf("pthread_attr_destroy %d\n", rc);
	printf("thread id %ld\n", threadId);
	return threadId;
}
#endif
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
	//printf("thread id %lx\n", threadId);
	return threadId;
}

int ithread_start_with_name(void *(run)(void *), void *arg, const char *name) {
	int rc = ithread_start(run, arg);
	if(-1 == rc) {
		perror("ithread_start %s error", name);
	} else {
		printf("ithread_start %s ok\n", name);
	}
	return rc;
}

/**
   @brief 线程同步
   @author yangguozheng 
   @param[in] thread_r 当前线程同步变量
   @param[in] process_r 要等待的同步变量
   @return 
   @note
     阻塞
 */
void thread_synchronization(volatile int *thread_r, volatile int *process_r) {
	*thread_r = 1;
	while(*process_r == 0) {
		thread_usleep(0);
	}
	thread_usleep(0);
}

/**
   @brief 线程睡眠
   @author yangguozheng 
   @param[in] micro_seconds 睡眠时间，毫秒
   @return 
   @note
 */
void thread_usleep(int micro_seconds) {
	usleep(micro_seconds);
}

/**
   @brief host_thread_synchronization
   @author  
   @param[in] r_thread 
   @param[in] r_host 
   @return 
   @note
 */
void host_thread_synchronization(volatile int *r_thread, volatile int *r_host){
	while (1) {
		if (*r_thread == 1) {
//			printf("rHost%d\n",*r_host);
			*r_host = 1;
			break;
		}
		thread_usleep(0);
	}
	thread_usleep(0);
}

void update_thread_time(int id) {
	if(id < 0 || id >= THREAD_MAX_THREAD_NUM) {
		return;
	}
	time(rtu_a118_t.thread_time + id);
}

void update_thread_com_time(int id) {
	if(id < 0 || id >= THREAD_MAX_THREAD_NUM) {
		return;
	}
	time(rtu_a118_t.thread_com_time + id);
}

void thread_time_init() {
	int i = 0;
	time_t tm0 = time(NULL);
	for(;i < THREAD_MAX_THREAD_NUM;++i) {
		rtu_a118_t.thread_time[i] = tm0;
		rtu_a118_t.thread_com_time[i] = tm0;
	}
}


void test_thread_time() {
	int i = 0;
	time_t tm0 = time(NULL);
	char buf[512] = "";
	int rc0 = 0;
	int rc1 = 0;
	for(;i < THREAD_MAX_THREAD_NUM;++i) {
		if(tm0 - rtu_a118_t.thread_time[i] > THREAD_MAX_DELAY_TIME) {
			rc1 = sprintf(buf + rc0, "%d ", i);
			if(rc1 > 0) {
				rc0 += rc1;
			}
		}
	}
	if(rc0 > 0) {
		log_msg("thread slow %s\n", buf);
	}
}


