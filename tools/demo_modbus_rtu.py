#!/usr/bin/python
# -*- coding: utf-8 -*-

#串口小程序，实现modbus rtu协议，slave模式

import serial #导入串口模块，相当于C语言中的#include <serial.h>
import struct #导入struct模块，数据解压和压缩，C语言里的结构体
import json
import random

com1 = serial.Serial("COM1", 9600) #打开串口COM1，波特率为9600
com1.timeout = 1 #设置超时为1s

#register = [random.randint(0, 0xFFFF) for i in range(10000)] #10000个0xFFFF
register = [0xFFFF for i in range(10000)] #10000个0xFFFF
#[0xFFFF for i in range(10000)] #10000个0xFFFF

def crc16(*x): #crc16校验函数，低字节靠前
    b = 0xA001
    a = 0xFFFF
    for byte in x:
        a = a^byte
        for i in range(8):
            last = a%2
            a = a>>1
            if last ==1: a = a^b
    return [a&0xFF, (a>>8)&0xFF]

try:
    file = open("register.json", "r")
    config = file.read()
    register = json.loads(config)
    file.close()
except:
    pass

while 1: #循环接收串口数据
    a0 = com1.read(1) #读取第一个字节数据，slave号
    if(len(a0) != 1):        #读取失败返回False
        continue
    a0, = struct.unpack("B", a0) #解压收到的数据，一个8位整数
    if a0 < 0x01 or a0 > 0x99:   #判断slave号不能小于0x01或者大于0x99
        continue
    slave = a0
    a0 = com1.read(1) #读取第一个字节数据，功能码
    if(len(a0) != 1):
        continue
    a0, = struct.unpack("B", a0) #解压收到的数据，一个8位整数
    fun_no = a0
    #print a0
    #register = [random.randint(0, 0xFFFF) for i in range(10000)] #10000个0xFFFF
    if 0x03 == a0: #读取多个寄存器
        a0 = com1.read(6) #读取第6个字节数据，数据
        if(len(a0) != 6):
            continue
        r_start, r_len, crc_l, crc_h = struct.unpack(">HHBB", a0) #">"网络字节序，H 16位的无符号整数，B 8位的无符号整数
        if r_len < 1 or r_len > 0x7D:
            continue
        a1 = list(struct.unpack("B" * 4, a0[0:4]))
        crc16_sum = crc16(slave, fun_no, *a1) #crc16校验值
        if crc16_sum[0] != crc_l or crc16_sum[1] != crc_h: #判断校验是否正确
            continue
        data = [slave, fun_no, r_len*2] #打包
        for i in range(r_len):
            data += [(register[r_start + i]>>8)&0xFF, register[r_start + i]&0xFF]
        #data += [0xFF for i in range(r_len*2)] #有r_len*2个0xFF
        data += crc16(*data)
        com1.write(struct.pack("B" * len(data), *data))
    elif 0x06 == a0: #写单个寄存器
        a0 = com1.read(6) #读取第6个字节数据，数据
        if(len(a0) != 6):
            continue
        r_start, r_value, crc_l, crc_h = struct.unpack(">HHBB", a0) #">"网络字节序，H 16位的无符号整数，B 8位的无符号整数
        a1 = list(struct.unpack("B" * 4, a0[0:4]))
        crc16_sum = crc16(slave, fun_no, *a1) #crc16校验值
        if crc16_sum[0] != crc_l or crc16_sum[1] != crc_h: #判断校验是否正确
            continue
        if 29 == r_start:
            config = json.dumps(register) #将寄存器压缩成字符串
            file = open("register.json", "w") #存储
            file.write(config)
            file.close()
        register[r_start] = r_value
        data = [slave, fun_no, (r_start>>8)&0xFF, r_start&0xFF, (r_value>>8)&0xFF, r_value&0xFF] #打包
        data += crc16(*data)
        com1.write(struct.pack("B" * len(data), *data))
        
    elif 0x10 == a0: #写单个寄存器
        a0 = com1.read(5) #读取第6个字节数据，数据
        if(len(a0) != 5):
            continue
        r_start, r_len, b_len = struct.unpack(">HHB", a0) #">"网络字节序，H 16位的无符号整数，B 8位的无符号整数
        
        a0 = com1.read(b_len + 2)
        if(len(a0) != b_len + 2):
            continue
        a1 = struct.unpack("B" * b_len, a0[0:b_len])
        crc_l, crc_h = struct.unpack("B" * 2, a0[b_len:b_len+2])
        crc16_sum = crc16(slave, fun_no, (r_start>>8)&0xFF, r_start&0xFF, (r_len>>8)&0xFF, r_len&0xFF, b_len, *a1) #crc16校验值
        print crc16_sum, crc_l, crc_h
        if crc16_sum[0] != crc_l or crc16_sum[1] != crc_h: #判断校验是否正确
            print "crc16_sum[0] != crc_l or crc16_sum[1] != crc_h"
            continue
        r_values = list(struct.unpack(">" + "H" * r_len, a0[0:b_len]))
        register[r_start:r_start + r_len] = r_values
        data = [slave, fun_no, (r_start>>8)&0xFF, r_start&0xFF, (r_len>>8)&0xFF, r_len&0xFF] #打包
        data += crc16(*data)
        com1.write(struct.pack("B" * len(data), *data))
        