#!/usr/bin/python
# -*- coding: utf-8 -*-
import threading
import time
import struct
import binascii
import sys
import datetime
import os
import socket
import serial
import select
import re
import wx
import  wx.lib.mixins.listctrl as listmix
import images

reload(sys)
sys.setdefaultencoding('utf8')

class TcpClientServer(threading.Thread):
    def __init__(self, ip="127.0.0.1", port="10086", imagePath=None, log=lambda x:x):
        threading.Thread.__init__(self)
        self.client = TcpClient(ip, port, imagePath, log)
        self.running = False
        self.log = log
        self.power = True
        
    def close(self):
        self.power = False
        self.client.close()
        
    def run(self):
        self.running = True
        self.log(self.running)
        rc = False
        while not rc and self.power:
            #print self.power
            try:
                rc = self.client.run()
            except:
                pass
        self.running = False
        self.log(self.running)

class TcpClient(threading.Thread):
    def __init__(self, ip="127.0.0.1", port="10086", imagePath=None, log=lambda x:x):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.imagePath = imagePath
        self.log = log
        self.sock = None
        
    def crc16(self, x):
        b = 0xA001
        a = 0xFFFF
        for byte in x:
            a = a^byte
            for i in range(8):
                last = a%2
                a = a>>1
                if last ==1: a = a^b
        aa = '0'*(6-len(hex(a)))+hex(a)[2:]
        ll,hh = int(aa[:2],16),int(aa[2:],16)
        return [ll,hh]
        
    def close(self):
        self.sock.close()
        
    def recv(self, r_len):
        data = ""
        while len(data) < r_len:
            data += self.sock.recv(r_len - len(data))
        return data
        
    def send(self, data):
        return self.sock.send(data)
        
    def request(self, slave, start, length):
        str0 = struct.pack("12B", 0x00, 0x04,
                          0x00, 0x00, 0x00,
                          0x06, slave, 0x03,
                          start >> 8, start & 0xff, length >> 8, length & 0xff)
        return str0

    def response(self, str0):
        str0 = str0[9:]
        str0 = struct.unpack(">" + "H" * (len(str0)/2), str0)
        return str0
        
    def vercomp(self):
        req = self.request(1, 40003 - 40001, 1)
        self.send(req)
        rep = self.recv(9 + 2)
        if 9 + 2 != len(rep):
            raise Exception("rcv error")
        sRev, = self.response(rep)
        self.log("soft rev : " + str(sRev))
        r = re.compile(".*8003.*-BIN-Rev\d*\.\d*-Rev([0-9]*).*\.bin")
        m = r.search(self.imagePath)
        if m:
            self.log("image soft rev : " + m.group(1))
            if sRev == int(m.group(1)):
                return True
        else:
            self.log("image error")
            return True
        return False
        
    def upgradeHandler(self):
        if self.vercomp():
            return True
        print self.imagePath
        imgSize = os.path.getsize(self.imagePath)
        print imgSize
        self.log("imgSize: " + str(imgSize))
        packSize = 128
        packIndex = 0
        totalPackNum = imgSize/packSize
        if imgSize%packSize:
            totalPackNum += 1
        print "totalPackNum", totalPackNum
        self.log("totalPackNum: " + str(totalPackNum))
        a = struct.pack("BB", 0xAA, 0x55)                             #头部
        a += struct.pack(">I", packIndex)                             #起始包
        a += struct.pack(">I", imgSize)                               #文件长度
        a += struct.pack(">H", packSize)                              #每包字节数
        a += struct.pack(">I", totalPackNum)                          #总包数
        a += struct.pack(">I", 0)                                     #文件总校验和
        a += "123456789123"
        print len(a)
        crc = self.crc16(struct.unpack("B" * len(a), a))
        a += struct.pack("BB", *crc)
        print len(a)
        rc = False
        max_resend_times = 5
        for i in range(max_resend_times):
            self.log(binascii.hexlify(a))
            self.send(a)
            data = ""
            #while len(data) != 9:
            data = self.recv(9)
            if 9 != len(data):
                continue
            self.log(binascii.hexlify(data))
            a_r = struct.unpack("B" * 9, data)
            #crc = self.crc16(a_r[0:len(data)-2])
            if crc[0] == a_r[len(data)-2] and crc[1] == a_r[len(data)-1]:
                print "pack index ", packIndex
                self.log("packIndex: " + str(packIndex))
                rc = True
                break
            else:
                self.log(binascii.hexlify(data))
                time.sleep(1)
                continue
                
        if not rc:
            return False
        
        rc = False
        imageFile = open(self.imagePath, "rb")
        
        packIndex = 1
        while packIndex <= totalPackNum:
            imageFile.seek(packSize * (packIndex-1))
            validData = imageFile.read(packSize)
            if not validData:
                print "error"
                self.log("error")
                break
            if len(validData) < packSize:
                lessPackSize = packSize - len(validData)
                lessData = [0xFF for x in range(lessPackSize)]
                validData += struct.pack("B" * lessPackSize, *lessData)
            a = struct.pack("BB", 0xAA, 0x55)                             #头部
            a += struct.pack("B", 0)                                      #加密字节
            a += struct.pack(">I", packIndex)                             #包ID
            a += validData
            crc = self.crc16(struct.unpack("B" * len(a), a))
            a += struct.pack("BB", *crc)
            #while len(data) != 9:
            data = ""
            for i in range(max_resend_times):
                self.log(binascii.hexlify(a))
                self.send(a)
                data = self.recv(9)
                if 9 == len(data):
                    break
            if 9 != len(data):
                return False
            a_r = struct.unpack("B" * 9, data)
            #crc = self.crc16(a[0:len(data)-2])
            if crc[0] == a_r[len(data)-2] and crc[1] == a_r[len(data)-1]:
                print "pack index ", packIndex
                self.log("packIndex: " + str(packIndex))
                self.log(binascii.hexlify(data))
                packIndex += 1
        if packIndex == totalPackNum + 1:
            print "upgrade ok"
            self.log("upgrade ok")
            rc = True
            rc = False
        return rc
        
    def run(self):
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
                s.settimeout(5)
            except socket.error, msg:
                s = None
                continue
            try:
                s.connect(sa)
            except socket.error, msg:
                s.close()
                s = None
                print "connect error"
                continue
            break
        if None == s:
            return
        self.sock = s
        rc = self.upgradeHandler()
        self.sock.close()
        return rc
            
