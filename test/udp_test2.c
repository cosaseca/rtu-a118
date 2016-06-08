/*
 * udp_test2.c
 *
 *  Created on: Nov 25, 2014
 *      Author: ygz
 */




#include <stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <netdb.h>

int main(int argc,char* argv[])

{



//创建套接字//

struct addrinfo hints;

struct addrinfo *res;

memset(&hints,0,sizeof(hints));

hints.ai_family=AF_INET6;

hints.ai_socktype=SOCK_DGRAM;

hints.ai_protocol=IPPROTO_UDP;

hints.ai_flags=AI_NUMERICHOST;

getaddrinfo("::","18001",&hints,&res);

int s;

s=socket(res->ai_family,res->ai_socktype,res->ai_protocol);

//绑定套接字//

bind(s,res->ai_addr,res->ai_addrlen);

char buf[1024];

int len;

struct sockaddr_in6 sin;

len=sizeof(sin);
printf("aaaa\n");

while(1) {
recvfrom(s,buf,sizeof(buf),0,(struct sockaddr*)&sin,&len);//closesocket(s);
printf("bbb\n");
sendto(s, "12121", 2, 0, &sin, len);
}
}
