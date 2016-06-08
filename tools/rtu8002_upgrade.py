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

reload(sys)
sys.setdefaultencoding('utf8')

class R():
    
    frameSize = (700, 500)
    
    clients = {}
    
    imagePath = "WZH8002-STM32_GPRS.bin"
    
    logFilePath = "log"
    
    logFile = None
    
    server = None
    
    app = None
    
    autoUpgrade = False
    
    timeOut = 10
    
    maxClientsNum = 5
    
    type = "udp"
    
    port = "10086"
    
    @staticmethod
    def log(*msg):
        a0 = (time.ctime(),) + msg
        print a0
        if R.logFile:
            R.logFile.write(str(a0) + "\r\n")
            
    @staticmethod
    def msgBox(*msg):
        if "err" == msg[0]:
            wx.MessageBox(msg[1], msg[0], wx.OK|wx.ICON_ERROR)
            
    @staticmethod
    def WriteText(*msg):
        print msg
        
    @staticmethod
    def write(*msg):
        print msg
        
class Image():
    @staticmethod
    def sndSysData(client, *cmd):
        data = ""
        if len(cmd) == 9:
            head0, len0, sum0, flowno0, id0, dev_type0, sub_dev_type0, cmd0, data = cmd
        elif len(cmd) == 8:
            head0, len0, sum0, flowno0, id0, dev_type0, sub_dev_type0, cmd0 = cmd
        else:
            return
        R.log("####sndSysData ", flowno0, id0, dev_type0, sub_dev_type0, cmd0)
        head_tail = [flowno0, id0, dev_type0, sub_dev_type0, cmd0]
        pack_head_tail = struct.pack("<BHBBB", *head_tail) + data
        
        data_sum = 0
        
        for i in struct.unpack("B" * len(pack_head_tail), pack_head_tail):
            data_sum += i
        data_sum = (0x55 - (data_sum & 0xFF)) & 0xFF
        
        head = [0x7E, len(pack_head_tail) + 1, data_sum]          #头部
        pack_head = struct.pack("<BHB", *head)                    #打包头部
        data = pack_head                                          #连接头部
        data += pack_head_tail
        R.log("sendto ", client["addr"], head, head_tail[0:5])
        if "udp" == R.type:
            client["sock"].sendto(data, client["addr"])
        else:
            s_len = 0
            while s_len != len(data):
                s_len += client["sock"].send(data[s_len:len(data)])
        return True
    @staticmethod
    def sndCommonData(client, *cmd):
        head0, len0, sum0, flowno0, id0, dev_type0, sub_dev_type0, cmd0 = cmd
        R.log(cmd)
        if (0x50) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD0, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x51) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD1, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x52) == cmd0:
            #head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD2, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD2, 0xFFFFFFFF]
            pack_head_tail = struct.pack("<BHBBBI", *head_tail)
        elif (0x53) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD3, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x54) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD4, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x55) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD5, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x56) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD6, 0xFFFFFFFF]
            head_tail_info = [0xFF for i in range(15)] + [0xFF for i in range(80)] + [0xFF for i in range(56)] + [0xFF for i in range(10)]
            head_tail += head_tail_info
            pack_head_tail = struct.pack("<BHBBBI" + "B" * len(head_tail_info), *head_tail)
        elif (0x57) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD7, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x58) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD8, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x59) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xD9, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x60) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE0, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x61) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE1, 0xFFFFFFFF]
            head_tail_info = [0xFF for i in range(15)] + [0xFF for i in range(80)] + [0xFF for i in range(56)] + [0xFF for i in range(10)]
            head_tail += head_tail_info
            pack_head_tail = struct.pack("<BHBBBI" + "B" * len(head_tail_info), *head_tail)
        elif (0x62) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE2, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x63) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE3, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x64) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE4, 0xFFFFFFFF]
            head_tail_info = [0xFF for i in range(20)] + [0xFF for i in range(20)] + [0xFF for i in range(20)] + [0xFF for i in range(20)]
            head_tail += head_tail_info
            pack_head_tail = struct.pack("<BHBBBI" + "B" * len(head_tail_info), *head_tail)
        elif (0x65) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE5, 0xFFFFFFFF]
            head_tail_info = [0xFF for i in range(20)]
            head_tail += head_tail_info
            pack_head_tail = struct.pack("<BHBBBI" + "B" * len(head_tail_info), *head_tail)
        elif (0x66) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE6, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x67) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE7, 0xFFFFFFFF]
            head_tail_info = [0xFF for i in range(8)]
            head_tail += head_tail_info
            pack_head_tail = struct.pack("<BHBBBI" + "B" * len(head_tail_info), *head_tail)
        elif (0x68) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE8, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        elif (0x69) == cmd0:
            head_tail = [flowno0, id0, dev_type0, sub_dev_type0, 0xE9, 0xFFFFFFFF, 0xFFFF, 0xFFFF]
            pack_head_tail = struct.pack("<BHBBBIHH", *head_tail)
        else:
            return False
        data_sum = 0
        
        for i in struct.unpack("B" * len(pack_head_tail), pack_head_tail):
            data_sum += i
        data_sum = (0x55 - (data_sum & 0xFF)) & 0xFF
        
        head = [0x7E, len(pack_head_tail) + 1, data_sum]          #头部
        pack_head = struct.pack("<BHB", *head)                    #打包头部
        data = pack_head                                          #连接头部
        data += pack_head_tail
        R.log("sendto ", client["addr"], head, head_tail[0:5])
        if "udp" == R.type:
            client["sock"].sendto(data, client["addr"])
        else:
            s_len = 0
            while s_len != len(data):
                s_len += client["sock"].send(data[s_len:len(data)])
        return True
        
    @staticmethod
    def sndImageData(client, *cmd):
        if cmd[0]:
            R.log(cmd)
            head0, len0, sum0, flowno0, id0, dev_type0, sub_dev_type0, cmd0, flag0 = cmd
        else:
            flowno0, id0, dev_type0, sub_dev_type0, cmd0, flag0 = (0, client["id"], client["dev_type"], client["sub_dev_type"], 0, 6)
            R.log("###########", flowno0, id0, dev_type0, sub_dev_type0, cmd0, flag0)
        ##------------------------------##
        rc_sum_sum = False
        if flowno0 == client["flow_no"] and client["image_len"] != 0 and (1 == flag0 or 2 == flag0 or 0 == flag0):
            if client["current_len"] >= client["image_len"]:
                client["current_len"] = client["image_len"]
                client["image_len"] = 0
            else:
                client["current_len"] += 512
                rc_sum_sum = True
            client["flow_no"] += 1
            if client["flow_no"] >= 0xF0:
                client["flow_no"] = 1
            image_len = client["current_len"] + 1 if client["image_len"] == 0 else client["image_len"] + 1
            R.app.msg("change", client["labelKey"], client["labelKey"] + 1, time.ctime(), hex(id0), "upgrade", int(client["current_len"] * 100/image_len), "%", client["status"], client["ver"])
        elif flowno0 == client["flow_no"] and flag0 == 0x03:
            client["imageFile"].close()
            client["upgrade"] = False
            client["imageFile"] = None
            R.log("upgrade ok")
            image_len = client["current_len"] if client["image_len"] == 0 else client["image_len"]
            client["status"] = "upgrade ok"
            R.app.msg("change", client["labelKey"], client["labelKey"] + 1, time.ctime(), hex(id0), "upgrade", int(client["current_len"] * 100/image_len), "%", client["status"], client["ver"])
            return True
        
        if 0 == client["current_len"]:
            image_data = [0x00 for x in range(512)]
        elif 0 == client["image_len"]:
            sum_sum = struct.pack("<HH", 0xFFFF & client["sum_sum"][0], 0xFFFF & client["sum_sum"][1])
            sum_sum = list(struct.unpack("BBBB", sum_sum))
            image_data = sum_sum + [0x00 for x in range(512 - len(sum_sum))]
        else:
            client["imageFile"].seek(client["current_len"] - 512)
            image_data = client["imageFile"].read(512)
            image_data = list(struct.unpack("B" * len(image_data), image_data))
            image_data += [0x00 for x in range(512 - len(image_data))]
            if rc_sum_sum:
                for x in image_data:
                    client["sum_sum"][0] += x
                for i in range(0,len(image_data),2):
                    client["sum_sum"][1] ^= ((image_data[i]) | (image_data[i+1]<<8)) & 0xFFFF
            
            encrypt = image_data[0]^0x55
            image_data = [image_data[0]] + [(x^encrypt) for x in image_data[1:]]
            
        ##------------------------------------------------------##
        pack_data = struct.pack("<I", client["image_len"])     #镜像大小
        pack_data += struct.pack("<I", client["current_len"])  #当前长度
        pack_data += struct.pack("B" * len(image_data) , *image_data)   #打包镜像数据
        data_sum = 0
        for i in image_data:
            data_sum += i
        data_sum = data_sum
        
        pack_data += struct.pack("<I", data_sum)                        #镜像数据校验和
        
        data_sum = 0
        
        head_tail = [client["flow_no"], id0, dev_type0, sub_dev_type0, 0x18, 0xFFFFFFFF]
        pack_head_tail = struct.pack("<BHBBBI", *head_tail)
        
        for i in struct.unpack("B" * len(pack_head_tail + pack_data), pack_head_tail + pack_data):
            data_sum += i
        data_sum = (0x55 - (data_sum & 0xFF)) & 0xFF
        
        head = [0x7E, 535, data_sum]                              #头部
        pack_head = struct.pack("<BHB", *head)                    #打包头部
        data = pack_head                                          #连接头部
        data += pack_head_tail
        data += pack_data                                         #连接数据
        R.log("sendto ", client["addr"], head, head_tail[0:5])
        client["time"] = time.time()
        if "udp" == R.type:
            client["sock"].sendto(data, client["addr"])
        else:
            s_len = 0
            while s_len != len(data):
                s_len += client["sock"].send(data[s_len:len(data)])
        return True
    @staticmethod
    def rcvData(req, sock, addr):
        a0 = struct.unpack("<BHBBHBBB", req[0:10])
        head0, len0, sum0, flowno0, id0, dev_type0, sub_dev_type0, cmd0 = a0
        if id0 not in R.clients.keys():
            R.clients[id0] = {
                "id": id0,
                "dev_type": dev_type0,
                "sub_dev_type": sub_dev_type0,
                "flow_no": 1,
                "image_len": 0,
                "current_len": 0,
                "addr": addr,
                "imageFile": None,
                "sum_sum": [0, 0],
                "upgrade": R.autoUpgrade,
                "sock": sock,
                "time": time.time(),
                "labelKey": -1,
                "ver": -1,
                "status": "",
            }
            client = R.clients[id0]
            Image.sndSysData(client, *(head0, len0, sum0, 1, id0, dev_type0, sub_dev_type0, 0x11, struct.pack("<BBBB", 0xFF, 0xFF, 0xFF, 0xFF)))
            if client["upgrade"]:
                client["image_len"] = os.path.getsize(R.imagePath)
                if client["image_len"]%512:
                    client["image_len"] -= client["image_len"]%512
                    client["image_len"] += 512
                client["imageFile"] = open(R.imagePath, "rb")
            client["labelKey"] = R.app.msg("add", time.ctime(), hex(id0), addr, "on", client["status"])
        client = R.clients[id0]
        if addr != client["addr"] or sock != client["sock"]:
            client["dev_type"] = dev_type0
            client["sub_dev_type"] = sub_dev_type0
            client["flow_no"] = 1
            client["image_len"] = 0
            client["current_len"] = 0
            client["addr"] = addr
            client["imageFile"] = None
            client["sum_sum"] = [0, 0]
            #client["upgrade"] = False
            client["sock"] = sock
            client["time"] = time.time()
            Image.sndSysData(client, *(head0, len0, sum0, 1, id0, dev_type0, sub_dev_type0, 0x11, struct.pack("<BBBB", 0xFF, 0xFF, 0xFF, 0xFF)))
            if client["upgrade"]:
                if client["imageFile"]:
                    client["imageFile"].close()
                client["image_len"] = os.path.getsize(R.imagePath)
                if client["image_len"]%512:
                    client["image_len"] -= client["image_len"]%512
                    client["image_len"] += 512
                client["imageFile"] = open(R.imagePath, "rb")
            R.app.msg("change", client["labelKey"], client["labelKey"] + 1, time.ctime(), hex(id0), addr, "on", client["status"], client["ver"])
        if (0x18|0x80) == cmd0:
            if client["upgrade"] and len(req) >= 11:
                flag0, = struct.unpack("<B", req[10])
                Image.sndImageData(client, *(a0 + (flag0,)))
        elif (0x11|0x80) == cmd0:
            if len(req) >= 11:
                ver = struct.unpack("<BBBB", req[10:14])
                R.log("firmware version ", ver)
                client["ver"] = ver
                R.app.msg("change", client["labelKey"], client["labelKey"] + 1, time.ctime(), hex(id0), addr, "on", client["status"], client["ver"])
            return False
        else:
            Image.sndCommonData(client, *a0)

