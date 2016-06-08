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
#include <modbus-private.h>
#include <modbus-tcp-private.h>
#include <modbus.h>
#include "zigbee/zigbee.h"
#include "config.h"
#include "cJSON/cJSON.h"

/* tcp客户端服务线程, 支持ipv4/ipv6 */
void *thread_tcp_client(void *arg) {
	char service[2][20] = {"10086", "10087"};
	char *name = "client";
	char node[2][64] = {"::", "::"};
	int rc = -1;
	Rtu_a118_t *self = &rtu_a118_t;

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

    //读取远端ipv4服务器地址
	uint16_t server_info[5];
	zigbee_read_registers(server_info, 5, 1, MAP_ADDR_REMOTE_IPV4_ADDR);

	sprintf(node[0], "::ffff:%hd.%hd.%hd.%hd", server_info[0]
			, server_info[1]
			, server_info[2]
			, server_info[3]);

	sprintf(service[0], "%hd", server_info[4]);

    //读取远端ipv6服务器地址
	uint16_t server6_info[9];
	zigbee_read_registers(server6_info, 9, 1, MAP_ADDR_REMOTE_IPV6_ADDR);
	sprintf(node[1], "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx%%eth1", server6_info[0]
			, server6_info[1]
			, server6_info[2]
			, server6_info[3]
			, server6_info[4]
			, server6_info[5]
			, server6_info[6]
			, server6_info[7]);

	sprintf(service[1], "%hd", server6_info[8]);

	modbus_t *ctx;

	if(0 != strcmp("client", name)) {
		return NULL;
	}

	ctx = modbus_new_tcp_pi(node[0], service[0]); //支持ipv6
//	if (modbus_connect(ctx) == -1) {
//		fprintf(stderr, "Connexion failed: %s\n",
//				modbus_strerror(errno));
////		modbus_free(ctx);
////		return NULL;
//	}
	int msg_fd = self->tcp_client_msg_fd;
	int max_fd;
	fd_set fdset0;
	fd_set fdset1;
	FD_ZERO(&fdset0);
	if(msg_fd > 0) {
		FD_SET(msg_fd, &fdset0);
		max_fd = msg_fd;
	}
//	if(ctx->s > 0) {
//		FD_SET(ctx->s, &fdset0);
//	}
//	max_fd = msg_fd > ctx->s?msg_fd:ctx->s;
	modbus_tcp_pi_t *ctx_tcp_pi;
	ctx_tcp_pi = (modbus_tcp_pi_t *)ctx->backend_data;


	struct timeval time0 = {2, 0};
	struct timeval time1 = time0;
	printf("client connect ok node %s service %s\n", node[0], service[0]);
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH + 1020 * 2];


	int msg_handler(int fd, char *msg) {
		if(MSG_CONFIG_START == *(int *)msg) {
			if(ctx->s > 0) {
				shutdown(ctx->s, SHUT_RDWR);
				close(ctx->s);
			}
			zigbee_read_registers(server_info, 5, 1, MAP_ADDR_REMOTE_IPV4_ADDR);

			sprintf(node[0], "::ffff:%hd.%hd.%hd.%hd", server_info[0]
					, server_info[1]
					, server_info[2]
					, server_info[3]);

			sprintf(service[0], "%hd", server_info[4]);

			//uint16_t server6_info[9];
			zigbee_read_registers(server6_info, 9, 1, MAP_ADDR_REMOTE_IPV6_ADDR);
			sprintf(node[1], "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx%%eth1", server6_info[0]
					, server6_info[1]
					, server6_info[2]
					, server6_info[3]
					, server6_info[4]
					, server6_info[5]
					, server6_info[6]
					, server6_info[7]);

			sprintf(service[1], "%hd", server6_info[8]);
			modbus_free(ctx);

			printf("MSG_CONFIG_START tcp client %s %s\n", node[0], service[0]);
			ctx = modbus_new_tcp_pi(node[0], service[0]);
//			if (modbus_connect(ctx) == -1) {
//				fprintf(stderr, "Connexion failed: %s\n",
//						modbus_strerror(errno));
//		//		modbus_free(ctx);
//		//		return NULL;
//			}
			ctx_tcp_pi = (modbus_tcp_pi_t *)ctx->backend_data;

			FD_ZERO(&fdset0);
			if(msg_fd > 0) {
				FD_SET(msg_fd, &fdset0);
				max_fd = msg_fd;// > ctx->s?msg_fd:ctx->s;
			}
//			if(ctx->s > 0) {
//				FD_SET(ctx->s, &fdset0);
//			}
		}
		return 0;
	}
	int i;
	int j = 0;
	for (i = 0;;++i) {
		update_thread_time(THREAD_TCP_CLIENT);
		if(ctx->s <= 0) {
			strcpy(ctx_tcp_pi->node, node[i%2]);
			strcpy(ctx_tcp_pi->service, service[i%2]);
			if (modbus_connect(ctx) == -1) {
				log_warning("Connexion failed %s %s", ctx_tcp_pi->node, ctx_tcp_pi->service);
			} else {
				FD_SET(ctx->s, &fdset0);
				max_fd = msg_fd < ctx->s?ctx->s:msg_fd;
				j = 0;
			}
		}
		fdset1 = fdset0;
		time1 = time0;
		if(-1 == (rc = select(max_fd + 1, &fdset1, NULL, NULL, &time1))) {
			break;
		}
		if(0 == rc) {
//			printf("time out %d\n", j);
			++j;
			if(j > 200 && ctx->s > 0) {
				FD_CLR(ctx->s, &fdset0);
				if (ctx->s == max_fd) {
					max_fd = msg_fd;
				}
				modbus_close(ctx);
				ctx->s = -1;
			}
			continue;
		}
		if(ctx->s > 0) {
			if(FD_ISSET(ctx->s, &fdset1)) {
				rc = modbus_receive(ctx, query);
				printf("------------------rc %d\n", rc);
				if (rc > 0) {
					j = 0;
					rc = modbus_reply(ctx, query, rc, NULL);
					update_thread_com_time(THREAD_TCP_CLIENT);
					printf("modbus_receive %d\n", rc);
				} else { //if(errno == ECONNRESET)
					/* Connection closed by the client or server */
					printf("modbus_receive error\n");
//      			break;
					FD_CLR(ctx->s, &fdset0);
					if (ctx->s == max_fd) {
						max_fd = msg_fd;
					}
					modbus_close(ctx);
					ctx->s = -1;
				}
			}
		}
		printf("msg fd is ----------%d %d-------------\n", msg_fd, max_fd);
		if(msg_fd > 0) {
			if(FD_ISSET(msg_fd, &fdset1)) {
				printf("tcp client recieve msg ----\n");
				pmsg_q_receive(msg_fd, self->tcp_client_msg_buf, 0, NULL);
				msg_handler(msg_fd, self->tcp_client_msg_buf);
			} else {
				
			}
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	modbus_close(ctx);
	modbus_free(ctx);
	return 0;
}
