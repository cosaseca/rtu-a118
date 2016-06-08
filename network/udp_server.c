/*
 * udp_server.c
 *
 *  Created on: Nov 25, 2014
 *      Author: ygz
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <modbus.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <netdb.h>

#include "libmodbus-3.0.6/src/modbus-private.h"
#include "config.h"
#include "cJSON/cJSON.h"
#include "network.h"

#define NB_CONNECTION                 5

#define MAX_REMOTE_UDP_SERVER_NUM     6
#define MAX_REMOTE_UDP4_SERVER_NUM    3
#define MAX_REMOTE_UDP6_SERVER_NUM    2



static void close_sigint(int dummy)
{
//    close(server_socket);
//    modbus_free(ctx);
//    modbus_mapping_free(mb_mapping);

//    exit(dummy);
}

#if 1
static int get_addr(struct sockaddr_in6 *addr, int *len, const char *node, const char *service)
{
	int rc;
	struct addrinfo *ai_list;
	struct addrinfo *ai_ptr;
	struct addrinfo ai_hints;

	memset(&ai_hints, 0, sizeof (ai_hints));
	//ai_hints.ai_flags |= AI_PASSIVE;
#ifdef AI_ADDRCONFIG
	ai_hints.ai_flags |= AI_ADDRCONFIG;
#endif
	ai_hints.ai_family = AF_UNSPEC;
	ai_hints.ai_socktype = SOCK_DGRAM;//SOCK_STREAM;
	ai_hints.ai_flags = AI_PASSIVE;
	ai_hints.ai_protocol = 0;
	ai_hints.ai_addr = NULL;
	ai_hints.ai_canonname = NULL;
	ai_hints.ai_next = NULL;

	ai_list = NULL;
	rc = getaddrinfo(node, service, &ai_hints, &ai_list);
	//printf("rc: %d, node: %s, service: %s\n", rc, node, service);
	if (rc != 0) {
		perror("rc: ");
		*len = -1;
		return -1;
	}

	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {

//		s = socket(ai_ptr->ai_family, ai_ptr->ai_socktype,
//							ai_ptr->ai_protocol);
//		printf("socket %d\n", s);
//		if (s < 0) {
//			if (ctx->debug) {
//				perror("socket");
//			}
//			continue;
//		}
		memcpy(addr, ai_ptr->ai_addr, sizeof(struct sockaddr_in6));
		*len = ai_ptr->ai_addrlen;
		//printf("get addr udp addr ok %d\n", ai_ptr->ai_family);
		break;
	}
//	freeaddrinfo(ai_list);
	//printf("get addr udp addr\n");
	return 0;
}
#endif

/* udp服务线程支持ipv4/ipv6 */
void *thread_udp_server(void *arg)
{
	Rtu_a118_t *self = &rtu_a118_t;
	char *name = "null";
	char *node = "::";
	char *service = "10086";
	short port_bin = 10086;
	char port_str[20] = "10086";

	cJSON *config = (cJSON *)arg;
	if(NULL == config) {
		printf("cJSON_Parse error\n");
		return NULL;
	}
	cJSON *leaf = cJSON_GetObjectItem(config, "name");
	if(NULL != leaf && cJSON_String == leaf->type) {
		name = leaf->valuestring;
	} else {
		printf("no name config\n");
		return NULL;
	}
	leaf = cJSON_GetObjectItem(config, "node");
	if(NULL != leaf && cJSON_String == leaf->type) {
		node = leaf->valuestring;
	} else {
		printf("no node config\n");
	}
	leaf = cJSON_GetObjectItem(config, "service");
	if(NULL != leaf && cJSON_String == leaf->type) {
		service = leaf->valuestring;
	} else {
		printf("no service config\n");
	}

//	printf("name %s, node %s, service %s\n", name, node, service);
	if(0 == strcmp("server", name)) {

	} else {
		return NULL;
	}

	printf("[thread] start name %s, node %s, service %s\n", name, node, service);

	struct sockaddr_in6 addr_in[MAX_REMOTE_UDP_SERVER_NUM];
	int addr_in_len[MAX_REMOTE_UDP_SERVER_NUM];
	int rc;
    fd_set refset;
    fd_set rdset;

    modbus_t *ctx_pi = NULL;
    int server_socket_pi;

//    char udp_heart_pack[] = {
//    		0xC0
//			,00
//			,00
//			,00
//			,00
//			,00
//			,00
//			,00
//    };

    typedef struct Udp_heart_pack_t {
    	char type;
    	short addr;
    	short len;
    	int rtu_id;
    	short verify;
    	//data
    }__attribute__((packed)) Udp_heart_pack_t;

    Udp_heart_pack_t udp_heart_pack_t = {
    		.type = 0xC0,
			.addr = 0x0000,
			.len  = 0x0000,
			.rtu_id = 0x00000000,
			.verify = 0x0000,
    };

    time_t time0 = time(NULL);

    /* Maximum file descriptor number */
    int fdmax;

    zigbee_read_registers_big_l((uint16_t *)&udp_heart_pack_t.rtu_id, 2, 1, MAP_ADDR_RTU_ID, 1);

    uint16_t heart_beat_info[3];
    char has_beat = 0;
    zigbee_read_registers(heart_beat_info, 3, 1, MAP_ADDR_UDP_HEART_BEAT_STATUS);

    if(heart_beat_info[1] < 5) {
    	heart_beat_info[1] = 5;
    }
    if(heart_beat_info[2] < 10) {
		heart_beat_info[2] = 10;
	}

    zigbee_read_registers((uint16_t *)&port_bin, 1, 1, MAP_ADDR_UDP_PORT);
//    port_bin = 10086;
	sprintf(port_str, "%hd", port_bin);
	printf("port_str %s\n", port_str);
    ctx_pi = (modbus_t *)modbus_new_udp_pi(node, port_str); //支持ipv6

    /* 读取udp ipv4/ipv6服务器地址 */
    uint16_t addr_info[5 * MAX_REMOTE_UDP4_SERVER_NUM];
	uint16_t addr6_info[9 * MAX_REMOTE_UDP6_SERVER_NUM];
    zigbee_read_registers(addr_info, sizeof(addr_info)/sizeof(uint16_t), 1, MAP_ADDR_UDP_REMOTE_IPV4_ADDR);
	zigbee_read_registers(addr6_info, sizeof(addr6_info)/sizeof(uint16_t), 1, MAP_ADDR_UDP_REMOTE_IPV6_ADDR);
    char server_ip_addr[256];// = "::ffff:192.168.31.125";
    char server_port[64];// = "10087";
    int a0 = 0;
    for(a0 = 0;a0 < MAX_REMOTE_UDP4_SERVER_NUM;++a0) {
		sprintf(server_ip_addr, "::ffff:%hd.%hd.%hd.%hd",
				*(addr_info + a0 * 5)
				,*(addr_info + 1 + a0 * 5)
				,*(addr_info + 2 + a0 * 5)
				,*(addr_info + 3 + a0 * 5));
		sprintf(server_port, "%hd",
				*(addr_info + 4 + a0 * 5));
		printf("%s %s\n", server_ip_addr, server_port);
//		getaddrinfo(server_ip_addr, server_port,&hints,&res);
//		if(NULL != res) {
//			memcpy(dest + a0, res, sizeof(dest));
//		}
//		getaddrinfo(server_ip_addr,server_port,&hints,(ress + a0));
		get_addr(addr_in + a0, addr_in_len + a0, server_ip_addr, server_port);
//		printf("ddddddddddddddddddddddd %d\n", (udp_server_addr + a0)->ai_family);
    }

	//get addr udp6 server ip
	for(a0 = 0;a0 < MAX_REMOTE_UDP6_SERVER_NUM;++a0) {
		sprintf(server_ip_addr, "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
				*(addr6_info + a0 * 9)
				,*(addr6_info + 1 + a0 * 9)
				,*(addr6_info + 2 + a0 * 9)
				,*(addr6_info + 3 + a0 * 9)
				,*(addr6_info + 4 + a0 * 9)
				,*(addr6_info + 5 + a0 * 9)
				,*(addr6_info + 6 + a0 * 9)
				,*(addr6_info + 7 + a0 * 9));
		sprintf(server_port, "%hd",
				*(addr6_info + 8 + a0 * 9));
		printf("%s %s\n", server_ip_addr, server_port);
//		getaddrinfo(server_ip_addr, server_port,&hints,&res);
//		if(NULL != res) {
//			memcpy(dest + a0, res, sizeof(dest));
//		}
//		getaddrinfo(server_ip_addr,server_port,&hints,(ress + a0));
		get_addr(addr_in + MAX_REMOTE_UDP4_SERVER_NUM + a0, 
			addr_in_len + MAX_REMOTE_UDP4_SERVER_NUM + a0, 
			server_ip_addr, server_port);
//		printf("ddddddddddddddddddddddd %d\n", (udp_server_addr + a0)->ai_family);
    }

    leaf = cJSON_GetObjectItem(config, "debug");
	if(NULL == leaf) {
		modbus_set_debug(ctx_pi, FALSE);
		printf("no debug\n");
	} else if(0 == strcmp("on", leaf->valuestring)) {
		modbus_set_debug(ctx_pi, TRUE);
	} else {
		modbus_set_debug(ctx_pi, FALSE);
	}

    ctx_pi->msgfd = self->udp_server_msg_fd;
    server_socket_pi = modbus_tcp_pi_udp_listen(ctx_pi, NB_CONNECTION);

//    signal(SIGINT, close_sigint);
    if(SIG_ERR == signal(SIGPIPE, close_sigint)) {
		perror("SIGPIPE register error\n");
	}

    /* Clear the reference set of socket */
    FD_ZERO(&refset); //清空描述符集合
    /* Add the server socket */
    FD_SET(server_socket_pi, &refset); //添加描述符
    fdmax = server_socket_pi;
    if(ctx_pi->msgfd > 0) {
    	FD_SET(ctx_pi->msgfd, &refset);
    	fdmax = ctx_pi->msgfd > server_socket_pi?ctx_pi->msgfd:server_socket_pi;
    }

    int msg_handler(int msgfd, MsgData *data) {
    	printf("udp msgfd %d data %d\n", msgfd, *(int *)data);
    	if(*(int *)data == MSG_CONFIG_START) {
//			uint16_t addr_info[5 * 3];
			zigbee_read_registers(addr_info, 5 * 3, 1, MAP_ADDR_UDP_REMOTE_IPV4_ADDR);
//			char server_ip_addr[128];// = "::ffff:192.168.31.125";
//			char server_port[64];// = "10087";
//			int a0 = 0;
			for(a0 = 0;a0 < 3;++a0) {
				sprintf(server_ip_addr, "::ffff:%hd.%hd.%hd.%hd",
						*(addr_info + a0 * 5)
						,*(addr_info + 1 + a0 * 5)
						,*(addr_info + 2 + a0 * 5)
						,*(addr_info + 3 + a0 * 5));
				sprintf(server_port, "%hd",
						*(addr_info + 4 + a0 * 5));
				printf("%s %s\n", server_ip_addr, server_port);
		//		getaddrinfo(server_ip_addr, server_port,&hints,&res);
		//		if(NULL != res) {
		//			memcpy(dest + a0, res, sizeof(dest));
		//		}
		//		getaddrinfo(server_ip_addr,server_port,&hints,(ress + a0));
				get_addr(addr_in + a0, addr_in_len + a0, server_ip_addr, server_port);
		//		printf("ddddddddddddddddddddddd %d\n", (udp_server_addr + a0)->ai_family);
			}

			zigbee_read_registers(addr6_info, sizeof(addr6_info)/sizeof(uint16_t), 1, MAP_ADDR_UDP_REMOTE_IPV6_ADDR);

			//get addr udp6 server ip
			for(a0 = 0;a0 < MAX_REMOTE_UDP6_SERVER_NUM;++a0) {
				sprintf(server_ip_addr, "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
						*(addr6_info + a0 * 9)
						,*(addr6_info + 1 + a0 * 9)
						,*(addr6_info + 2 + a0 * 9)
						,*(addr6_info + 3 + a0 * 9)
						,*(addr6_info + 4 + a0 * 9)
						,*(addr6_info + 5 + a0 * 9)
						,*(addr6_info + 6 + a0 * 9)
						,*(addr6_info + 7 + a0 * 9));
				sprintf(server_port, "%hd",
						*(addr6_info + 8 + a0 * 9));
				printf("%s %s\n", server_ip_addr, server_port);
		//		getaddrinfo(server_ip_addr, server_port,&hints,&res);
		//		if(NULL != res) {
		//			memcpy(dest + a0, res, sizeof(dest));
		//		}
		//		getaddrinfo(server_ip_addr,server_port,&hints,(ress + a0));
				get_addr(addr_in + MAX_REMOTE_UDP4_SERVER_NUM + a0, 
					addr_in_len + MAX_REMOTE_UDP4_SERVER_NUM + a0, 
					server_ip_addr, server_port);
		//		printf("ddddddddddddddddddddddd %d\n", (udp_server_addr + a0)->ai_family);
		    }

			zigbee_read_registers_big_l((uint16_t *)&udp_heart_pack_t.rtu_id, 2, 1, MAP_ADDR_RTU_ID, 1);
			zigbee_read_registers(heart_beat_info, 3, 1, MAP_ADDR_UDP_HEART_BEAT_STATUS);
			if(heart_beat_info[1] < 5) {
				heart_beat_info[1] = 5;
			}
			if(heart_beat_info[2] < 10) {
				heart_beat_info[2] = 10;
			}


    		zigbee_read_registers((uint16_t *)&port_bin, 1, 1, MAP_ADDR_UDP_PORT);
    		if(port_bin == atoi(port_str)) {
    			printf("==\n");
    			return 0;
    		}
			shutdown(server_socket_pi, SHUT_RDWR);
			close(server_socket_pi);

			FD_ZERO(&refset);
			modbus_free(ctx_pi);
			sprintf(port_str, "%hd", port_bin);
			ctx_pi = (modbus_t *)modbus_new_udp_pi(node, port_str); //支持ipv6
			ctx_pi->msgfd = self->udp_server_msg_fd;
			server_socket_pi = modbus_tcp_pi_udp_listen(ctx_pi, NB_CONNECTION);
			printf("server_socket_pi %d\n", server_socket_pi);
			if(-1 == server_socket_pi) {
				perror("-1 == server_socket_pi");
			}

			FD_SET(server_socket_pi, &refset); //添加描述符
			fdmax = server_socket_pi;
			if(ctx_pi->msgfd > 0) {
				FD_SET(ctx_pi->msgfd, &refset);
				fdmax = ctx_pi->msgfd > server_socket_pi?ctx_pi->msgfd:server_socket_pi;
			}
			ctx_pi->msgfun = msg_handler;
			printf("reset ok\n");
    	}
    	return 0;
    }

    ctx_pi->msgfun = msg_handler;

    printf("start for select server_socket_pi: %d\n", server_socket_pi);
//    modbus_set_debug(ctx_pi, 1);

    for (;;) {
		update_thread_time(THREAD_UDP_SERVER);
        rdset = refset;
        struct timeval s_timeval = {3, 0};
        if ((rc = select(fdmax+1, &rdset, NULL, NULL, &s_timeval)) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }
        
        /* 发送心跳包 */
        //printf("heart_beat_info[0] %hd\n", heart_beat_info[0]);
        if(heart_beat_info[0]) {
			if(has_beat) {
				if(time(NULL) > time0 + heart_beat_info[2]) {
					for(a0 = 0;a0 < MAX_REMOTE_UDP_SERVER_NUM;++a0) {
						if(-1 != addr_in_len[a0]) {
							rc = sendto(server_socket_pi,&udp_heart_pack_t,sizeof(Udp_heart_pack_t),0,(struct sockaddr *)(addr_in + a0),addr_in_len[a0]);
							has_beat = 0;
						}
						printf("rc rc  %d\n", rc);
					time(&time0);
					}
				}
			} else {
				if(time(NULL) > time0 + heart_beat_info[1]) {
					for(a0 = 0;a0 < MAX_REMOTE_UDP_SERVER_NUM;++a0) {
						if(-1 != addr_in_len[a0]) {
							rc = sendto(server_socket_pi,&udp_heart_pack_t,sizeof(Udp_heart_pack_t),0,(struct sockaddr *)(addr_in + a0),addr_in_len[a0]);
							has_beat = 0;
						}
						printf("rc rc  %d\n", rc);
					time(&time0);
					}
				}
			}
        }
        if(0 == rc) {
//        	printf("after select rc = 0\n");
        	continue;
        }

        if (FD_ISSET(server_socket_pi, &rdset)) {
        	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
			modbus_set_socket(ctx_pi, server_socket_pi);
			struct sockaddr_in6 addr_in;
			int addr_len= sizeof(addr_in);
			rc = recvfrom(server_socket_pi, (void *)query, sizeof(query), 0
					, (struct sockaddr *)(&addr_in), (socklen_t *)&addr_len);//s o s
//			memcpy(&ctx_pi->ip_addr, &addr_in, addr_len);
			ctx_pi->addr_len = addr_len;
			ctx_pi->ip_addr = addr_in;
			if (rc > 7) {
				if(rc > 11 && 0xC0 == *query 
					&& -1 != udp_heart_pack_check_integrity(query + 9, rc - 9)) {
					has_beat = 1;
					log_msg("heart beat\n");
				} else {
					modbus_reply(ctx_pi, query, rc, NULL);
				}
				update_thread_com_time(THREAD_UDP_SERVER);
			} else {

			}
		}

        if (FD_ISSET(ctx_pi->msgfd, &rdset)) {
			pmsg_q_receive(ctx_pi->msgfd, ctx_pi->msgbuf, 0, NULL);
			if(NULL != ctx_pi->msgfun) {
				ctx_pi->msgfun(ctx_pi->msgfd, (MsgData *)ctx_pi->msgbuf);
			}
		}
	}
    return 0;
}