class UdpServer(threading.Thread):
    def __init__(self, ip="::", port="10086"):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
        self.clients = {}
        self.inputs = []
        self.index = 0
        self.running = True
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_DGRAM):
            af, socktype, proto, canonname, sa = res
            try:
                server = socket.socket(af, socktype, proto)
                server.settimeout(2)
            except socket.error, msg:
                server = None
                continue
            server.bind(sa)
            break
        if None == server:
            return None
        self.inputs = [server]
        self.server = server
        
    def close(self):
        self.running = False
        self.server.close()
        
    def msg(self, *msg):
        rc = -1
        if "upgrade" == msg[0]:
            for key, value in R.clients.iteritems():
                if value["labelKey"] in msg[1] and value["upgrade"] != msg[2]:
                    client = value
                    if msg[2]:
                        client["image_len"] = os.path.getsize(R.imagePath)
                        if client["image_len"]%512:
                            client["image_len"] -= client["image_len"]%512
                            client["image_len"] += 512
                        client["imageFile"] = open(R.imagePath, "rb")
                        client["time"] = 0
                    else:
                        client["imageFile"].close()
                        client["imageFile"] = None
                    value["upgrade"] = msg[2]
        return rc
        
    def run(self):
        while self.running:
            readable, writable, exceptional = select.select(self.inputs, [], [], 5)
            if not readable:
                R.log("time out", self.port)
            elif self.server in readable:
                data, client = self.server.recvfrom(1024)
                R.log("recvfrom ", client)
                if len(data) < 10:
                    R.log("len(data) < 10 ")
                    continue
                Image.rcvData(data, self.server, client)
            for key in R.clients.keys():
                if R.clients[key]["upgrade"] and time.time() - R.clients[key]["time"] > 10:
                    Image.sndImageData(R.clients[key], None)
                    
