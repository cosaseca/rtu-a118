/*
 * posix_m_q.c
 *
 *  Created on: Oct 2, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>   //头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MQ_NAME ("/tmp")
#define MQ_FLAG (O_RDWR | O_CREAT | O_EXCL) // 创建MQ的flag
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) // 设定创建MQ的权限

int main()

{
	mqd_t posixmq;
	int rc = 0;

	/*
	 函数说明：函数创建或打开一个消息队列
	 返回值：成功返回消息队列描述符，失败返回-1，错误原因存于errno中
	 */
	posixmq = mq_open(MQ_NAME, MQ_FLAG, FILE_MODE, NULL);

	if (-1 == posixmq) {
		perror("创建MQ失败");
		exit(1);
	}

	/*
	 函数说明：关闭一个打开的消息队列，表示本进程不再对该消息队列读写
	 返回值：成功返回0，失败返回-1，错误原因存于errno中
	 */
	rc = mq_close(posixmq);
	if (0 != rc) {
		perror("关闭失败");
		exit(1);
	}

	/*
	 函数说明：删除一个消息队列，好比删除一个文件，其他进程再也无法访问
	 返回值：成功返回0，失败返回-1，错误原因存于errno中
	 */
	rc = mq_unlink(MQ_NAME);
	if (0 != rc) {
		perror("删除失败");
		exit(1);
	}

	return 0;
}

