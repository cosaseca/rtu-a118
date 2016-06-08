/*
 * msgreceive.c
 *
 *  Created on: Oct 14, 2014
 *      Author: ygz
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

struct msg_st
{
	long int msg_type;
	char text[BUFSIZ];
};

int main()
{
	int running = 1;
	int msgid = -1;
	struct msg_st data;
	long int msgtype = 0; //注意1

	//建立消息队列
	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	fd_set rdset;
	fd_set fdset;

	FD_ZERO(&fdset);
	printf("before fd set\n");
	FD_SET(msgid, &fdset);// error
	printf("after fd set\n");

	struct timeval timeout = {1, 0};
	int rc;

	//从队列中获取消息，直到遇到end消息为止
	while(running)
	{
		rdset = fdset;

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		rc = select(msgid + 1, &rdset, NULL, NULL, &timeout);

		switch(rc) {
			case -1:{
				perror("select -1");
				break;
			}
			case 0:{
				printf("time out\n");
				break;
			}
			default: {
				if(FD_ISSET(msgid, &rdset)) {
					if(msgrcv(msgid, (void*)&data, BUFSIZ, msgtype, 0) == -1)
					{
						fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
						exit(EXIT_FAILURE);
					}
					printf("You wrote: %s\n",data.text);
					//遇到end结束
					if(strncmp(data.text, "end", 3) == 0)
						running = 0;
				}
			}
		}
	}
	//删除消息队列
	if(msgctl(msgid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}


