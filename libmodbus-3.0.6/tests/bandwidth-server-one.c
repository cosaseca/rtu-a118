/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus-private.h>
#include <modbus.h>

enum {
	TCP, RTU, ZIGBEE,
};

int main(int argc, char *argv[]) {
	int socket;
	modbus_t *ctx;
	modbus_mapping_t *mb_mapping;
	int rc;
	int use_backend;

	/* TCP */
	if (argc > 1) {
		if (strcmp(argv[1], "tcp") == 0) {
			use_backend = TCP;
		} else if (strcmp(argv[1], "rtu") == 0) {
			use_backend = RTU;
		} else if (strcmp(argv[1], "zigbee") == 0) {
			use_backend = ZIGBEE;
		} else {
			printf(
					"Usage:\n  %s [tcp|rtu] - Modbus client to measure data bandwith\n\n",
					argv[0]);
			exit(1);
		}
	} else {
		/* By default */
		use_backend = TCP;
	}

	if (use_backend == TCP) {
		ctx = modbus_new_tcp(argv[2], 1502);
		socket = modbus_tcp_listen(ctx, 1);
		modbus_tcp_accept(ctx, &socket);

	} else if (use_backend == RTU) {
		ctx = modbus_new_rtu(argv[2], 115200, 'N', 8, 1);
		modbus_set_slave(ctx, 1);
		modbus_connect(ctx);
	} else {
		ctx = modbus_new_rtu_zigbee(argv[2], 115200, 'N', 8, 1);
		modbus_set_slave(ctx, 1);
		modbus_connect(ctx);
	}
	ctx->debug = 1;

	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
	MODBUS_MAX_READ_REGISTERS, 0);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

//    int len = 0;

	for (;;) {
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

		if (use_backend == ZIGBEE) {
			rc = modbus_receive_zigbee(ctx, query);
			if (rc >= 0) {
				printf("modbus_receive_zigbee ok\n");
			} else {
				/* Connection closed by the client or server */
				break;
			}
			continue;
		}
		rc = modbus_receive(ctx, query);
		if (rc >= 0) {
			modbus_reply(ctx, query, rc, mb_mapping);
		} else {
			/* Connection closed by the client or server */
			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	modbus_mapping_free(mb_mapping);
	close(socket);
	modbus_free(ctx);

	return 0;
}
