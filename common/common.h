/*
 * common.h
 *
 *  Created on: Sep 26, 2014
 *      Author: caspar
 */

#ifndef COMMON_H_
#define COMMON_H_

//基本数据类型

typedef unsigned char uint8;                   //无符号8位整型变量
typedef signed char int8;                    //有符号8位整型变量
typedef unsigned short uint16;                  //无符号16位整型变量
typedef signed short int16;                   //有符号16位整型变量
typedef unsigned long uint32;                  //无符号32位整型变量
typedef signed long int32;                   //有符号32位整型变量
typedef unsigned long long uint64;                  //有符号64位整型变量
typedef signed long long int64;                   //无符号64位整型变量
typedef float fp32;                    //单精度浮点数（32位长度）
typedef double fp64;					 //双精度浮点数（64位长度）

typedef unsigned char u8;                   	 //无符号8位整型变量
typedef unsigned short u16;              		 //无符号16位整型变量
typedef unsigned long u32;                     //无符号32位整型变量
typedef unsigned long long u64;					 //无符号32位整型变量

/* takes a byte out of a uint32 : var - uint32,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) \
  (uint8)((uint32)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
  ((uint32)((uint32)((Byte0) & 0x00FF) \
    + ((uint32)((Byte1) & 0x00FF) << 8) \
      + ((uint32)((Byte2) & 0x00FF) << 16) \
        + ((uint32)((Byte3) & 0x00FF) << 24)))

#define BUILD_UINT322(Word0, Word1) \
  ((uint32)((uint32)((Word0) & 0x0000FFFF) \
    + ((uint32)((Word1) & 0x0000FFFF) << 16)))

#define BUILD_UINT64(loByte, hiByte) \
  ((uint64)(((loByte) & 0xFFFFFFFF) + (((hiByte) & 0xFFFFFFFF) << 32)))

#define BUILD_UINT16(loByte, hiByte) \
  ((uint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define BUILD_UINT8(loByte,hiByte ) \
  ((uint8)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)

extern u32 swap_register32(u32 var);
extern u64 swap_register64(u64 var);
extern uint16 swap_int16(uint16 n);
extern uint32 swap_int32(uint32 n);
extern uint64 swap_int64(uint64 n);
extern float swap_rcv_float_seq(float var);
extern float swap_stor_float_seq(float var);
extern uint16 int16_to_bcd(uint16 n);
extern uint8 bcd_to_int8(uint8 bcd);
extern uint16 bcd_to_int16(uint16 bcd);


#endif /* COMMON_H_ */