class PortClient(TcpClient):
    def __init__(self, name="COM4", baud=9600, imagePath="WZH8003A-BIN-Rev4.2-Rev6029.bin", log=lambda x:x):
        TcpClient.__init__(self, imagePath=imagePath, log=log)
        self.name = name
        self.baud = baud
        self.serial = None
        self.power = True
        self.running = False
        
    def close(self):
        self.power = False
        self.serial.close()
        
    def recv(self, r_len):
        return self.serial.read(r_len)
        
    def send(self, data):
        time.sleep(0.1)
        return self.serial.write(data)
        
    def request(self, slave, start, length):
        data = [slave, 0x03, start >> 8, start & 0xff, length >> 8, length & 0xff]
        data += reversed(self.crc16(data))
        str0 = struct.pack("B" * len(data), *data)
        return str0

    def response(self, str0):
        str0 = str0[3:len(str0)-2]
        str0 = struct.unpack(">" + "H" * (len(str0)/2), str0)
        return str0
        
    def vercomp(self):
        r = re.compile(".*8003.*-BIN-Rev\d*\.\d*-Rev([0-9]*).*\.bin")
        r2 = re.compile(".*625.*-BIN-Rev\d*\.\d*-Rev([0-9]*).*\.bin")
        #r = re.compile(".*-BIN-Rev\d*\.\d*-Rev([0-9]*).*\.bin")
        m = r.search(self.imagePath)
        m2 = r2.search(self.imagePath)
        if m:
            self.log("image soft rev : " + m.group(1))
            req = self.request(1, 40003 - 40001, 1)
            self.send(req)
            rep = self.recv(3 + 2 + 2)
            sRev, = self.response(rep)
            self.log("soft rev : " + str(sRev))
            if sRev == int(m.group(1)):
                return True
        elif m2:
            self.log("image soft rev : " + m2.group(1))
            req = self.request(1, 40002 - 40001, 1)
            self.send(req)
            rep = self.recv(3 + 2 + 2)
            sRev, = self.response(rep)
            self.log("soft rev : " + str(sRev))
            if sRev == int(m2.group(1)):
                return True
        else:
            self.log("image error")
            return True
        return False
    
    def run(self):
        self.serial = serial.Serial(self.name, self.baud)
        self.serial.timeout = 1
        self.running = True
        self.log(True)
        rc = False
        while not rc and self.power:
            try:
                rc = self.upgradeHandler()
            except:
                pass
        self.serial.close()
        self.log(False)
        self.running = False
    