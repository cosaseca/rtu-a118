#!/usr/bin/python
# -*- coding: utf-8 -*-
import wx
import threading
import time
import serial
import struct
import binascii
import sys
import datetime
import os
import zigbee_data
import json
import upgrade
from serial.tools import list_ports

reload(sys)
sys.setdefaultencoding('utf8')

class Meter():
    @staticmethod
    def packValue(meter):
        data = []
        if "on" == meter["auto_change"]:
            value = []
            value_b_len = 0
            for i in range(len(meter["data"])):
                if "f" == meter["format"][i]:
                    value.append(meter["data"][i] + 0.01 * meter["index"])
                    value_b_len += 4
            data += struct.unpack("B" * value_b_len, struct.pack(">" + meter["format"][0:len(value)], *value))
            meter["index"] += 1
        else:
            value = []
            value_b_len = 0
            for i in range(len(meter["data"])):
                if "f" == meter["format"][i]:
                    value.append(meter["data"][i])
                    value_b_len += 4
            data += struct.unpack("B" * value_b_len, struct.pack(">" + meter["format"][0:len(value)], *value))
        return data
    @staticmethod
    def sndData(meter):
        if 0x0001 == meter["type"]: #载荷 
            if 0 == meter["flow"]: #常规数据
                data = [0x00, 0x00, 0x00, 0x06, 
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"], 
                        0x00, 0x00,
                        0x34, 0x23, 0x00, 0x78, 0x00, 0x00]
                data += Meter.packValue(meter)
            elif 1 == meter["flow"]: #参数
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x10,
                        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x01, 0x03, 0x00, 0x04,
                        0x00, 0x05, 0x00, 0x06, 0x00, 0x07,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10]
            elif 2 == meter["flow"]: #第一包
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x20,
                        0x00, 0x01, 0x01, 0x01,
                        0>>8, 0&0xFF, 2000>>8, 2000&0xFF, 0x00, 200,
                        #0x00, 6, 17840>>8, 17840&0xFF,
                        0x00, 6, 1779>>8, 1779&0xFF,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10]
            if 15 != meter["packIndex"]: #周期数据
                #R.log("载荷周期")
                data = [0x00, 0x00, 0x00, 0x06, 
                    meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"], 
                    0x00, 0x20,
                    meter["packIndex"], 0x00,  
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                #R.log("index " + str(meter["packIndex"]))
                
                for i in range(12,12+30,2):
                    j = ((meter["packIndex"] - 1) * 15 + (i - 12)/2)
                    #R.log("index j " + str(j))
                    if j >= 0 and j < 100:
                        tmp_a0 = 10 * ((zigbee_data.pos[j * 2]<<8) |  (zigbee_data.pos[j * 2 + 1])) #5 * j * 10
                        #R.log("index tmp_a0 " + str(tmp_a0))
                        data[i] = tmp_a0>>8
                        data[i+1] = tmp_a0&0xFF
                    elif j < 200 and j >= 100:
                        #tmp_a0 = (500 - 5 * (j - 100 + 1))*10
                        tmp_a0 = 10 * ((zigbee_data.pos[j * 2]<<8) |  (zigbee_data.pos[j * 2 + 1]))
                        #R.log("index tmp_a0 " + str(tmp_a0))
                        data[i] = tmp_a0>>8
                        data[i+1] = tmp_a0&0xFF
                    else:
                        data[i] = 0xFF
                        data[i+1] = 0xFF
                for i in range(12+30,12+60,2):
                    j = ((meter["packIndex"] - 1) * 15 + (i - 30 - 12)/2)
                    if j >= 0 and j < 100:
                        tmp_a0 = 250 * 10
                        data[i] = zigbee_data.loadData[j * 2] #tmp_a0>>8
                        data[i+1] = zigbee_data.loadData[j * 2 + 1] #tmp_a0&0xFF
                    elif j >= 100 and j < 200:
                        tmp_a0 = 210 * 10
                        data[i] = zigbee_data.loadData[j * 2] #tmp_a0>>8
                        data[i+1] = zigbee_data.loadData[j * 2 + 1] #tmp_a0&0xFF
                    else:
                        data[i] = 0xFF
                        data[i+1] = 0xFF
        elif 0x0004 == meter["type"]: #电参
            if 0 == meter["flow"]: #常规数据
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x00,
                        0x34, 0x23, 0x00, 0x78, 0x00, 0x00]
                data += Meter.packValue(meter)
            elif 1 == meter["flow"]: #参数
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x10,
                        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x01, 0x03, 0x00, 0x04,
                        0x00, 0x05, 0x00, 0x06, 0x00, 0x07,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10]
            elif 2 == meter["flow"]: #第一包
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x30,
                        0x00, 0x01, 0x01, 0x01,
                        0x00, 10, 2000>>8, 2000&0xFF, 0x00, 200]
                
            if 15 != meter["packIndex"]: #周期数据
                #R.log("电参周期")
                data = [0x00, 0x00, 0x00, 0x06, 
                    meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"], 
                    0x00, 0x30,
                    meter["packIndex"], 0x00,  
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                for i in range(12,12+30,2):
                    j = ((meter["packIndex"] - 1) * 15 + (i - 12)/2)
                    #R.log("index j " + str(j))
                    if j >= 0 and j < 100:
                        tmp_a0 = 1950 - 4 * j
                        #R.log("index tmp_a0 " + str(tmp_a0))
                        data[i] = zigbee_data.current[j * 2] #tmp_a0>>8
                        data[i+1] = zigbee_data.current[j * 2 + 1] #tmp_a0&0xFF
                    elif j < 200 and j >= 100:
                        tmp_a0 = 1550 + (j - 100 + 1) * 4
                        #R.log("index tmp_a0 " + str(tmp_a0))
                        data[i] = zigbee_data.current[j * 2] #tmp_a0>>8
                        data[i+1] = zigbee_data.current[j * 2 + 1] #tmp_a0&0xFF
                    else:
                        data[i] = 0xFF
                        data[i+1] = 0xFF
                for i in range(12+30,12+60,2):
                    j = ((meter["packIndex"] - 1) * 15 + (i - 30 - 12)/2)
                    if j >= 0 and j < 100:
                        tmp_a0 = 1000 - 15 * j
                        data[i] = zigbee_data.power[j * 2] #(tmp_a0>>8)&0xFF
                        data[i+1] = zigbee_data.power[j * 2 + 1] #tmp_a0&0xFF
                    elif j >= 100 and j < 200:
                        tmp_a0 = (j - 100 + 1) * 15 - 500
                        data[i] = zigbee_data.power[j * 2] #(tmp_a0>>8)&0xFF
                        data[i+1] = zigbee_data.power[j * 2 + 1] #tmp_a0&0xFF
                    else:
                        data[i] = 0xFF
                        data[i+1] = 0xFF
        else: #其它仪表
            if 0 == meter["flow"]: #常规数据
                data = [0x00, 0x00, 0x00, 0x06, 
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"], 
                        0x00, 0x00,
                        0x34, 0x23, 0x00, 0x78, 0x00, 0x00]
                data += Meter.packValue(meter)
                #if "on" == meter["auto_change"]:
                #    value = 0.01 * meter["index"]
                #    a0 = struct.unpack("B" * 4, struct.pack(">f", value))
                #    data[len(data)-4:len(data)] = a0
                #    meter["index"] += 1
            elif 1 == meter["flow"]: #参数
                data = [0x00, 0x00, 0x00, 0x06,
                        meter["type"]>>8, meter["type"] & 0xFF, meter["group"], meter["num"],
                        0x00, 0x10,
                        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                        0x01, 0x03, 0x00, 0x04,
                        0x00, 0x05, 0x00, 0x06, 0x00, 0x07,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10]
        
        head = [0x7E, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                   0x00, 0x00, 0x00, 0xFF, 0xFE, 0xE8, 0xE8, 0x00, 0x11, 0x18, 
                   0x57, 0x00, 0x00]
        data = head + data
        data_len = len(data)
        data[1] = ((data_len - 3) >> 8) & 0xFF
        data[2] = (data_len - 3) & 0xFF
        sum0 = 0
        for i in range(3, data_len): #计算校验和
            sum0 += data[i]
        sum0 = 0xFF - (sum0 & 0xFF)
        a1 = ""
        for i in data: #数据打包
            a1 += struct.pack("B", i)
        a1 += struct.pack("B", sum0)
        return a1
        
    @staticmethod
    def rcvData(meter, req):
        cmd = (req[29 - 3]<<8) | req[30 - 3]
        rc = True
        if 0x0001 == meter["type"]: #载荷
            if 0x0100 == cmd: #常规数据
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " comm data")
                rc = False
            elif 0x0101 == cmd: #参数
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " arg")
                meter["flow"] = 1
            elif 0x0200 == cmd: #第一包
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " load pack 00")
                tmp_str = str(req[31 - 3])
                for i in range(32-3,32-3+3*2,2):
                    tmp_str += " " + str((req[i]<<8) | (req[i + 1]))
                x1 = meter["interval"]
                if "auto" == meter["interval_type"]:
                    x1 = req[38 - 3]
                    if x1 < 5 or x1 > 20:
                        x1 = 20
                    meter["interval"] = x1 * 60
                R.log(tmp_str + " " + str(x1))
                meter["flow"] = 2
            elif 0x0201 == cmd: #周期数据
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " load pack " + str(req[31 - 3]))
                meter["packIndex"] = req[31 - 3] + 1
                if 15 == meter["packIndex"]:
                    meter["flow"] = 0
            elif 0x0210 == cmd: #电参#第一包
                R.log(u"电参" + " rcv elec pack 0000")
                tmp_str = str(req[31 - 3])
                for i in range(32-3,32-3+3*2,2):
                    tmp_str += " " + str((req[i]<<8) | (req[i + 1]))
                R.log(tmp_str)
                has_meter = False
                for m in R.meters:
                    if 0x0004 == m["type"] and req[27 - 3] == m["group"] and m["num"] == req[28 - 3]:
                        m["flow"] = 2
                        R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack 0000")
                        has_meter = True
                        break
                if not has_meter:
                    for m in R.meters:
                        if 0x0004 == m["type"] and m["num"] == req[28 - 3]:
                            m["flow"] = 2
                            R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack 0000")
                            has_meter = True
                            break
                rc = False
            elif 0x0211 == cmd: #周期数据
                R.log(u"电参" + " rcv elec pack " + str(req[31 - 3]))
                has_meter = False
                for m in R.meters:
                    if 0x0004 == m["type"] and req[27 - 3] == m["group"] and m["num"] == req[28 - 3]:
                        m["packIndex"] = req[31 - 3] + 1
                        R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack " + str(req[31 - 3]))
                        if 15 == m["packIndex"]:
                            m["flow"] = 0
                        has_meter = True
                        break
                if not has_meter:
                    for m in R.meters:
                        if 0x0004 == m["type"] and m["num"] == req[28 - 3]:
                            m["packIndex"] = req[31 - 3] + 1
                            R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack " + str(req[31 - 3]))
                            if 15 == m["packIndex"]:
                                m["flow"] = 0
                            has_meter = True
                            break
                rc = False
        elif 0x0004 == meter["type"]: #电参
            if 0x0100 == cmd: #常规数据
                if "auto" == meter["interval_type"]:
                    meter["interval"] = (req[31 - 3]<<8) | req[32 - 3]
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " comm data sleep " + str(meter["interval"]))
            elif 0x0101 == cmd: #参数
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " arg")
                meter["flow"] = 1
            elif 0x0210 == cmd: #第一包
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " elec pack 0000")
                tmp_str = str(req[31 - 3])
                for i in range(32-3,32-3+3*2,2):
                    tmp_str += " " + str((req[i]<<8) | (req[i + 1]))
                R.log(tmp_str)
                meter["flow"] = 2
            elif 0x0211 == cmd: #周期数据
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " elec pack " + str(req[31 - 3]))
                meter["packIndex"] = req[31 - 3] + 1
                if 15 == meter["packIndex"]:
                    meter["flow"] = 0
        else:
            if 0x0100 == cmd: #常规数据
                if "auto" == meter["interval_type"]:
                    meter["interval"] = (req[31 - 3]<<8) | req[32 - 3]
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " comm data sleep " + str(meter["interval"]))
            elif 0x0101 == cmd: #参数
                R.log(meter["name"] + " rcv group " + str(meter["group"]) + " arg")
                meter["flow"] = 1
            elif 0x0210 == cmd: #电参#第一包
                R.log(u"电参" + " rcv elec pack 00")
                tmp_str = str(req[31 - 3])
                for i in range(32-3,32-3+3*2,2):
                    tmp_str += " " + str((req[i]<<8) | (req[i + 1]))
                R.log(tmp_str)
                has_meter = False
                for m in R.meters:
                    if 0x0004 == m["type"] and req[27 - 3] == m["group"] and m["num"] == req[28 - 3]:
                        R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack 0000")
                        m["flow"] = 2
                        has_meter = True
                        break
                if not has_meter:
                    for m in R.meters:
                        if 0x0004 == m["type"] and m["num"] == req[28 - 3]:
                            m["flow"] = 2
                            R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack 0000")
                            has_meter = True
                            break
                rc = False
            elif 0x0211 == cmd: #周期数据
                R.log(u"电参" + " rcv elec pack " + str(req[31 - 3]))
                has_meter = False
                for m in R.meters:
                    if 0x0004 == m["type"] and req[27 - 3] == m["group"] and m["num"] == req[28 - 3]:
                        m["packIndex"] = req[31 - 3] + 1
                        R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack " + str(req[31 - 3]))
                        if 15 == m["packIndex"]:
                            m["flow"] = 0
                        has_meter = True
                        break
                if not has_meter:
                    for m in R.meters:
                        if 0x0004 == m["type"] and m["num"] == req[28 - 3]:
                            m["packIndex"] = req[31 - 3] + 1
                            R.log(u"电参" + " rcv group " + str(m["group"]) + " elec pack " + str(req[31 - 3]))
                            if 15 == m["packIndex"]:
                                m["flow"] = 0
                            has_meter = True
                            break
                rc = False
        return rc
                        
