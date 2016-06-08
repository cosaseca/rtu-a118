/*
 * map.h
 *
 *  Created on: Dec 14, 2014
 *      Author: ygz
 */

#ifndef DATA_MAP_H_
#define DATA_MAP_H_
#include <stdint.h>
#include <sys/stat.h>

#ifndef RTU_VERSION
#define RTU_VERSION                   1         //软件版本
#endif
#define RTU_HW_VERSION                2 


enum {
	MAP_ADDR_RTU_VERSION              = 2,
	MAP_ADDR_APP_START_COUNT          = 15,
	MAP_ADDR_SERIAL_PORT              = 33,     //rs485 slave寄存器起始地址
	MAP_ADDR_IPV4_ADDR                = 38,     //本地ipv4地址
	MAP_ADDR_UDP_PORT                 = 57,     //本地udp端口
	MAP_ADDR_TCP_PORT                 = 58,     //本地tcp端口
	MAP_ADDR_REMOTE_IPV4_ADDR         = 59,     //ipv4服务器地址
	MAP_ADDR_IPV6_ADDR                = 66,     //本地ipv6地址
	MAP_ADDR_REMOTE_IPV6_ADDR         = 85,     //服务器ipv6地址
	MAP_ADDR_ZIGBEE_PID               = 99,     //zigbee配置首地址
	MAP_ADDR_UDP_REMOTE_IPV4_ADDR     = 108,    //udp ipv4服务器首地址
	MAP_ADDR_RTU_ID                   = 139,    //大庆定义的rtu标志
	MAP_ADDR_UDP_HEART_BEAT_STATUS    = 141,    //udp心跳开关
	MAP_ADDR_UDP_REMOTE_IPV6_ADDR     = 153,    //udp ipv6服务器首地址
	MAP_ADDR_RS485_MASTER_SERIAL_PORT = 171,
};

/* 函数映射索引 */
enum {
	MAP_CHANGE_WELL_TYPE     = 0,               //修改井类型
	MAP_GET_RTU_VER          = 2,               //rtu软件版本
	MAP_GET_TIME_HOUR        = 4,               //时
	MAP_GET_TIME_MIN         = 5,               //分
	MAP_GET_TIME_SEC         = 6,               //秒
	MAP_GET_TIME_YEAR        = 7,               //年
	MAP_GET_TIME_MON         = 8,               //月
	MAP_GET_TIME_DAY         = 9,               //日
	MAP_GET_BATTERY_VOLTAGE  = 10,              //电压
	MAP_CONFIG1              = 16,              //30配置
	MAP_GET_MAC0             = 50,              //mac 首地址
	MAP_GET_MAC1,
	MAP_GET_MAC2,
	MAP_GET_MAC3,
	MAP_GET_MAC4,
	MAP_GET_MAC5,
	MAP_CONFIG2_ELEC_INTERV  = 1796,            //电参同步
	MAP_CONFIG2_ACT_INTERV   = 1870,            //执行器同步
	MAP_GET_DI0              = 1854,            //DO首地址
	MAP_GET_DI1,
	MAP_GET_DI2,
	MAP_GET_DI3,
	MAP_GET_DI4,
	MAP_GET_DI5,
	MAP_GET_DI6,
	MAP_GET_DI7,
	MAP_GET_DO0,
	MAP_GET_DO1,
	MAP_GET_DO2,
	MAP_GET_DO3,
	MAP_GET_DO4,
	MAP_GET_DO5,
	MAP_GET_DO6,
	MAP_GET_DO7,
	MAP_GET_HISTORY          = 1901,            //获取历史数据
};

typedef struct Map_t {
	void *ram;                  //内存首地址
	uint16_t *ram_x;            //ram映射区
	char *ram_x_map_type;       //映射类型
	char *ram_x_map_record;     //记录类型
	char *ram_x_map_rw;         //读写类型(只读, 读写)
	uint16_t *ram_y;            //寄存器ram映射
	char *ram_y_map_type;       //寄存器映射类型
	uint16_t *flash1;           //flash1首地址
	uint16_t *flash2;           //flash2首地址
	char flash1_is_new;         //flash1是否首次创建
	char flash2_is_new;         //flash2是否首次创建
	uint16_t x_max;             //单个ram映射区大小
}Map_t;

extern int map_init(Map_t *map_t);
extern int zigbee_read_registers(uint16_t *buf, int len, int well_num, int addr);
extern int zigbee_read_registers_big_l(uint16_t *buf, int len, int well_num, int addr, char big_l);
extern int zigbee_write_registers(uint16_t *buf, int len, int well_num, int addr);
extern int map_modbus_read_registers(uint16_t *buf, int len, int well_num, int addr, char big_l);
extern int map_modbus_write_registers(uint16_t *buf, int len, int well_num, int addr, char big_l);
extern int get_ipv6_addr(uint16_t *ipv6);
extern int zigbee_write_registers_mset(uint16_t v,int len, int well_num, int addr);
extern int reset_flash1_to_default();
extern int rtu_reset_flash1(uint16_t addr, uint16_t *v);
extern int map_modbus_write_data_registers(uint16_t *buf, int len, int well_num, int addr, char big_l);
#endif /* DATA_MAP_H_ */
