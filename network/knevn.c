#if 0
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>  
#include <linux/netlink.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "config.h"
#include "data/data.h"
#if 1
#define MAX_PAYLOAD       1024
static struct sockaddr_nl src_addr, dest_addr;
static char *KernelMsg = NULL;
static struct iovec iov;
static int sock_fd;
static struct msghdr msg;
static int msglen;
static int USBKeyboardReady;

#define DEVICE_ADD             "add"
#define DEVICE_REMOVE          "remove"
#define DEVICE_NAME            "event0"
#define DEVICE_NAMELEN         6
#define NETLINK_TEST           25
#endif
void *thread_knevn(void *arg)
{
#if 1
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    int msgle;
    
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = pthread_self() << 16 | getpid();
    src_addr.nl_groups = 1;
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    
    memset(&dest_addr, 0, sizeof(dest_addr));
    KernelMsg = (char *)malloc(MAX_PAYLOAD);
    memset(KernelMsg, 0,     MAX_PAYLOAD);
    
    iov.iov_base = (void *)KernelMsg;
    iov.iov_len = MAX_PAYLOAD;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    while(1) {
        printf("Waiting for message from kernel\n");
//        sleep(1);
        recvmsg(sock_fd, &msg, 0);
        printf("Receved message: %d %d\n", *(int *)NLMSG_DATA(KernelMsg), *((int *)NLMSG_DATA(KernelMsg) + 1));

//      {GPIO_TO_PIN(0, 7), -1},
//		{GPIO_TO_PIN(1, 15), -1},
//		{GPIO_TO_PIN(1, 14), -1},
//		{GPIO_TO_PIN(1, 13), -1},
//		{GPIO_TO_PIN(1, 12), -1},
//		{GPIO_TO_PIN(0, 27), -1},
//		{GPIO_TO_PIN(0, 26), -1},
//		{GPIO_TO_PIN(0, 23), -1},
//		{GPIO_TO_PIN(0, 22), -1},
//		{GPIO_TO_PIN(0, 31)}

//        zigbee_write_registers(NLMSG_DATA(KernelMsg), 4, 1, 302);
//        printf("Receved message payload: %s\n", KernelMsg);
//        msglen = strlen(KernelMsg);
//        printf("Device: %s\n", KernelMsg+msglen-DEVICE_NAMELEN);
//        if(!strncmp(DEVICE_NAME, KernelMsg+msglen-DEVICE_NAMELEN, DEVICE_NAMELEN)) {
//            if(!strncmp(DEVICE_ADD, KernelMsg, strlen(DEVICE_ADD)))
//            {
//                printf("Add event0 device\n");
//                USBKeyboardReady = 1;
//            }
//            else if(!strncmp(DEVICE_REMOVE, KernelMsg, strlen(DEVICE_REMOVE))) {
//                printf("Remove event0 device\n");
//                USBKeyboardReady = 0;
//            }
//        }
    }
    close(sock_fd);
    return NULL;
#endif
}

#if 0
int ithread_start(void *(run)(void *), void *arg) {
	pthread_t threadId;
	pthread_attr_t threadAttr;
	memset(&threadAttr,0,sizeof(pthread_attr_t));
	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
	int result = pthread_create(&threadId, &threadAttr, run, arg);
	if(0 != result) {
		perror("pthread_create");
	}
	pthread_attr_destroy(&threadAttr);
	printf("thread id %ld\n", threadId);
	return threadId;
}
int main() {
  ithread_start(DeviceManagement, NULL);
  while(1) {
    sleep(10);
  }
  return 0;
}
#endif
#endif