class R():
    serial = None
    
    name = "COM5"
    
    baud = 9600
    
    logFile = None
    
    logBox = None
    
    logFilePath = "log.txt"
    
    configPath = "config.txt"
    
    displaySendData = True
    
    meters = zigbee_data.meters
    
    meterTypes = zigbee_data.meterTypes
    
    @staticmethod
    def log(msg):
        if R.logFile:
            R.logFile.write(time.ctime() + " " + msg + "\r\n")
        #print time.ctime() + " " + msg
        if R.logBox:
            pass
            R.logBox.AppendText(time.ctime() + " " + msg + "\r\n")
            
    @staticmethod
    def logTest(msg):
        if R.logFileTest:
            R.logFileTest.write(time.ctime() + " " + msg + "\r\n")
        print time.ctime() + " " + msg
        if R.logBoxTest:
            R.logBoxTest.AppendText(time.ctime() + " " + msg + "\r\n")
    

class MeterRcvThread(threading.Thread):
    def __init__(self, name="COM5", baud=9600):
        threading.Thread.__init__(self)
        self.serial = serial.Serial(name, baud)
        self.serial.timeout = 1
        R.serial = self.serial
        
    def send(self, msg):
        #R.log("snd " + binascii.hexlify(msg))
        if R.displaySendData:
            R.log("snd " + binascii.hexlify(msg[3:]))
        self.serial.write(msg)
        
    def rcvHandler(self, m):
        a0 = self.serial.read(1)
        if(len(a0) != 1):
            return False
        a0, = struct.unpack("B", a0)
        if 0x7E != a0:
            return False
        a0 = self.serial.read(2)
        if(len(a0) != 2):
            return False
        a0 = struct.unpack("B" * 2, a0)
        a0_len = ((a0[0]<<8) | a0[1]) + 1
        if a0_len < 27 or a0_len > 1024:
            return False
        a0 = self.serial.read(a0_len)
        if(len(a0) != a0_len):
            return False
        b0 = struct.unpack(">" + "B" * len(a0), a0)
        R.log("rcv " + binascii.hexlify(a0))
        return Meter.rcvData(m, b0)
                
    def sndHandler(self):
        tm = time.time()
        for m in R.meters:
            action = "no"
            if "on" != m["power"]:
                continue
            if 0x0001 == m["type"] and 15 != m["packIndex"]:
                action = "go"
                R.log(m["name"] + " snd load pack " + str(m["packIndex"]))
            elif 0x0004 == m["type"] and 15 != m["packIndex"]:
                action = "go"
                R.log(m["name"] + " snd elec pack " + str(m["packIndex"]))
            elif 0 != m["flow"]:
                action = "go"
            elif tm - m["time"] > m["interval"]:
                if 0x0001 == m["type"]:
                    for mm in R.meters:
                        if 0x0004 == mm["type"] and mm["group"] == m["group"] and mm["num"] == m["num"]:
                            if 15 == mm["packIndex"]:
                                action = "go"
                                break
                else:
                    action = "go"
            if action == "go":
                a0 = Meter.sndData(m)
                R.log(m["name"] + " snd group " + str(m["group"]))
                self.send(a0)
                time.sleep(0.1)
                if 1 != m["flow"]:
                    again_count = 0
                    while not self.rcvHandler(m):
                        if again_count > 3:
                            break
                        R.log("again")
                        again_count += 1
                    if again_count <= 3:
                        m["time"] = tm
                    else:
                        m["flow"] = 0
                else:
                    m["flow"] = 0
        #time.sleep(0.5)
        return 1
                    
    def run(self):
        while R.serial:
            self.sndHandler()
            time.sleep(1)
            
class ZigbeePanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        
        rcvBoxHeight = 355
        rcvBoxWidth = 500
        
        self.nameListBox = wx.ComboBox(self, -1, value = "COM5", 
            choices = [str(port) for port, desc, hwid in sorted(list_ports.comports())], 
            pos=(5, 5),
            size = (rcvBoxWidth,28),
            style = wx.CB_DROPDOWN)
            
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
        
        self.runBtn = wx.Button(self, -1, u"运行", pos=(rcvBoxWidth + 5, 3), size=(100, 28))
        self.Bind(wx.EVT_BUTTON, self.OnClick, self.runBtn)
        
        self.rcvBox = wx.TextCtrl(self,-1,u"接收区域", pos=(5, 35),
                              size=(rcvBoxWidth,rcvBoxHeight),
                              style=wx.TE_MULTILINE)
        R.logBox = self.rcvBox
    
    def comScanHandler(self, event):
        name = self.nameListBox.GetValue()
        self.nameListBox.Set([str(port) for port, desc, hwid in sorted(list_ports.comports())])
            #ImageUpgradePanel.log(port + " ")
            #if wx.NOT_FOUND == self.nameListBox.FindString(port):
            #    self.nameListBox.Append(port)
        self.nameListBox.SetValue(name)
        

    def OnClick(self, event):
        if not R.serial:
            meterRcv = MeterRcvThread(self.nameListBox.GetValue(), R.baud)
            meterRcv.start()
        elif R.serial:
            R.serial.close()
            R.serial = None
        if R.serial:
            self.runBtn.SetLabel(u"停止")
        elif not R.serial:
            self.runBtn.SetLabel(u"运行")
            
