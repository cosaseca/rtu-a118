#!/usr/bin/python
# -*- coding: utf-8 -*-
import wx
import threading
import time
import serial
import struct        #结构类型，封装数据类型
import binascii      #二进制和ascii相互转换
import sys           #系统的具体参数和功能
import datetime
import os            #系统相关功能
import json          #一种数据格式
from serial.tools import list_ports #扫描串口

reload(sys)
sys.setdefaultencoding('utf8')

class R(): #全局变量
    serial = None  #串口
    
    name = "COM1"  #串口名称
    
    baud = 9600    #波特率
    
    frameSize = (700, 500) #绘制用户界面的大小
    
    meters = {}            #仪表
    
    imagePath = "Load5438A_A28303_C28H1A.bin"  #镜像路径
    
    logFilePath = "log"                        #日志文件路经
    
    logFile = None                             #日志文件
    
    server = None                              #服务
    
    app = None                                 #界面
    
    autoUpgrade = False                        #自动升级标志位
    
    timeOut = 10                               #超时10s
    
    maxClientsNum = 5                          #最大客户端数
    
    @staticmethod
    def log(*msg): #日志打印
        a0 = (time.ctime(),) + msg
        print a0
        if R.logFile:
            R.logFile.write(str(a0) + "\r\n")
            
    @staticmethod
    def msgBox(*msg): #弹出消息窗口
        if "err" == msg[0]:
            wx.MessageBox(msg[1], msg[0], wx.OK|wx.ICON_ERROR)
            
    @staticmethod
    def WriteText(*msg):
        print msg
        
    @staticmethod
    def write(*msg):
        print msg
        
    @staticmethod
    def serialWrite(data): #向串口发送数据
        rc = R.serial.write(data)
        R.log("send msg len ", rc, len(data))

class Meter(): #仪表类
    @staticmethod
    def sendData(m): #发送数据
        if m["upgrade"]:
            R.log(hex(m["currentPackNum"]), hex(m["totalPackNum"]))
        try:
            m["imageFile"].seek(m["currentPackNum"] * 60)
            data = m["imageFile"].read(60)
        except:
            return ""
        data = list(struct.unpack("B" * len(data), data))
        data = [0xAA, 0xBB, 0, m["totalPackNum"]>>8, m["totalPackNum"]&0xFF,
                m["currentPackNum"]>>8, m["currentPackNum"]&0xFF] + data
        mac = list(m["mac"])
        head = [0x7E, 0x00, 0x00, 0x11, 0x00] + mac#[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00] 
        head += [0xFF, 0xFE, 0xE8, 0xE8, 0x00, 0x11, 0x18, 0x57, 0x00, 0x00]
        data = head + data
        data_len = len(data)
        data[1] = ((data_len - 3) >> 8) & 0xFF
        data[2] = (data_len - 3) & 0xFF
        sum0 = 0
        for i in range(3, data_len): #计算校验和
            sum0 += data[i]
        sum0 = 0xFF - (sum0 & 0xFF)
        data += [sum0]
        a1 = struct.pack("B" * len(data), *data)
        R.log("snd " + binascii.hexlify(a1))
        return a1
        
        
    @staticmethod
    def rcvData(req): #接收数据
        mac = req[1:9]
        if 0xAA == req[21 - 3] and 0xBB == req[22 - 3]:
            rc = "T"
            if not R.meters[mac]["upgrade"]:
                return
            if mac not in R.meters.keys():
                return
            if 0 == req[27 - 3] or 3 == req[27 - 3]:
                R.meters[mac]["currentPackNum"] += 1
                if 3 == req[27 - 3]:
                    R.log("currentPackNum error")
                    rc = "F"
            elif 5 == req[27 - 3]:
                R.meters[mac]["currentPackNum"] = 0
                    
            if R.meters[mac]["currentPackNum"] >= R.meters[mac]["totalPackNum"]:
                #if 3 == req[27 - 3]:
                #    R.meters[mac]["currentPackNum"] = 0
                #    return
                R.log("upgrade ok")
                R.meters[mac]["upgrade"] = False
                if R.meters[mac]["imageFile"] != None:
                    R.meters[mac]["imageFile"].close()
                    R.meters[mac]["imageFile"] = None
                R.app.msg("change", R.meters[mac]["labelKey"], R.meters[mac]["labelKey"] + 1, time.ctime(), binascii.hexlify(struct.pack("B" * len(mac), *mac)), "upgrade", int(R.meters[mac]["currentPackNum"] * 100/R.meters[mac]["totalPackNum"]), "%", rc)
                return
            R.meters[mac]["time"] = time.time()
            R.serialWrite(Meter.sendData(R.meters[mac]))
            R.app.msg("change", R.meters[mac]["labelKey"], R.meters[mac]["labelKey"] + 1, time.ctime(), binascii.hexlify(struct.pack("B" * len(mac), *mac)), "upgrade", int(R.meters[mac]["currentPackNum"] * 100/R.meters[mac]["totalPackNum"]), "%", rc)
            return
        cmd = (req[29 - 3]<<8) | req[30 - 3]
        type = (req[25 - 3]<<8) | req[26 - 3]
        if 0x0001 != type:
            return
        if len(R.meters) >= R.maxClientsNum:
            return
        if mac not in R.meters.keys():
            totalPackNum = 1024 * 1024
            R.meters[mac] = {
                    "mac":mac,
                    "totalPackNum":totalPackNum,
                    "currentPackNum":0,
                    "upgrade":R.autoUpgrade,
                    "imageFile":None,
                    "labelKey": -1,
                    "time":time.time(),
                }
            R.meters[mac]["labelKey"] = R.app.msg("add", time.ctime(), binascii.hexlify(struct.pack("B" * len(mac), *mac)))
            if R.meters[mac]["upgrade"]:
                fileSize = os.path.getsize(R.imagePath)
                totalPackNum = fileSize/60
                if 0 != fileSize%60:
                    totalPackNum += 1
                R.meters[mac]["totalPackNum"] = totalPackNum
                R.meters[mac]["imageFile"] = open(R.imagePath, "rb")
                R.serialWrite(Meter.sendData(R.meters[mac]))
            #R.app.msg("del", R.meters[mac]["labelKey"], binascii.hexlify(struct.pack("B" * len(mac), *mac)))
            #R.serial.write(Meter.sendData(R.meters[mac]))
        #if R.meters[mac]["upgrade"] and 0 != R.meters[mac]["currentPackNum"]:
        #    R.meters[mac]["currentPackNum"] = 0
        elif R.meters[mac]["upgrade"]:
            R.serialWrite(Meter.sendData(R.meters[mac]))
        

