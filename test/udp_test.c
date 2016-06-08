/*
 * udp_test.c
 *
 *  Created on: Nov 25, 2014
 *      Author: ygz
 */



#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define PORT 18001
#define SERVER_IP "0.0.0.0"
main()
{
int s,len;
struct sockaddr_in addr;
int addr_len=sizeof(struct sockaddr_in);
char buffer[256];
/*建立socket*/
if((s=socket(AF_INET,SOCK_DGRAM,0))<0){
perror("socket");
exit(1);
}
/*填写sockaddr_in*/
bzero(&addr,sizeof(addr));
addr.sin_family=AF_INET;
addr.sin_port=htons(PORT);
addr.sin_addr.s_addr=inet_addr(SERVER_IP);
bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
while(1){
//bzero(buffer,sizeof(buffer));
/*从标准输入设备取得字符串*/
//len=read(STDIN_FILENO,buffer,sizeof(buffer));
/*将字符串传送给server端*/
//sendto(s,buffer,len,0,(struct sockaddr*)&addr,addr_len);
/*接收server端返回的字符串*/
	printf("len %d\n", len);
len=recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr*)&addr,&addr_len);
//printf("receive:%s",buffer);
printf("len %d %d\n", len, addr_len);
sendto(s, "1212", 2, 0, &addr, addr_len);
}
}