class MeterItemPanel(wx.Panel):
    def __init__(self, parent, meter=None, pos=(0, 0)):
        if not meter:
            return
        wx.Panel.__init__(self, parent, -1, pos, size=(580, 28))
        self.meter = meter
        x_offset = 38
        wx.StaticText(self, -1, str(meter["item_num"]), pos=(0, 3))
        self.nameListBox = wx.ComboBox(self, -1, value = meter["name"], 
            choices = R.meterTypes["name"], 
            pos=(x_offset, 0),
            size = (100,28),
            style = wx.CB_READONLY)
        self.groupListBox = wx.ComboBox(self, -1, value = str(meter["group"]), 
            choices = R.meterTypes["group"], 
            pos=(105 + x_offset, 0),
            size = (50,28),
            style = wx.CB_READONLY)
        self.intervalListBox = wx.ComboBox(self, -1, value = str(meter["interval_type"]), 
            choices = R.meterTypes["interval"], 
            pos=(160 + x_offset, 0),
            size = (80,28),
            style = wx.CB_DROPDOWN)
        self.autoChangeListBox = wx.ComboBox(self, -1, value = meter["auto_change"], 
            choices = R.meterTypes["power"], 
            pos=(245 + x_offset, 0),
            size = (50,28),
            style = wx.CB_READONLY)
        self.powerListBox = wx.ComboBox(self, -1, value = meter["power"], 
            choices = R.meterTypes["power"], 
            pos=(300 + x_offset, 0),
            size = (50,28),
            style = wx.CB_READONLY)
            
        self.dataBox = wx.TextCtrl(self,-1, ", ".join([str(x)for x in meter["data"]]), pos=(355 + x_offset, 0),
                              size=(140,-1))
            
        self.delBtn = wx.Button(self,-1,u"删除", pos=(500 + x_offset, 0),
                              size=(40,28),
                              style=wx.TE_MULTILINE)
        self.Bind(wx.EVT_BUTTON, self.delHandler, self.delBtn)
        
    def delHandler(self, event):
        self.Destroy()
        
    def getMeter(self):
        name = self.nameListBox.GetValue()
        power = self.powerListBox.GetValue()
        interval = self.intervalListBox.GetValue()
        group = int(self.groupListBox.GetValue())
        #num =  R.meterTypes["num"][self.nameListBox.FindString(self.nameListBox.GetValue())]
        #type = R.meterTypes["type"][self.nameListBox.FindString(self.nameListBox.GetValue())]
        auto_change = self.autoChangeListBox.GetValue()
        type_index = self.nameListBox.FindString(self.nameListBox.GetValue())
        data = [float(x) for x in self.dataBox.GetValue().split(",")]
        m = {
            "name": name,
            "power": power,
            "time": 0,
            "interval": 7,
            "interval_type": interval,
            "group": group,
            "num": R.meterTypes["num"][type_index],
            "type": R.meterTypes["type"][type_index],
            "index": 0,
            "auto_change": auto_change,
            "flow": 0,
            "packIndex": 15,
            "data": data,
            "format": R.meterTypes["format"][type_index],
        }
        if "auto" != interval:
            m["interval"] = int(interval)
        return m
                              