class TcpServer(UdpServer):
    def __init__(self, ip="::", port="10086"):
        UdpServer.__init__(self, ip, port)
        self.inputs = []
        self.clients = {}
        self.running = True
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
            return None
        self.inputs = [server]
        self.server = server
        
    def close(self):
        self.running = False
        for i in self.inputs:
            i.close()
        self.clients = {}
        self.server.close()
    def run(self):
        while self.running:
            readable, writable, exceptional = select.select(self.inputs, [], [], 5)
            if not readable:
                R.log("time out", self.port)
            for s in readable:
                if s is self.server:
                    try:
                        connection, client_address = s.accept()
                        R.log("accept ", client_address)
                        #connection.setblocking(0)
                        self.inputs.append(connection)
                    except:
                        pass
                else:
                    data = None
                    try:
                        data = s.recv(1024)
                    except Exception, e:
                        pass
                    if not data:
                        R.log("disconnect from ")
                        for key, value in R.clients.iteritems():
                            if s == value["sock"]:
                                value["sock"] = -1
                                R.app.msg("change", value["labelKey"], value["labelKey"] + 1, time.ctime(), hex(value["id"]), value["addr"], "off", value["status"], value["ver"])
                        s.close()
                        self.inputs.remove(s)
                        continue
                    if len(data) < 10:
                        R.log("data error len(data) < 10", repr(data))
                        continue
                    Image.rcvData(data, s, "")
            for key in R.clients.keys():
                if R.clients[key]["upgrade"] and -1 != R.clients[key]["sock"] and time.time() - R.clients[key]["time"] > 10:
                    Image.sndImageData(R.clients[key], None)

