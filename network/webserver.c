/*
 * webserver.c
 *
 *  Created on: Oct 21, 2014
 *      Author: ygz
 */

#if 0
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <modbus.h>
#include "config.h"
#include "cJSON/cJSON.h"
#include "libmodbus-3.0.6/src/modbus-private.h"

#define NB_CONNECTION    5

static void close_sigint(int dummy)
{
//    close(server_socket);
//    modbus_free(ctx);
//    modbus_mapping_free(mb_mapping);

//    exit(dummy);
}

void *thread_webserver(void *arg)
{
	Rtu_a118_t *self = &rtu_a118_t;
	char *name = "null";
	char *node = "::";
	char *service = "80";
	short port_bin = 80;
	char port_str[20] = "80";

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
	printf("web server!!!!!!!!!!!!!!!!!!!!!!!!\n");

	printf("[thread] start name %s, node %s, service %s\n", name, node, service);

	int master_socket;
	int rc;
	fd_set refset;
	fd_set rdset;

	modbus_t *ctx_pi = NULL;
	int server_socket_pi;

	/* Maximum file descriptor number */
	int fdmax;
	sprintf(port_str, "%hd", port_bin);
	ctx_pi = modbus_new_tcp_pi(node, port_str); //支持ipv6

	leaf = cJSON_GetObjectItem(config, "debug");
	if(NULL == leaf) {
		modbus_set_debug(ctx_pi, FALSE);
		printf("no debug\n");
	} else if(0 == strcmp("on", leaf->valuestring)) {
		modbus_set_debug(ctx_pi, TRUE);
	} else {
		modbus_set_debug(ctx_pi, FALSE);
	}

	ctx_pi->msgfd = -1;
	server_socket_pi = modbus_tcp_pi_listen(ctx_pi, NB_CONNECTION);

//    signal(SIGINT, close_sigint);
	if(SIG_ERR == signal(SIGPIPE, close_sigint)) {
		perror("SIGPIPE register error\n");
	}

	/* Clear the reference set of socket */
	FD_ZERO(&refset); //清空描述符集合
	/* Add the server socket */
	FD_SET(server_socket_pi, &refset); //添加描述符
	fdmax = server_socket_pi;

	printf("start for select server_socket_pi: %d\n", server_socket_pi);
	for (;;) {
		rdset = refset;
		struct timeval s_timeval = {3, 0};
		if ((rc = select(fdmax+1, &rdset, NULL, NULL, &s_timeval)) == -1) {
			perror("Server select() failure.");
			close_sigint(1);
		}
		if(0 == rc) {
//        	printf("after select rc = 0\n");
		}

		/* Run through the existing connections looking for data to be
		 * read */
		for (master_socket = 0; master_socket <= fdmax; master_socket++) {

			if (FD_ISSET(master_socket, &rdset)) {
				if(master_socket == server_socket_pi) {
					struct sockaddr_storage addr;
					socklen_t addrlen;
					int newfd;
					addrlen = sizeof(addr);
					newfd = accept(server_socket_pi, (void *)&addr, &addrlen);
					if (newfd == -1) {
						perror("Server accept() error");
					} else {
						FD_SET(newfd, &refset);

						if (newfd > fdmax) {
							/* Keep track of the maximum */
							fdmax = newfd;
						}
						if (ctx_pi->debug) {
							printf("The client connection is accepted.\n");
						}
						char ip_addr[50] = {'\0'};
						struct sockaddr_in6 *ipv6_addr = (struct sockaddr_in6 *)&addr;
						inet_ntop(addr.ss_family, (const void *)&ipv6_addr->sin6_addr, ip_addr, addrlen);
						printf("a client from socket ipv4/ipv6 %d %s\n", newfd, ip_addr);
					}
				}
				else {
					/* An already connected master has sent a new query */
//					uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
					uint8_t query[1024];
					uint8_t resp[1024];

					modbus_set_socket(ctx_pi, master_socket);
//					rc = modbus_receive(ctx_pi, query);
					rc = recv(ctx_pi->s, query, sizeof(query), 0);
					if (rc > 0) {
//						modbus_reply(ctx_pi, query, rc, NULL);
						sprintf(resp, "http/1.0 200 OK\r\n"
								"Content-type: text/html\r\n"
								"Connection: close\r\n"
								"Content-length: 7\r\n\r\n"
								"hello !");
						rc = send(ctx_pi->s, resp, strlen(resp), 0);
						printf("%s\n", resp);
					} else {
						/* Connection closed by the client, end of server */
						struct sockaddr_in6 ipv6_addr;
						socklen_t addrlen;
						addrlen = sizeof(ipv6_addr);
						getpeername(master_socket, (struct sockaddr *)&ipv6_addr, &addrlen);
						char ip_addr[50] = {'\0'};
						inet_ntop(ipv6_addr.sin6_family, (const void *)&ipv6_addr.sin6_addr, ip_addr, addrlen);
						printf("a client closed on socket ipv4/ipv6 %d %s\n", master_socket, ip_addr);
//                        printf("Connection closed on socket %d\n", master_socket);
						shutdown(master_socket, SHUT_RDWR);
						close(master_socket);

						/* Remove from reference set */
						FD_CLR(master_socket, &refset);

						if (master_socket == fdmax) {
							fdmax--;
						}
					}
				}
			}
		}
	}
	return 0;
}
#endif