class MeterSetPanel(wx.ScrolledWindow):
    def __init__(self, parent):
        wx.ScrolledWindow.__init__(self, parent, -1)
        self.scanBtn = wx.Button(self,-1,u"扫描", pos=(6, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        self.addBtn = wx.Button(self,-1,u"添加", pos=(110, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        self.applyBtn = wx.Button(self,-1,u"应用", pos=(215, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        self.saveBtn = wx.Button(self,-1,u"保存", pos=(320, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
                              
        self.Bind(wx.EVT_BUTTON, self.scanHandler, self.scanBtn)
        self.Bind(wx.EVT_BUTTON, self.addHandler, self.addBtn)
        self.Bind(wx.EVT_BUTTON, self.applyHandler, self.applyBtn)
        self.Bind(wx.EVT_BUTTON, self.saveHandler, self.saveBtn)
        #size=(630, 465)
        x_offset = 38
        wx.StaticText(self, -1, u"索引", pos=(6, 33))
        wx.StaticText(self, -1, u"仪表", pos=(x_offset + 6, 33))
        wx.StaticText(self, -1, u"组号", pos=(x_offset + 105 + 6, 33))
        wx.StaticText(self, -1, u"休眠时间(s)", pos=(x_offset + 160 + 6, 33))
        wx.StaticText(self, -1, u"自增长", pos=(x_offset + 245 + 6, 33))
        wx.StaticText(self, -1, u"开关", pos=(x_offset + 300 + 6, 33))
        wx.StaticText(self, -1, u"默认值", pos=(x_offset + 355 + 6, 33))
        self.items = {}
        self.SetScrollbars(1, 1, 500, 300)
        self.SetScrollRate(50, 50)
        self.scanHandler(None)
    
    def scanHandler(self, event):
        i = 0
        for key in self.items:
            if self.items[key] and not self.items[key].IsBeingDeleted():
                self.items[key].Destroy()
        self.items = {}
        for m in R.meters:
            m["item_num"] = i + 1
            self.items[i] = MeterItemPanel(self, meter=m, pos=(6, i * 30 + 60))
            i += 1
        self.SetScrollbars(1, 1, 500, i * 30 + 90)
        self.SetScrollRate(50, 30)
    def addHandler(self, event):
        m = {
            "name": u"油压",
            "power": "on",
            "time": 0,
            "interval": 2,
            "interval_type": "auto",
            "group": 1,
            "num": 1,
            "type": 0x0002,
            "index": 0,
            "auto_change": "on",
            "flow": 0,
            "packIndex": 15,
            "item_num": 0,
            "data": [0.01],
            "format": "f",
        }
        i = len(self.items)
        m["item_num"] = i + 1
        self.items[i] = MeterItemPanel(self, 
                        meter=m, 
                        pos=(6, i * 30 + 60))
        self.SetScrollbars(1, 1, 500, i * 30 + 90)
        self.SetScrollRate(50, 30)
        
    def applyHandler(self, event):
        R.meters = [self.items[key].getMeter() for key in self.items if self.items[key] and not self.items[key].IsBeingDeleted()]

    def saveHandler(self, event):
        R.meters = [self.items[key].getMeter() for key in self.items if self.items[key] and not self.items[key].IsBeingDeleted()]
        config = json.dumps(R.meters)
        file = open(R.configPath, "w")
        file.write(config)
        file.close()
    
class TestPanel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        rcvBoxHeight = 170
        rcvBoxWidth = 500
        self.nameListBox = wx.ComboBox(self, -1, value = "udp", 
            choices = ["udp", "tcp client", "tcp server"], 
            pos=(5, 5),
            size = (rcvBoxWidth,28),
            style = wx.CB_DROPDOWN)
         
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
        
        self.rcvBox = wx.TextCtrl(self,-1,u"接收区域", pos=(5, 35),
                              size=(rcvBoxWidth,rcvBoxHeight),
                              style=wx.TE_MULTILINE)
                              
        self.runBtn = wx.Button(self,-1,"run", pos=(rcvBoxWidth + 6, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
                              
        self.sndBox = wx.TextCtrl(self,-1,u"发送区域", pos=(5, rcvBoxHeight + 50),
                              size=(rcvBoxWidth,rcvBoxHeight),
                              style=wx.TE_MULTILINE)
                              
        self.sndBtn = wx.Button(self,-1,u"发送", pos=(rcvBoxWidth + 6, 218),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        
        self.Bind(wx.EVT_BUTTON, self.runHandler, self.runBtn)
        self.Bind(wx.EVT_BUTTON, self.sndHandler, self.sndBtn)
    
    def comScanHandler(self, event):
        name = self.nameListBox.GetValue()
        self.nameListBox.Set(["udp", "tcp client", "tcp server"] + 
            [str(port) for port, desc, hwid in sorted(list_ports.comports())])
            #ImageUpgradePanel.log(port + " ")
            #if wx.NOT_FOUND == self.nameListBox.FindString(port):
            #    self.nameListBox.Append(port)
        self.nameListBox.SetValue(name)
    
    def runHandler(self, event):
        R.log(self.nameListBox.GetValue())
        file_wildcard = "Paint files(*.paint)|*.paint|All files(*.*)|*.*" 
        dlg = wx.FileDialog(self, "Open paint file...",
                            os.getcwd(), 
                            style = wx.OPEN,
                            wildcard = file_wildcard)
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetPath()
            #self.ReadFile()
            #self.SetTitle(self.title + '--' + self.filename)
            print filename
        dlg.Destroy()
        
    def sndHandler(self, event):
        R.log(self.nameListBox.GetValue())
        #frame = ZigbeeFrame()
        #frame.Show()
        
class ImageUpgradePanel(wx.Panel):
    logBox = None
    startBtn = None
    @staticmethod
    def log(msg):
        if True == msg:
            ImageUpgradePanel.startBtn.SetLabel(u"停止")
        elif False == msg:
            ImageUpgradePanel.startBtn.SetLabel(u"开始升级")
        elif ImageUpgradePanel.logBox:
            ImageUpgradePanel.logBox.AppendText(time.ctime() + " " + msg + "\r\n")
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)
        self.filename = None
        self.selectFileBtn = wx.Button(self,-1,u"选择镜像", pos=(506, 3),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        self.Bind(wx.EVT_BUTTON, self.selectFileHandler, self.selectFileBtn)
        
        self.nameListBox = wx.ComboBox(self, -1, value = "tcp client", 
            choices = ["udp", "tcp client", "tcp server"], 
            pos=(5, 5),
            size = (100,28),
            style = wx.CB_DROPDOWN)
            
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
            
        self.ipBox = wx.TextCtrl(self,-1,"192.168.10.10", pos=(110, 5),
                              size=(150,-1))
                              
        self.portBox = wx.TextCtrl(self,-1,"7000", pos=(265, 5),
                              size=(100,-1))
        
        self.rcvBox = wx.TextCtrl(self,-1,u"接收区域", pos=(5, 33),
                              size=(500,355),
                              style=wx.TE_MULTILINE)
                              
        ImageUpgradePanel.logBox = self.rcvBox
        
        self.upgradeBtn = wx.Button(self,-1,u"开始升级", pos=(506, 3 + 60),
                              size=(100,28),
                              style=wx.TE_MULTILINE)
        self.Bind(wx.EVT_BUTTON, self.upgradeHandler, self.upgradeBtn)
        
        ImageUpgradePanel.startBtn = self.upgradeBtn
        
        self.client = None
        
    def selectFileHandler(self, event):
        file_wildcard = "All files(*.*)|*.*" 
        dlg = wx.FileDialog(self, "Open paint file...",
                            os.getcwd(), 
                            style = wx.OPEN,
                            wildcard = file_wildcard)
        if dlg.ShowModal() == wx.ID_OK:
            self.filename = dlg.GetPath()
            print self.filename
            ImageUpgradePanel.log(self.filename)
        dlg.Destroy()
        
    def upgradeHandler(self, event):
        if not self.filename:
            ImageUpgradePanel.log("file name error")
            return
        ImageUpgradePanel.log(self.filename)
        
        if None != self.client and self.client.running:
            self.client.close()
            return
        self.client = None    
        type = str(self.nameListBox.GetValue())
        if "tcp client" == type:
            self.client = upgrade.TcpClientServer(ip=str(self.ipBox.GetValue()), port=str(self.portBox.GetValue()), imagePath=self.filename, log=ImageUpgradePanel.log)
        elif "COM" == type[0:3]:
            self.client = upgrade.PortClient(name=type, baud=int(9600), imagePath=self.filename, log=ImageUpgradePanel.log)
        #client = upgrade.TcpClient(ip="127.0.0.1", port="10086", imagePath=self.filename, log=ImageUpgradePanel.log)
        self.client.start()
        
    def comScanHandler(self, event):
        #self.nameListBox
        name = self.nameListBox.GetValue()
        self.nameListBox.Set(["udp", "tcp client", "tcp server"] + 
            [str(port) for port, desc, hwid in sorted(list_ports.comports())])
            #ImageUpgradePanel.log(port + " ")
            #if wx.NOT_FOUND == self.nameListBox.FindString(port):
            #    self.nameListBox.Append(port)
        self.nameListBox.SetValue(name)

class ZigbeeFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, 'zigbee', 
                size=(630, 465))
        #self.SetIcon(wx.Icon(name='zigbee_test.ico', type=wx.BITMAP_TYPE_ICO))
        self.Center()
        self.Bind(wx.EVT_CLOSE, self.close, self)
        
        nb = wx.Notebook(self)
        nb.AddPage(TestPanel(nb), u"调试助手")
        nb.AddPage(ZigbeePanel(nb), u"Zigbee测试")
        nb.AddPage(MeterSetPanel(nb), u"仪表设置")
        nb.AddPage(ImageUpgradePanel(nb), u"固件升级")
        
    def close(self, event):
        if R.serial:
            R.serial.close()
        R.serial = None
        R.logBox = None
        R.logFile = None
        sys.exit(0)
        
if __name__ == '__main__':
    R.logFilePath = "log-" + time.ctime().replace(" ", "-").replace(":", "_") + ".txt"
    R.logFile = open(R.logFilePath, "w")
    try:
        file = open(R.configPath, "r")
        config = file.read()
        R.meters = json.loads(config)
        file.close()
    except:
        R.meters = zigbee_data.meters
    app = wx.App()
    frame = ZigbeeFrame()
    frame.Show()
    app.MainLoop()