#---------------------------------------------------------------------------

class TestListBox(wx.Panel):
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)
        R.app = self
        y = R.frameSize[1] - 70
        x = 5
        
        self.portBox = wx.TextCtrl(self,-1, R.port, pos=(x, y + 1),
                              size=(80,-1))
        x += 80 + 5
        self.startServerBtn = wx.Button(self,-1,u"运行", pos=(x, y),
                              size=(60,28),
                              style=wx.TE_MULTILINE)
        x += 60 + 5                      
        self.startUpgradeBtn = wx.Button(self,-1,u"升级", pos=(x, y),
                              size=(60,28),
                              style=wx.TE_MULTILINE)
        x += 60 + 5                       
        self.stopUpgradeBtn = wx.Button(self,-1,u"取消升级", pos=(x, y),
                              size=(80,28),
                              style=wx.TE_MULTILINE)
        x += 80 + 5 
        self.selectFileBtn = wx.Button(self,-1,u"选择镜像", pos=(x, y),
                              size=(80,28),
                              style=wx.TE_MULTILINE)
        x += 80 + 5 
        self.filePathBox = wx.TextCtrl(self,-1, R.imagePath, pos=(x, y + 1),
                              size=(100,-1))                     
        x += 100 + 5
        self.cb1 = wx.CheckBox(self, -1, u"自动升级", pos=(x, y + 5), size=(65, -1), style=wx.NO_BORDER)
        x += 65 + 5
        self.cb2 = wx.CheckBox(self, -1, u"日志", pos=(x, y + 5), size=(40, -1), style=wx.NO_BORDER)
        x += 40 + 5
        self.cb3 = wx.CheckBox(self, -1, u"tcp", pos=(x, y + 5), size=(40, -1), style=wx.NO_BORDER)

        self.lb2 = wx.ListBox(self, 100, pos=(5, 5), 
            size=(R.frameSize[0] - 30, R.frameSize[1] - 85), 
            choices=[], style=wx.LB_EXTENDED)
            
        #self.Bind(wx.EVT_LISTBOX, self.EvtMultiListBox, self.lb2)
        
        self.Bind(wx.EVT_BUTTON, self.startServerBtnHandler, self.startServerBtn)
        self.Bind(wx.EVT_BUTTON, self.startUpgradeBtnHandler, self.startUpgradeBtn)
        self.Bind(wx.EVT_BUTTON, self.stopUpgradeBtnHandler, self.stopUpgradeBtn)
        self.Bind(wx.EVT_BUTTON, self.selectFileHandler, self.selectFileBtn)
        self.Bind(wx.EVT_CHECKBOX, self.EvtCheckBox, self.cb1)
        self.Bind(wx.EVT_CHECKBOX, self.EvtCheckBox2, self.cb2)
        self.Bind(wx.EVT_CHECKBOX, self.EvtCheckBox3, self.cb3)
        
    def msg(self, *msg):
        rc = -1
        if "add" == msg[0]:
            rc = self.lb2.Append(str(msg[1:]))
        elif "del" == msg[0]:
            self.lb2.Delete(msg[1])
        elif "change" == msg[0]:
            self.lb2.SetString(msg[1], str(msg[2:]))
        return rc
        
    def startServerBtnHandler(self, event):
        if not os.path.exists(R.imagePath) or os.path.getsize(R.imagePath) < 40 * 1024:
            R.msgBox("err", u"镜像错误")
            return
        if not R.server:
            serverCreator = UdpServer
            port = str(self.portBox.GetValue())
            if "tcp" == R.type:
                serverCreator = TcpServer
            try:
                R.server = serverCreator(ip="0.0.0.0", port=port)
                R.server.start()
            except:
                return
        elif R.server:
            R.server.close()
            R.server = None
        if R.server:
            self.startServerBtn.SetLabel(u"停止")
        elif not R.server:
            self.startServerBtn.SetLabel(u"运行")
            
    def startUpgradeBtnHandler(self, event):
        R.log(self.lb2.GetSelections())
        R.server.msg("upgrade", self.lb2.GetSelections(), True)
        
    def stopUpgradeBtnHandler(self, event):
        R.log(self.lb2.GetSelections())
        R.server.msg("upgrade", self.lb2.GetSelections(), False)
        
    def selectFileHandler(self, event):
        file_wildcard = "All files(*.*)|*.*" 
        dlg = wx.FileDialog(self, "Open paint file...",
                            os.getcwd(), 
                            style = wx.OPEN | wx.CHANGE_DIR,
                            wildcard = file_wildcard)
        if dlg.ShowModal() == wx.ID_OK:
            R.imagePath = dlg.GetPath()
            self.filePathBox.SetValue(R.imagePath)
            R.log(R.imagePath)
        dlg.Destroy()
        
    def EvtCheckBox(self, event):
        self.log.write('EvtCheckBox: %d\n' % event.IsChecked())
        cb = event.GetEventObject()
        if event.IsChecked():
            self.log.write("\t3StateValue: %s\n" % cb.Get3StateValue())
            R.autoUpgrade = True
        else:
            R.autoUpgrade = False
            
    def EvtCheckBox2(self, event):
        self.log.write('EvtCheckBox: %d\n' % event.IsChecked())
        cb = event.GetEventObject()
        if event.IsChecked():
            self.log.write("\t3StateValue: %s\n" % cb.Get3StateValue())
            file_wildcard = "All files(*.*)|*.*" 
            
            dlg = wx.FileDialog(self, "Open paint file...",
                                os.getcwd(), 
                                style = wx.SAVE | wx.CHANGE_DIR ,
                                wildcard = file_wildcard,
                                defaultFile="log-" + time.ctime().replace(" ", "-").replace(":", "_") + ".txt")
            if dlg.ShowModal() == wx.ID_OK:
                R.logFilePath = dlg.GetPath()
                R.log(R.logFilePath)
                R.logFile = open(R.logFilePath, "w")
            dlg.Destroy()
            
        else:
            R.logFile.close()
            R.logFile = None
            
    def EvtCheckBox3(self, event):
        if event.IsChecked():
            R.type = "tcp"
        else:
            R.type = "udp"
    
class ZigbeeFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, u'8002升级', 
                size=R.frameSize)
        self.Center()
        self.Bind(wx.EVT_CLOSE, self.close, self)
        win = TestListBox(self, R)
        
        
    def close(self, event):
        if R.server:
            R.server.close()
        R.server = None
        R.logFile = None
        sys.exit(0)


#---------------------------------------------------------------------------
            
if __name__ == "__main__":
    app = wx.App()
    frame = ZigbeeFrame()
    frame.Show()
    app.MainLoop()
    