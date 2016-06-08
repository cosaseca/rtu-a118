/*
 * utils.c
 *
 *  Created on: Dec 18, 2014
 *      Author: ygz
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include "common/thread.h"
#include "config.h"
#include "map.h"

#ifndef MAX_WELL_NUM
#define MAX_WELL_NUM  8
#endif

#define IPV6_CONF_FILE   "/proc/net//if_inet6"


/**
   @brief assic字符转换为整形
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源字符 
   @return 0
   @note
 */
inline int utils_atoi_a(char *to, const char *from) {
	if(*from > 'F' || *from < '0') {
		return -1;
	}
	if(*from > 'A' - 1) {
		*to = *from - 'A' + 10;
	}
	else if(*from > '0' - 1) {
		*to = *from - '0';
	}
//	DEBUGPRINT(DEBUG_INFO, "<>--[%d]--<>\n", *to);
	return 0;
}

/**
   @brief assic双字符转换为整形
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源双字符 
   @return 0
   @note
 */
inline int utils_atoi_b(char *to, const char *from) {
	char b = 0;
	if(utils_atoi_a(&b, from) == -1) {
		return -1;
	}
	*to = 0;
	*to += b * 16;
	if(utils_atoi_a(&b, from + 1) == -1) {
		return -1;
	}
	*to += b;
	return 0;
}

/**
   @brief assic字符串转换为整形
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源字符串，必须为偶数个 
   @return 0
   @note
 */
inline int utils_astois(char *to, const char *from, int size) {
	int i = 0;
	for(;i < size;++i) {
		if(utils_atoi_b(to, from) == -1) {
			return -1;
		}
//		DEBUGPRINT(DEBUG_INFO, "<>--[%d]--<>\n", *to);
		++to;
		from += 2;
	}
	return 0;
}

/**
   @brief 半字节整形转换为assic字符
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源整形 
   @return 0
   @note
 */
inline int utils_itoa_a(char *to, const char from) {
	if(from > 0x09) {
		*to = 'A' + from - 0x0A;
	}
	else {
		*to = '0' + from - 0x00;
	}
	return 0;
}

/**
   @brief 一字节整形转换为assic双字符
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源整形 
   @return 0
   @note
 */
inline int utils_itoa_b(char *to, const char from) {
	utils_itoa_a(to, (from&0xFF)>>4);
	utils_itoa_a(to + 1, from&0x0F);
	return 0;
}

/**
   @brief 整形转换为assic字符串
   @author yangguozheng 
   @param[out] to 目标 
   @param[in] from 源整形 
   @return 0
   @note
 */
inline int utils_istoas(char *to, const char *from, int size) {
	int i = 0;
	for(;i < size;++i) {
		utils_itoa_b(to, *from);
		to += 2;
		++from;
	}
	return 0;
}

/*
 * 获取ipv6地址
 * ipv6 out ipv6地址
 */
int get_ipv6_addr(uint16_t *ipv6) {
	FILE *file = fopen(IPV6_CONF_FILE, "r");
	if(NULL == file) {
                perror("IPV6_CONF_FILE ");
		return -1;
	}
	char buf[256];
	char ip_addr[64 + 1];
	int d_num;
	int ip_prefix_len;
	int ip_scope;
	int i_face;
	char d_name[32];
	int rc;
	char ip_addr_str[64 + 1];
	memset(ip_addr_str, ' ', 64);
	ip_addr_str[64] = '\0';
	
	while(fgets(buf, sizeof(buf), file)) {
		//printf("%s", buf);
		sscanf(buf, "%s%x%x%x%x%s", ip_addr, &d_num,
			&ip_prefix_len, &ip_scope, &i_face, d_name);
		if(0 == strcmp("eth1", d_name) && (0x20 == ip_scope || (0 == ip_scope && 0 == i_face))) {
			printf("%s %x %x %x %x %s\n", ip_addr, d_num,
				ip_prefix_len, ip_scope, i_face, d_name);
			int i;
			for(i = 0;i < 8;++i) {
				memcpy(ip_addr_str + i * 5, ip_addr + i * 4, 4);
			}

			rc = sscanf(ip_addr_str, "%hx%hx%hx%hx%hx%hx%hx%hx", 
				ipv6 + 0,
				ipv6 + 1,
				ipv6 + 2,
				ipv6 + 3,
				ipv6 + 4,
				ipv6 + 5,
				ipv6 + 6,
				ipv6 + 7);
			if(8 != rc) {
				continue;
			}
			
			*(ipv6 + 8) = ip_prefix_len;
			//for(i = 0;i < 9;++i) {
			//	printf("%hx:", *(ipv6 + i));
			//}
			//printf("\n");
			if(0 == ip_scope && 0 == i_face) {
				break;
			}
		}
	}
	fclose(file);
	return 0;
}


/*
 * 获取接口mac地址
 * name in 接口名称
 * mac out mac 地址
 */
static int get_mac_addr(const char *name, uint16_t *mac) {
	struct ifreq ifreq;
	int sock = 0;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("error sock");
		return -1;
	}

	strcpy(ifreq.ifr_name, name);
	if(ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		perror("error ioctl");
		close(sock);
		return -1;
	}

	int i = 0;
	for(i = 0; i < 6; i++){
		mac[i] = ifreq.ifr_hwaddr.sa_data[i];
	}
	printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3],
			mac[4], mac[5]);
	close(sock);
	return 0;
}

/* 系统信息初始化 */
int rtu_sys_info_init(Rtu_a118_t *self) {
	get_mac_addr("eth1", self->mac_addr);
	thread_time_init();
//	get_actuator_well_num(&self->actuator_well_num);
	return 0;
}

