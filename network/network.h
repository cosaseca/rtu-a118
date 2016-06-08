/*
 * network.h
 *
 *  Created on: Oct 8, 2014
 *      Author: abc
 */

#ifndef NETWORK_H_
#define NETWORK_H_

extern void *thread_tcp_server(void *arg);
extern void *thread_tcp_client(void *arg);
extern void *thread_webserver(void *arg);
extern void *thread_netlink_server(void *arg);

/**
 * 设置网络地址参数：ip地址，子网掩码，网关
 * 所有参数都是字符串，尾部有'\0', NULL时不设置此参数
 * 如：ip ＝ "192.168.1.123"
 * netmask = "255.255.0.0"
 * gateway = "192.168.1.1"
 */
extern int set_ipv4_config(const char *ip, const char *netmask, const char *gateway);

/**
 * 设置网络地址参数的二进制版本：ip地址，子网掩码，网关
 * 每个参数长度为4的数组：NULL时不设置此参数
 * 如：ip ＝ {192, 168, 1, 123}
 * netmask ＝ {255, 255, 0, 0}
 * gateway ＝ {192, 168, 1, 1}
 */
extern int set_ipv4_config_bin(uint16_t *ip, uint16_t *netmask, uint16_t *gateway);


/**
 * 设置网络地址参数：ipv6地址，子网掩码，网关
 * 所有参数都是字符串，尾部有'\0', NULL时不设置此参数
 * 如：ip ＝ "192.168.1.123"
 * netmask = "255.255.0.0"
 * gateway = "192.168.1.1"
 */
extern int set_ipv6_config(const char *ip, const char *netmask, const char *gateway);

extern int set_ipv6_config_bin(uint16_t *ip, uint16_t *netmask, uint16_t *gateway);

/**
 * 设置网络tcp端口
 * port: 端口
 */
extern int set_tcp_port(uint16_t port);

/**
 * 设置网络udp端口
 * port: 端口
 */
extern int set_udp_port(uint16_t port);

/**
 * 设置网络tcp/udp端口
 * config: 配置(40057-40059)
 */
extern int set_tcp_udp_port(const short *config);

extern int set_remote_ipv4_config(const char *ip, short port, short type);

extern int set_remote_ipv4_config_bin(const short *ip, short port, short type);

extern int set_remote_ipv6_config(const char *ip, short port, short type);

extern int set_remote_ipv6_config_bin(const short *ip, short port, short type);

extern void *thread_knevn(void *arg);

extern int modbus_tcp_pi_udp_listen(modbus_t *ctx, int nb_connection);

extern void *thread_udp_server(void *arg);

extern int udp_heart_pack_check_integrity(uint8_t *buffer, uint16_t buffer_length);

#endif /* NETWORK_H_ */
