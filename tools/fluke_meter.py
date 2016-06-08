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
import json
from serial.tools import list_ports
import re

reload(sys)
sys.setdefaultencoding('utf8')

class R():
    serial = None
    
    name = "COM1"
    
    baud = 9600
    
    frameSize = (700, 500)
    
    logFilePath = "log"
    
    logFile = None
    
    server = None
    
    app = None
    
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
        
    @staticmethod
    def serialWrite(data):
        rc = R.serial.write(data)
        R.log("send msg len ", rc, data)

class Meter():
    rCmd0 = re.compile("[=|\?|!]>\r\n")
    rOk0 = re.compile("=>\r\n")
    rErr0 = re.compile("\?>\r\n")
    rErr1 = re.compile("!>\r\n")
    
    action1 = [
        {"name":"a1", "cmd":"*CLS", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a2", "cmd":"AAC", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a3", "cmd":"AACDC", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a4", "cmd":"ADC", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a5", "cmd":"CONT", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a6", "cmd":"DIODE", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a7", "cmd":"FREQ", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a8", "cmd":"FUNC1?", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a9", "cmd":"OHMS", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a10", "cmd":"WIRE2", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a11", "cmd":"VAC", "value":None},
        {"name":"a2", "cmd":"VAL?", "value":None},
        {"name":"a2", "cmd":"*RST", "value":None},
    ]
        

class MeterRcvThread(threading.Thread):
    def __init__(self, name="COM5", baud=9600):
        threading.Thread.__init__(self)
        self.serial = serial.Serial(name, baud)
        self.serial.timeout = 1
        R.serial = self.serial
        
    def crc16(self, *x):
        b = 0xA001
        a = 0xFFFF
        for byte in x:
            a = a^byte
            for i in range(8):
                last = a%2
                a = a>>1
                if last ==1: a = a^b
        aa = '0'*(6-len(hex(a)))+hex(a)[2:]
        hh, ll = int(aa[:2],16),int(aa[2:],16)
        return [ll, hh]
        
    def msg(self, *msg):
        rc = -1
        if "upgrade" == msg[0]:
            pass
        return rc
    def rcvHandler(self):
        b0 = ""
        while True:
            a0 = self.serial.read(1)
            if 1 != len(a0):
                return False
            b0 += a0
            if Meter.rCmd0.search(b0):
                break
        R.log(b0)
                    
    def run(self):
        #while R.serial:
        #    R.serialWrite("*CLS\r\n")
        #    self.rcvHandler()
        #    time.sleep(1)
        #R.log("select")
        for v in Meter.action1:
            R.log(v["name"])
            R.serialWrite(v["cmd"] + "\r\n")
            self.rcvHandler()
            time.sleep(5)
        R.log("Over")
#---------------------------------------------------------------------------
class MainPanel(wx.Panel):
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)
        R.app = self
        y = R.frameSize[1] - 70
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
        self.logCheckBox = wx.CheckBox(self, -1, u"日志", pos=(x, y + 5), size=(60, -1), style=wx.NO_BORDER)
        
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
        self.Bind(wx.EVT_BUTTON, self.startServerBtnHandler, self.startServerBtn)
        self.Bind(wx.EVT_CHECKBOX, self.logCheckBoxHandler, self.logCheckBox)
        
        ##------------------------------------------------------------------------------
        ##添加可选项
        ##------------------------------------------------------------------------------
        
    def msg(self, *msg):
        rc = -1
        if "change" == msg[0]:
            pass
        return rc
                
    def comScanHandler(self, event):
        name = self.nameListBox.GetValue()
        self.nameListBox.Set([str(port) for port, desc, hwid in sorted(list_ports.comports())])
        self.nameListBox.SetValue(name)
        
    def startServerBtnHandler(self, event):
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
            else:
                cb.SetValue(False)
            dlg.Destroy()
        else:
            R.logFile.close()
            R.logFile = None
    
class MainFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, u'FLUKE万用表', 
                size=R.frameSize)
        self.Center()
        self.Bind(wx.EVT_CLOSE, self.close, self)
        win = MainPanel(self, R)
        
        
    def close(self, event):
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