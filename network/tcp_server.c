/*
 * rtu_a118.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
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

#include "libmodbus-3.0.6/src/modbus-private.h"
#include "config.h"
#include "cJSON/cJSON.h"

#define NB_CONNECTION    5

static void close_sigint(int dummy)
{
//    close(server_socket);
//    modbus_free(ctx);
//    modbus_mapping_free(mb_mapping);

//    exit(dummy);
}

/* tcp服务线程, 支持ipv4/ipv6 */
void *thread_tcp_server(void *arg)
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

    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;

    modbus_t *ctx_pi = NULL;
    int server_socket_pi;

    /* Maximum file descriptor number */
    int fdmax;

    zigbee_read_registers((uint16_t *)&port_bin, 1, 1, MAP_ADDR_TCP_PORT);
//    port_bin = 10086;
	sprintf(port_str, "%hd", port_bin);
	printf("port_str %s\n", port_str);
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

    ctx_pi->msgfd = self->tcp_server_msg_fd;
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
    if(ctx_pi->msgfd > 0) {
    	FD_SET(ctx_pi->msgfd, &refset);
    	fdmax = ctx_pi->msgfd > server_socket_pi?ctx_pi->msgfd:server_socket_pi;
    }

    int msg_handler(int msgfd, MsgData *data) {
    	printf("msgfd %d data %d\n", msgfd, *(int *)data);
    	if(*(int *)data == MSG_CONFIG_START) {
    		zigbee_read_registers((uint16_t *)&port_bin, 1, 1, MAP_ADDR_TCP_PORT);
			printf("tcp server port %d\n", port_bin);
    		if(port_bin == atoi(port_str)) {
    			printf("==\n");
    			return 0;
    		}
			for (master_socket = 0; master_socket <= fdmax; master_socket++) {
				if (FD_ISSET(master_socket, &rdset) && server_socket_pi != master_socket && master_socket != ctx_pi->msgfd) {
					shutdown(master_socket, SHUT_RDWR);
					close(master_socket);
				}
			}
			shutdown(server_socket_pi, SHUT_RDWR);
			close(server_socket_pi);

			FD_ZERO(&refset);
			modbus_free(ctx_pi);
			sprintf(port_str, "%hd", port_bin);
			ctx_pi = modbus_new_tcp_pi(node, port_str); //支持ipv6
			ctx_pi->msgfd = self->tcp_server_msg_fd;
			server_socket_pi = modbus_tcp_pi_listen(ctx_pi, NB_CONNECTION);
			printf("server_socket_pi %d\n", server_socket_pi);
			if(-1 == server_socket_pi) {
				perror("");
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
    for (;;) {
		update_thread_time(THREAD_TCP_SERVER);
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
						log_msg("client connect %d [%s]:%d\n", newfd, ip_addr, ipv6_addr->sin6_port);
					}
            	} else if(master_socket == ctx_pi->msgfd) {
            		pmsg_q_receive(ctx_pi->msgfd, ctx_pi->msgbuf, 0, NULL);
					if(NULL != ctx_pi->msgfun) {
						ctx_pi->msgfun(ctx_pi->msgfd, (MsgData *)ctx_pi->msgbuf);
					}
            	} else {
                    /* An already connected master has sent a new query */
                    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

                    modbus_set_socket(ctx_pi, master_socket);
                    rc = modbus_receive(ctx_pi, query);
                    if (rc != -1) {
                        modbus_reply(ctx_pi, query, rc, NULL);
						update_thread_com_time(THREAD_TCP_SERVER);
                    } else { //if(errno == ECONNRESET)
                        /* Connection closed by the client, end of server */
                    	//perror("socket closed closed ------------------------");
                    	struct sockaddr_in6 ipv6_addr;
                    	socklen_t addrlen;
						addrlen = sizeof(ipv6_addr);
                    	getpeername(master_socket, (struct sockaddr *)&ipv6_addr, &addrlen);
                    	char ip_addr[50] = {'\0'};
						inet_ntop(ipv6_addr.sin6_family, (const void *)&ipv6_addr.sin6_addr, ip_addr, addrlen);
						printf("a client closed on socket ipv4/ipv6 %d %s\n", master_socket, ip_addr);
//                        printf("Connection closed on socket %d\n", master_socket);
						log_msg("client disconnect %d [%s]:%d\n", master_socket, ip_addr, ipv6_addr.sin6_port);
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
