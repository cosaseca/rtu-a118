#!/usr/bin/python
# -*- coding:utf-8 -*-

import threading
import socket
import struct
import binascii
import time
from Tkinter import *
import tkFont

class TcpClient(threading.Thread):
    def __init__(self, ip="::", port="10086", app=None, slave=1, addr=0, rlen=100, interval=5, log_file=None, heart_beat=True):
        threading.Thread.__init__(self)
        self.ip = str(ip)
        self.port = str(port)
        self.app = app
        self.slave = slave
        self.addr = addr
        self.rlen = rlen
        self.interval = interval
        self.log_file = log_file
        self.heart_beat = heart_beat
        print "ip:", self.ip, "port:", self.port

    def request(self, slave, start, length):
        str0 = struct.pack("12B", 0x00, 0x04,
                          0x00, 0x00, 0x00,
                          0x06, slave, 0x03,
                          start >> 8, start & 0xff, length >> 8, length & 0xff)
        return str0

    def response(self, str0):
        str0 = str0[9:]
        #print len(str0)
        str0 = struct.unpack(">" + "H" * (len(str0)/2), str0)
        return str0
    
    def run(self):
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
                #s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDTIMEO, 1)
                s.settimeout(2)
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
        #s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, 1)
        tm0 = time.time()
        heart_beat_data_rcv = [0xD7, 0x02, 0xD0, 0x6A]
        heart_beat_data_snd = [0xD7, 0x03, 0x12]
        if self.heart_beat:
            while 1:
                try:
                    rdata = s.recv(1024)
                    if rdata == struct.pack("B" * len(heart_beat_data_rcv), *heart_beat_data_rcv):
                        s.send(struct.pack("B" * len(heart_beat_data_snd), *heart_beat_data_snd))
                        break
                except:
                    return
        while 1:
            try:
                s.send(self.request(self.slave, self.addr - 40001, self.rlen))
                rdata = s.recv(1024)
                rdata = self.response(rdata)
                str0 = time.ctime() + "\n"
                j = 0
                for i in rdata:
                    str0 += (str(j + self.addr) + ": " + hex(i)[2:]).ljust(15)
                    if 6 == j % 7:
                        str0 += "\n"
                    j += 1
                str0 += "\n"
                if(time.time() - tm0 > 2 * 60):
                    tm0 = time.time()
                    str1 = struct.pack("12B", 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x06, 0x24, 0x64, 0x00, 0x00)
                    s.send(str1)
                    rdata = s.recv(1024)
                    str0 += "zero write 49317" + "\n"
                if self.log_file:
                    self.log_file.write(str0)
                    self.log_file.flush()
                print str0
                if self.app:
                    try:
                        item = self.app.canvas.find_withtag("info")
                        self.app.canvas.itemconfig(item, text = str0)
                    except:
                        break
                time.sleep(self.interval)
                    
            except:
                break

class TcpClientWorker(threading.Thread):
    def __init__(self, ip, port, slave, addr, rlen, interval, app):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.slave = slave
        self.addr = addr
        self.rlen = rlen
        self.interval = interval
        self.app = app
    def run(self):
        a0_name = time.ctime().replace(" ", "-").replace(":", "_")
        print a0_name
        a0 = open("log_" + a0_name + ".txt", "w")
        while 1:
            self.client = TcpClient(ip=self.ip, port=self.port, app=self.app,
                                    slave=self.slave, addr=self.addr, rlen=self.rlen, interval=self.interval, log_file=a0)
            self.client.run()
            a0.write(time.ctime() + ": " + "error\n")
            a0.flush()
            print time.ctime() + ": " + "error"
        a0.close()

class App(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        #root = Tk()
        #ft = tkFont.Font(size=12)
        #self.canvas = Canvas(root, width = 900, height = 900, bg = 'white')
        #self.canvas.create_text(5, 5, text = 'Use Canvas', fill = 'black', tag = "info", anchor='nw', font=ft)
        #self.canvas.pack()
        # 192.168.8.26 502 1 40001 20 3
        while 1:
            ip = raw_input("please input rtu ip port slave addr len interval: ")
            try:
                ip, port, slave, addr, rlen, interval = ip.split(" ")
                addr = int(addr)
                rlen = int(rlen)
                slave = int(slave)
                interval = int(interval)
                break
            except:
                continue
        worker = TcpClientWorker(ip, port, slave, addr, rlen, interval, None)
        worker.run()
        #ipLabel = Label(root, width=4, height=1, text="地址")
        #ipLabel.place(x = 0, y = 0)
        #self.ipText = Text(root, width = 32, height = 1)
        #self.ipText.place(x = 30, y = 0)
        #portLabel = Label(root, width=4, height=1, text="端口")
        #portLabel.place(x = 0, y = 0)
        #self.ipText = Text(root, width = 32, height = 1)
        #self.ipText.place(x = 30, y = 0)
        #root.mainloop()

if __name__ == "__main__":
    app = App()
    app.run()
