/*
 * common.c
 *
 *  Created on: Sep 26, 2014
 *      Author: abc
 */
#include <string.h>
#include "common/common.h"
#include <unistd.h>
#include <sys/time.h>


//--------------------------------------------------------------------------------
//函数作用:从寄存器取数32字节组成整数字节倒序
//参数说明:
//注意事项:
//返回说明:无
//--------------------------------------------------------------------------------
u32 swap_register32(u32 var) {
	u32 tmp1, tmp2;
	tmp1 = (var >> 16) & 0x0000FFFF;
	tmp2 = (var << 16) & 0xFFFF0000;
	return (tmp1 | tmp2);
}

//--------------------------------------------------------------------------------
//函数作用:从寄存器取数64字节组成整数字节倒序
//参数说明:
//注意事项:
//返回说明:无
//--------------------------------------------------------------------------------
u64 swap_register64(u64 var) {
	u64 tmp1, tmp2, tmp3, tmp4;

	tmp1 = (var >> 48) & 0x000000000000FFFF;
	tmp2 = (var >> 16) & 0x00000000FFFF0000;
	tmp3 = (var << 16) & 0x0000FFFF00000000;
	tmp4 = (var << 48) & 0xFFFF000000000000;
	return tmp1 | tmp2 | tmp3 | tmp4;
}

//--------------------------------------------------------------------------------
//函数作用:16位高低字节交换
//参数说明:
//注意事项:
//返回说明:无
//--------------------------------------------------------------------------------
uint16 swap_int16(uint16 n) {
	uint16 temp1, temp2;

	temp1 = (n >> 8) & 0x00FF;
	temp2 = (n << 8) & 0xFF00;
	return temp1 | temp2;
}

//--------------------------------------------------------------------------------
//函数作用:32位高低字节交换
//参数说明:
//注意事项:
//返回说明:无
//--------------------------------------------------------------------------------
uint32 swap_int32(uint32 n) {
	uint32 temp1, temp2, temp3, temp4;

	temp1 = (n >> 24) & 0x000000FF;
	temp2 = (n >> 8) & 0x0000FF00;
	temp3 = (n << 8) & 0x00FF0000;
	temp4 = (n << 24) & 0xFF000000;
	return temp1 | temp2 | temp3 | temp4;
}

//--------------------------------------------------------------------------------
//函数作用:64位高低字节交换
//参数说明:
//注意事项:
//返回说明:无
//--------------------------------------------------------------------------------
uint64 swap_int64(uint64 n) {
	uint64 temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;

	temp1 = (n >> 56) & 0x00000000000000FF;
	temp2 = (n >> 40) & 0x000000000000FF00;
	temp3 = (n >> 24) & 0x0000000000FF0000;
	temp4 = (n >> 8) & 0x00000000FF000000;
	temp5 = (n << 8) & 0x000000FF00000000;
	temp6 = (n << 24) & 0x0000FF0000000000;
	temp7 = (n << 40) & 0x00FF000000000000;
	temp8 = (n << 56) & 0xFF00000000000000;

	return temp1 | temp2 | temp3 | temp4 | temp5 | temp6 | temp7 | temp8;
}


//float 字节交换1234 4321
float swap_rcv_float_seq(float var)
{
	float ret;
	u8 tmp_array[4],tmp_change;
	memcpy(&tmp_array[0],&var,sizeof(float));
	tmp_change = tmp_array[0];
	tmp_array[0] = tmp_array[3];
	tmp_array[3] = tmp_change;
	tmp_change = tmp_array[1];
	tmp_array[1] = tmp_array[2];
	tmp_array[2] = tmp_change;
	memcpy(&ret,&tmp_array[0],sizeof(float));
	return ret;
}

//float 字节交换4321 2143
float swap_stor_float_seq(float var)
{
	float ret = 0.0;
	float tmp_var = var;
	u8 tmp_array[4],tmp_change;
	memcpy(tmp_array,&tmp_var,sizeof(float));
	tmp_change = tmp_array[0];
	tmp_array[0] = tmp_array[2];
	tmp_array[2] = tmp_change;
	tmp_change = tmp_array[1];
	tmp_array[1] = tmp_array[3];
	tmp_array[3] = tmp_change;
	memcpy(&ret,&tmp_array[0],sizeof(float));
	return ret;
}

uint16 int16_to_bcd(uint16 n) {
	if (n > 9999) {
		return 0;
	}

	return ((n/1000)<<12)|((n/100%10)<<8)|((n%100/10)<<4)|(n%10);
}


uint8 bcd_to_int8(uint8 bcd)
{
	return (bcd>>4)*10+(bcd&0x0F);
}

uint16 bcd_to_int16(uint16 bcd)
{
	return ((uint16)bcd_to_int8(bcd>>8))*100+bcd_to_int8(bcd&0xFF);
}


