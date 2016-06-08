/*
 * posix_m_q_snd.c
 *
 *  Created on: Oct 2, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/*读取某消息队列,消息队列名通过参数传递*/
int main(int argc, char *argv[]) {
	mqd_t mqd;
	struct mq_attr attr;
	char *ptr;
	unsigned int prio;
	size_t n;
	int rc;

	if (argc != 2) {
		printf("Usage: readmq <name>\n");
		exit(1);
	}

	/*只读模式打开消息队列*/
	mqd = mq_open(argv[1], O_RDONLY);
	if (mqd < 0) {
		perror("打开消息队列失败");
		exit(1);
	}

	// 取得消息队列属性，根据mq_msgsize动态申请内存
	rc = mq_getattr(mqd, &attr);
	if (rc < 0) {
		perror("取得消息队列属性失败");
		exit(1);
	}

	/*动态申请保证能存放单条消息的内存*/
	ptr = calloc(attr.mq_msgsize, sizeof(char));
	if (NULL == ptr) {
		printf("动态申请内存失败\n");
		mq_close(mqd);
		exit(1);
	}

	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(mqd, &fdset);

	fd_set rdset;

	struct timeval s_timeval = {1, 0};

	while(1) {
		rdset = fdset;
		s_timeval.tv_sec = 1;
		s_timeval.tv_usec = 0;
		rc = select(mqd+1, &rdset, NULL, NULL, &s_timeval);
		switch(rc) {
			case -1:{
				perror("select\n");
				exit(1);
				break;
			}
			case 0:{
				printf("select rc %d\n", rc);
				break;
			}
			default:{
				/*接收一条消息*/
				n = mq_receive(mqd, ptr, attr.mq_msgsize, &prio);
				if (n < 0) {
					perror("读取失败");
					mq_close(mqd);
					free(ptr);
					exit(1);
				}
				printf("读取 %ld 字节\n  优先级为 %u\n", (long) n, prio);
			}
		}
	}

	return 0;
}
