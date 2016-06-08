/*
 * pmsg_q.c
 *
 *  Created on: Oct 14, 2014
 *      Author: ygz
 */

#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../config.h"

#define MQ_FLAG (O_RDWR | O_CREAT | O_NONBLOCK )          // 创建MQ的flag
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // 设定创建MQ的权限
#define PMSG_ITEM_MAX_SIZE   MAX_MSG_BUF_SIZE
#define PMSG_MAXMSG          10

/* 创建消息队列 */
int pmsg_q_open(const char *path) {
	struct mq_attr attr = {
		.mq_flags = O_NONBLOCK,
		.mq_maxmsg = PMSG_MAXMSG,
		.mq_msgsize = PMSG_ITEM_MAX_SIZE,
	};
	int rc = mq_open(path, MQ_FLAG, FILE_MODE, &attr);
	if(-1 == rc) {
		perror("mq_open");
	}
	return rc;
}

/* 关闭消息队列 */
int pmsg_q_close(int mfd) {
	int rc = mq_close(mfd);
	if(-1 == rc) {
		perror("mq_close");
	}
	return rc;
}

/* 删除消息队列 */
int pmsg_q_rm(const char *path) {
	int rc = mq_unlink(path);
	if(-1 == rc) {
		perror("mq_unlink");
	}
	return rc;
}

/**
 * 接收消息
 * mfd in 消息队列描述符
 * msg out 消息体
 * len in 消息长度
 * prio in 优先级
 * return {-1: 失败, n>0: 成功}
 */
int pmsg_q_receive(int mfd, void *msg, size_t len, unsigned int *prio) {
	struct mq_attr attr;
	int rc;
	rc = mq_getattr(mfd, &attr);
	printf("%ld\n", attr.mq_msgsize);
	rc = mq_receive(mfd, msg, attr.mq_msgsize, prio);
	if(-1 == rc) {
		perror("mq_receive");
	} else {
		int *msg_data = (int *)msg;
		log_msg("%d %d %08x %08x %08x %08x %08x %08x %08x %08x", 
			mfd, rc,
			*msg_data, *(msg_data + 1), 
			*(msg_data + 2), *(msg_data + 3),
			*(msg_data + 4), *(msg_data + 5),
			*(msg_data + 6), *(msg_data + 7));
	}
	return rc;
}

/**
 * 发送消息
 * mfd in 消息队列描述符
 * msg in 消息体
 * len in 消息长度
 * prio in 优先级
 * return {-1: 失败, n>0: 成功}
 */
int pmsg_q_send(int mfd, void *msg, size_t len, unsigned int prio) {
	printf("mfd: %d len: %u\n", mfd, len);
	struct mq_attr attr;
	mq_getattr(mfd, &attr);
	printf("attr %ld %ld %ld %ld\n", attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
	//struct timespec abs_timeout = {0, 100};
	//int rc = mq_timedsend(mfd, msg, len, prio, &abs_timeout);
	int rc = mq_send(mfd, msg, len, prio);
	if(-1 == rc) {
		perror("mq_send");
	} else {
		int msg_data[8];
		len = len>8*sizeof(int)?8*sizeof(int):len;
		memcpy(msg_data, msg, len);
		log_msg("%d %d %08x %08x %08x %08x %08x %08x %08x %08x", 
			mfd, len,
			*msg_data, *(msg_data + 1), 
			*(msg_data + 2), *(msg_data + 3),
			*(msg_data + 4), *(msg_data + 5),
			*(msg_data + 6), *(msg_data + 7));
	}
	return rc;
}


