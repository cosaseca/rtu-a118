#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main() {
struct addrinfo hints;

struct addrinfo *res;
struct addrinfo *res2;

struct addrinfo dest;

memset(&hints,0,sizeof(hints));

hints.ai_family=AF_INET6;

hints.ai_socktype=SOCK_DGRAM;

hints.ai_protocol=IPPROTO_UDP;

hints.ai_flags=AI_NUMERICHOST;

getaddrinfo("::ffff:192.168.31.125","10087",&hints,&res);
memcpy(&dest, res, sizeof(dest));

int s;

getaddrinfo("::","10086",&hints,&res2);

s=socket(res2->ai_family,res2->ai_socktype,res2->ai_protocol);

bind(s,res->ai_addr,res->ai_addrlen);

char buf[]="HELLO IPV6";

sendto(s,buf,sizeof(buf),0,dest.ai_addr,dest.ai_addrlen);

close(s);
  return 0;
}