class MeterRcvThread(threading.Thread): #仪表数据接收线程
    def __init__(self, name="COM5", baud=9600):
        threading.Thread.__init__(self)
        self.serial = serial.Serial(name, baud)
        self.serial.timeout = 0.2
        R.serial = self.serial
        
    def msg(self, *msg):
        rc = -1
        if "upgrade" == msg[0]:
            fileSize = os.path.getsize(R.imagePath)
            for key, value in R.meters.iteritems():
                if value["labelKey"] in msg[1] and value["upgrade"] != msg[2]:
                    if msg[2]:
                        totalPackNum = fileSize/60
                        if 0 != fileSize%60:
                            totalPackNum += 1
                        value["totalPackNum"] = totalPackNum
                        value["currentPackNum"] = 0
                        value["imageFile"] = open(R.imagePath, "rb")
                        #value["time"] = time.time()
                    elif value["imageFile"]:
                        value["imageFile"].close()
                        value["imageFile"] = None
                    value["upgrade"] = msg[2]
        return rc
        
    def rcvHandler(self): #接收句柄
        a0 = self.serial.read(1)
        if(len(a0) != 1):
            #R.log("len(a0) != 1")
            return False
        a0, = struct.unpack("B", a0)
        if 0x7E != a0:
            R.log("0x7E != a0", a0)
            return False
        a0 = self.serial.read(2)
        if(len(a0) != 2):
            R.log("len(a0) != 2")
            return False
        a0 = struct.unpack("B" * 2, a0)
        a0_len = ((a0[0]<<8) | a0[1]) + 1
        if a0_len < 20 or a0_len > 1024:
            R.log("a0_len < 27 or a0_len > 1024")
            return False
        a0 = self.serial.read(a0_len)
        if(len(a0) != a0_len):
            R.log("len(a0) != a0_len")
            return False
        b0 = struct.unpack(">" + "B" * len(a0), a0)
        R.log("rcv " + binascii.hexlify(a0))
        return Meter.rcvData(b0)
                    
    def run(self):
        while R.serial:
            self.rcvHandler()
            tm = time.time()
            for key, value in R.meters.iteritems():
                if value["upgrade"] and tm - value["time"] > R.timeOut:
                    value["time"] = time.time()
                    R.serialWrite(Meter.sendData(value))
            #R.log("select")
            
