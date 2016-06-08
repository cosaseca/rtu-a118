#!/usr/bin/python
# -*- coding:utf-8 -*-

import threading
import socket
import struct
import binascii
import time
from Tkinter import *
import tkFont

#print binascii.hexlify(rdata)

data = [
    {
        "well": 1,
        "addr": "40001~40004",
        "label": ["井站类型 ", ", 设备厂家 ", ", 型号版本 ", ", 口令 "],
        "format": "HHXH",
        "data": [1, 6, 0xA118, 118],
        "act": "GET"
        },
    {
        "well": 1,
        "addr": "40005~40010",
        "label": ["时间 ", ":", ":", ", 日期", "-", "-"],
        "format": "XXXXXX",
        "data": [0x12, 0x12, 0x12, 0x2014, 0x12, 0x12],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40011~40015",
        "label": ["RAM电池电压 ", ", RTU柜内温度 ", ", 日时间起点 ", ":", ":", ""],
        "format": "HHXXX",
        "data": [0, 0, 0x12, 0x2014, 0x00],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40029~40030",
        "label": ["重启次数 ", ", 配置参数 "],
        "format": "HH",
        "data": [0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40031~40038",
        "label": ["通信方式 ", ", 通信协议 ", ", 终端通信地址 ", ", 波特率 ", ", 数据位 ", ", 停止位 ", ", 奇偶校验 ", ", 半全双工 "],
        "format": "H" * 8,
        "data": [0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40039~40050",
        "label": ["本地ip地址 ", ".", ".", ".", ", 子网掩码 ", ".", ".", ".", ", 网关 ", ".", ".", "."],
        "format": "H" * 12,
        "data": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40051~40056",
        "label": ["MAC地址 ", ":", ":", ":", ":", ":"],
        "format": "X" * 6,
        "data": [0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40057~40066",
        "label": ["TCP/UDP标识 ", ", 本地UDP端口号 ", ", 本地TCP端口号 ", ", 主站IP地址 ", ".", ".", ".", ", 主站端口号 ", ", \n主站通信方式 ", ", 下行通信接口 "],
        "format": "H" * 10,
        "data": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40067~40094",
        "label": ["本地ipv6地址 ", ":", ":", ":", ":", ":", ":", ":", ", 子网掩码 ", ", ipv6路由地址 ", ":", ":", ":", ":", ":", ":", ":", ", \n本地UDP端口号 ", ", 本地TCP端口号 ", ", 远端ipv6地址 ", ":", ":", ":", ":", ":", ":", ":", ", 主机端口号 "],
        "format": "X" * 8 + "H" + "X" * 8 + "HH" + "X" * 8 + "H",
        "data": [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40100~40108",
        "label": ["Pandid ", "", ", 通信模式 ", ", 信道号 ", ", 加密使能 ", ", 加密值 ", "", "", ""],
        "format": "X" * 9,
        "data": [0, 0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40109~40123",
        "label": ["服务器1ipv4地址 ", ".", ".", ".", ", 服务器1端口号 ", ", 服务器2ipv4地址 ", ".", ".", ".", ", \n服务器2端口号 ", ", 服务器3ipv4地址 ", ".", ".", ".", ", 服务器3端口号 "],
        "format": "H" * 15,
        "data": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "49000~49017",
        "label": ["第一井口ID ", "", ", 第二井口ID ", "", ", 第三井口ID ", "", ", 第四井口ID ", "", ", 第五井口ID ", "", ", 第六井口ID ", "", ", \n第七井口ID ", "", ", 第八井口ID ", "", ", RTU_ID ", ""],
        "format": "X" * 18,
        "data": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "49022~49024",
        "label": ["心跳状态 ", ", 无应答时心跳间隔 ", ", 有应答时心跳间隔 "],
        "format": "H" * 3,
        "data": [0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40300~40311",
        "label": ["油压值 ", ", 套压值 ", ", 回压值 ", ", 井口温度 ", ", \n载荷值 ", ", 加速度值 "],
        "format": "F" * 6,
        "data": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40351~40374",
        "label": ["A相电流 ", ", B相电流 ", ", C相电流 ", ", \nA相电压 ", ", B相电压 ", ", C相电压 ", ", \n电机无功功耗 ", ", 电机有功功耗", ", 电机无功功率 ", ", \n电机有功功率", ", 电机反向功率", ", 电机功率因数"],
        "format": "F" * 12,
        "data": [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40323~40326",
        "label": ["油压采集间隔 ", ", 套压采集间隔 ", ", 回压采集间隔 ", ", 温度采集间隔 "],
        "format": "H" * 4,
        "data": [0, 0, 0, 0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40375~40375",
        "label": ["电参数采集间隔 "],
        "format": "H",
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40436~40436",
        "label": ["一体化采集间隔 "],
        "format": "H",
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40327~40350",
        "label": ["油压采集时间 ", "-", "-", " ", ":", ":", ", 套压采集时间 ", "-", "-", " ", ":", ":", ", \n回压采集时间 ", "-", "-", " ", ":", ":", ", 井口温度采集时间 ", "-", "-", " ", ":", ":"],
        "format": "X" * 24,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "42001~42018",
        "label": ["一体化STL采集时间 ", "-", "-", " ", ":", ":", ", 载荷点套压采集时间 ", "-", "-", " ", ":", ":", ", \n电参点采集时间 ", "-", "-", " ", ":", ":"],
        "format": "X" * 18,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "40500~40515",
        "label": ["DI ", "", "", "", "", "", "", "", ", DO ", "", "", "", "", "", "", ""],
        "format": "X" * 16,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44000~44008",
        "label": ["油压变送器 厂商代码 ", ", 仪表类型 ", ", 仪表组号 ", ", 仪表编号 ", ", 通信效率 ", ", \n电池电压 ", ", 休眠时间 ", ", 仪表状态 ", ", 工作温度 "],
        "format": "H" * 9,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44020~44039",
        "label": ["套压变送器 仪表信息 ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "\n ", " ", " ", " ", " ", " ", " ", " "],
        "format": "H" * 20,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44040~44059",
        "label": ["回压变送器 仪表信息 ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "\n ", " ", " ", " ", " ", " ", " ", " "],
        "format": "H" * 20,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44060~44079",
        "label": ["温度变送器 仪表信息 ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "\n ", " ", " ", " ", " ", " ", " ", " "],
        "format": "H" * 20,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44090~44109",
        "label": ["载荷 仪表信息 ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "\n ", " ", " ", " ", " ", " ", " ", " "],
        "format": "H" * 20,
        "data": [0],
        "act": "GET"
    },
    {
        "well": 1,
        "addr": "44140~44159",
        "label": ["电参 仪表信息 ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "\n ", " ", " ", " ", " ", " ", " ", " "],
        "format": "H" * 20,
        "data": [0],
        "act": "GET"
    },
]


class TcpClient(threading.Thread):
    def __init__(self, ip="::", port="10086", app=None):
        threading.Thread.__init__(self)
        self.ip = str(ip)
        self.port = str(port)
        self.app = app
        print "ip:", self.ip, "port:", self.port

    def request(self, slave, start, stop):
        len = stop - start + 1
        str = struct.pack("12B", 0x00, 0x04,
                          0x00, 0x00, 0x00,
                          0x06, slave, 0x03,
                          start >> 8, start & 0xff, len >> 8, len & 0xff)
        return str
    
    def response(self, str0, item):
        format = ">"
        for x in item["format"]:
            if "H" == x or "X" == x :
                format += "H"
            elif "F":
                format += "f"
#        print format, len(str[9:]), ">" + "H" * (len(str[9:])/2)
        str0 = struct.unpack(format, str0[9:])
        
        str1 = ""
        i = 0
        for x in item["format"]:
            if "H" == x:
                str1 += item["label"][i] + str(str0[i])
            elif "X" == x:
                str1 += item["label"][i] + hex(str0[i])[2:]
            elif "F":
                str1 += item["label"][i] + str(str0[i])
            i += 1
        return str1
    
    def run(self):
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
            except socket.error, msg:
                s = None
                continue
            try:
                s.connect(sa)
            except socket.error, msg:
                s.close()
                s = None
                continue
            break
        if None == s:
            return
        self.sock = s
        while 1:
            time0 = time.time()
            #print "开始请求时间", time.ctime()
            str0 = ""
            for item in data:
                well = item["well"]
                start, stop = [int(x) - 40001 for x in item["addr"].split("~")]
                if "GET" == item["act"]:
                    s.send(self.request(well, start, stop))
                    rdata = s.recv(1024)
                    rdata = self.response(rdata, item)
                    #str0 +=  "-" * 100 + "\n"
                    str0 += rdata + "\n"
            #print str0
            item = self.app.canvas.find_withtag("info")
            self.app.canvas.itemconfig(item, text = str0)
            #print u"回复时间及差值", time.ctime(), time.time() - time0
            #print "\n" * 3
            time.sleep(3)

class App(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        root = Tk()

        ft = tkFont.Font(size=12)
        
        self.canvas = Canvas(root, width = 900, height = 900, bg = 'white')
        self.canvas.create_text(5, 5, text = 'Use Canvas', fill = 'black', tag = "info", anchor='nw', font=ft)
        self.canvas.pack()

        self.client = TcpClient(ip="192.168.8.26", port="502", app=self)
        self.client.start()
        root.mainloop()

if __name__ == "__main__":
    app = App()
    app.run()
