/*
 * posix_m_q_info.c
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

#define MQ_NAME ("/tmp")
#define MQ_FLAG (O_RDWR | O_CREAT ) // 创建MQ的flag
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // 设定创建MQ的权限

int main() {
	mqd_t posixmq;
	int rc = 0;

	struct mq_attr mqattr;

	// 创建默认属性的消息队列
	posixmq = mq_open(MQ_NAME, MQ_FLAG, FILE_MODE, NULL);
	if (-1 == posixmq) {
		perror("创建MQ失败");
		exit(1);
	}

	// 获取消息队列的默认属性
	rc = mq_getattr(posixmq, &mqattr);
	if (-1 == rc) {
		perror("获取消息队列属性失败");
		exit(1);
	}

	printf("队列阻塞标志位：%ld\n", mqattr.mq_flags);
	printf("队列允许最大消息数：%ld\n", mqattr.mq_maxmsg);
	printf("队列消息最大字节数：%ld\n", mqattr.mq_msgsize);
	printf("队列当前消息条数：%ld\n", mqattr.mq_curmsgs);

	rc = mq_close(posixmq);
	if (0 != rc) {
		perror("关闭失败");
		exit(1);
	}

	rc = mq_unlink(MQ_NAME);
	if (0 != rc) {
		perror("删除失败");
		exit(1);
	}
	return 0;
}