#---------------------------------------------------------------------------
class MainPanel(wx.Panel):  #主面板
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)
        R.app = self
        y = R.frameSize[1] - 70 #布局用的
        x = 5
        
        choices = [str(port) for port, desc, hwid in sorted(list_ports.comports())]
        
        comName = "" if len(choices) == 0 else choices[0]
        
        self.nameListBox = wx.ComboBox(self, -1, value = comName, 
            choices = choices, 
            pos=(x, y + 1),
            size = (80,-1),
            style = wx.CB_DROPDOWN)
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
        self.autoUpgradeCheckBox = wx.CheckBox(self, -1, u"自动升级", pos=(x, y + 5), size=(65, -1), style=wx.NO_BORDER)
        x += 65 + 5
        self.logCheckBox = wx.CheckBox(self, -1, u"日志", pos=(x, y + 5), size=(60, -1), style=wx.NO_BORDER)

        self.clientsListBox = wx.ListBox(self, 100, pos=(5, 5), 
            size=(R.frameSize[0] - 30, R.frameSize[1] - 85), 
            choices=[], style=wx.LB_EXTENDED)
            
        self.Bind(wx.EVT_LISTBOX, self.clientsListBoxHandler, self.clientsListBox)
        
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
        self.Bind(wx.EVT_BUTTON, self.startServerBtnHandler, self.startServerBtn)
        self.Bind(wx.EVT_BUTTON, self.startUpgradeBtnHandler, self.startUpgradeBtn)
        self.Bind(wx.EVT_BUTTON, self.stopUpgradeBtnHandler, self.stopUpgradeBtn)
        self.Bind(wx.EVT_BUTTON, self.selectFileHandler, self.selectFileBtn)
        self.Bind(wx.EVT_CHECKBOX, self.autoUpgradeCheckBoxHandler, self.autoUpgradeCheckBox)
        self.Bind(wx.EVT_CHECKBOX, self.logCheckBoxHandler, self.logCheckBox)
        
    def msg(self, *msg):
        rc = -1
        if "add" == msg[0]:
            rc = self.clientsListBox.Append(str(msg[1:]))
        elif "del" == msg[0]:
            self.clientsListBox.Delete(msg[1])
        elif "change" == msg[0]:
            self.clientsListBox.SetString(msg[1], str(msg[2:]))
        return rc

    def clientsListBoxHandler(self, event):
        self.log.WriteText('EvtMultiListBox: %s\n' % str(self.lb2.GetSelections()))
                
    def comScanHandler(self, event):
        name = self.nameListBox.GetValue()
        self.nameListBox.Set([str(port) for port, desc, hwid in sorted(list_ports.comports())])
        self.nameListBox.SetValue(name)
        
    def startServerBtnHandler(self, event):
        if not os.path.exists(R.imagePath) or os.path.getsize(R.imagePath) < 40 * 1024:
            R.msgBox("err", u"镜像错误")
            return
        if not R.serial:
            R.name = str(self.nameListBox.GetValue())
            try:
                R.server = MeterRcvThread(name=R.name, baud=R.baud)
                R.server.start()
            except:
                R.msgBox("err", u"串口打开失败")
                return
        elif R.serial:
            R.serial.close()
            R.serial = None
        if R.serial:
            self.startServerBtn.SetLabel(u"停止")
        elif not R.serial:
            self.startServerBtn.SetLabel(u"运行")
            
    def startUpgradeBtnHandler(self, event):
        R.log(self.nameListBox.GetSelections())
        R.server.msg("upgrade", self.nameListBox.GetSelections(), True)
        
    def stopUpgradeBtnHandler(self, event):
        R.log(self.nameListBox.GetSelections())
        R.server.msg("upgrade", self.nameListBox.GetSelections(), False)
        
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
        
    def autoUpgradeCheckBoxHandler(self, event):
        self.log.write('EvtCheckBox: %d\n' % event.IsChecked())
        cb = event.GetEventObject()
        if event.IsChecked():
            self.log.write("\t3StateValue: %s\n" % cb.Get3StateValue())
            R.autoUpgrade = True
        else:
            R.autoUpgrade = False
            
    def logCheckBoxHandler(self, event):
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
            
            #R.logFilePath = "log-" + time.ctime().replace(" ", "-").replace(":", "_") + ".txt"
            #R.logFile = open(R.logFilePath, "w")
        else:
            R.logFile.close()
            #os.remove(R.logFilePath)
            R.logFile = None
    
class MainFrame(wx.Frame): #主框架
    def __init__(self):
        wx.Frame.__init__(self, None, -1, u'载荷升级', 
                size=R.frameSize)
        self.Center()      #窗口居中
        self.Bind(wx.EVT_CLOSE, self.close, self)  #绑定关闭事件
        win = MainPanel(self, R)
        
        
    def close(self, event): #清理资源，退出程序
        if R.serial:
            R.serial.close()
        R.serial = None
        R.logFile = None
        sys.exit(0)


#---------------------------------------------------------------------------
            
if __name__ == "__main__":
    app = wx.App()
    frame = MainFrame()
    frame.Show()
    app.MainLoop()