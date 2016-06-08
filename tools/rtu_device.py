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
import select

reload(sys)
sys.setdefaultencoding('utf8')

class R():
    imageFile = None
    imageFilePath = "file_download.bin"

class TcpServer(threading.Thread):
    def __init__(self, ip="127.0.0.1", port="10086", imagePath=None, log=None):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.imagePath = imagePath
        self.log = log
        
    def run(self):
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                server = socket.socket(af, socktype, proto)
                server.settimeout(2)
            except socket.error, msg:
                server = None
                continue
            server.bind(sa)
            server.listen(10)
            break
        if None == server:
            return
        inputs = [server]
        while 1:
            readable, writable, exceptional = select.select(inputs, [], [], 3)
            if not readable:
                print "time out"
            for s in readable:
                if s is server:
                    connection, client_address = s.accept()
                    print "connection from ", client_address
                    #connection.setblocking(0)
                    inputs.append(connection)
                else:
                    data = None
                    try:
                        data = self.upgradeHandler(s)
                        #print repr(data)
                    except Exception, e:
                        print "disconnect from ", client_address, e
                        s.close()
                        #R.imageFile.close()
                        inputs.remove(s)

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
        return [ll, hh]
    
    def upgradeHandler(self, s):
        data = ""
        data += s.recv(1)
        a, = struct.unpack("B", data[0])
        if 0xAA != a:
            print "head1 error", a
            return data
        data += s.recv(1)
        a, = struct.unpack("B", data[1])
        if 0x55 != a:
            print "head2 error", a
            return data
        while len(data) != 34:
            data += s.recv(34 - len(data))
        a = struct.unpack("B" * len(data), data)
        ll, hh = self.crc16(a[0:len(data)-2])
        if ll == a[len(data)-2] and hh == a[len(data)-1]:
            print "pack index ", ((a[2]<<24) | (a[3]<<16) | (a[4]<<8) | a[5])
            #R.imageFile = open(R.imageFilePath, "wb")
            a = [0xAA, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00]
            crc = [ll, hh]#self.crc16(a)
            a += crc
            sdata = ""
            for i in a:
                sdata += struct.pack("B", i)
            s.send(sdata)
            return data
        else:
            print binascii.hexlify(data), ll, hh
        
        while len(data) != 137:
            data += s.recv(137 - len(data))
            
        a = struct.unpack("B" * len(data), data)
        ll, hh = self.crc16(a[0:len(data)-2])
        if ll == a[len(data)-2] and hh == a[len(data)-1]:
            print "pack index ", ((a[3]<<24) | (a[4]<<16) | (a[5]<<8) | a[6])
            #R.imageFile.write(data[7:7+128])
            a = [0xAA, 0x55, a[2], a[3], a[4], a[5], 0x00]
            crc = [ll, hh]#self.crc16(a)
            a += crc
            sdata = ""
            for i in a:
                sdata += struct.pack("B", i)
            s.send(sdata)
            return data
        else:
            print "data error"
        return "data error"
if __name__ == "__main__":
    ip = raw_input("ip : ")
    port = raw_input("port : ")
    server = TcpServer(ip=ip, port=port)
    server.run()
    