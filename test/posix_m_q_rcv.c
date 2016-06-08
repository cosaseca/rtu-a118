/*
 * posix_m_q_rcv.c
 *
 *  Created on: Oct 2, 2014
 *      Author: ygz
 */

/*头文件*/
#include <mqueue.h>

///*返回：若成功则为消息中字节数，若出错则为-1 */
//int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
//
///*返回：若成功则为0， 若出错则为-1*/
//ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
//		unsigned *msg_prio);
//
///*消息队列属性结构体*/
//struct mq_attr {
//	long mq_flags; /* Flags: 0 or O_NONBLOCK */
//	long mq_maxmsg; /* Max. # of messages on queue */
//	long mq_msgsize; /* Max. message size (bytes) */
//	long mq_curmsgs; /* # of messages currently in queue */
//};

#include <stdio.h>
#include <stdlib.h>
//#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MQ_NAME ("/tmp")
#define MQ_FLAG (O_RDWR | O_CREAT) // 创建MQ的flag
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // 设定创建MQ的权限

/*向消息队列发送消息，消息队列名及发送的信息通过参数传递*/
int main(int argc, char *argv[]) {
	mqd_t mqd;
	char *ptr;
	size_t len;
	unsigned int prio;
	int rc;

	if (argc != 4) {
		printf("Usage: sendmq <name> <bytes> <priority>\n");
		exit(1);
	}

	len = atoi(argv[2]);
	prio = atoi(argv[3]);

	//只写模式找开消息队列
//	mqd = mq_open(argv[1], O_WRONLY);
	mqd = mq_open(argv[1], MQ_FLAG, FILE_MODE, NULL);
	if (-1 == mqd) {
		perror("打开消息队列失败");
		exit(1);
	}

	// 动态申请一块内存
	ptr = (char *) calloc(len, sizeof(char));
	if (NULL == ptr) {
		perror("申请内存失败");
		mq_close(mqd);
		exit(1);
	}

	/*向消息队列写入消息，如消息队列满则阻塞，直到消息队列有空闲时再写入*/
	rc = mq_send(mqd, ptr, len, prio);
	if (rc < 0) {
		perror("写入消息队列失败");
		mq_close(mqd);
		exit(1);
	}

	// 释放内存
	free(ptr);
	return 0;
}

