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
import upgrade
from serial.tools import list_ports
                        
class R():
    serial = None
    
    name = "COM5"
    
    baud = 9600
    
    logFile = None
    
    logBox = None
    
    logFilePath = "log.txt"
    
    configPath = "config.txt"
    
    @staticmethod
    def log(msg):
        if R.logFile:
            R.logFile.write(time.ctime() + " " + msg + "\r\n")
        #print time.ctime() + " " + msg
        if R.logBox:
            pass
            R.logBox.AppendText(time.ctime() + " " + msg + "\r\n")
            
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
        R.log("send msg len ", rc, len(data))
        
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
        
        x = 370
        y = 3
        self.logCheckBox = wx.CheckBox(self, -1, u"日志", pos=(x, y + 5), size=(60, -1), style=wx.NO_BORDER)
        self.Bind(wx.EVT_CHECKBOX, self.logCheckBoxHandler, self.logCheckBox)
        
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
        
    def logCheckBoxHandler(self, event):
        #self.log.write('EvtCheckBox: %d\n' % event.IsChecked())
        cb = event.GetEventObject()
        if event.IsChecked():
            #self.log.write("\t3StateValue: %s\n" % cb.Get3StateValue())
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

class ZigbeeFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, 'rtu8003_upgrade', 
                size=(630, 465))
        #self.SetIcon(wx.Icon(name='zigbee_test.ico', type=wx.BITMAP_TYPE_ICO))
        self.Center()
        self.Bind(wx.EVT_CLOSE, self.close, self)
        ImageUpgradePanel(self)
        
    def close(self, event):
        if R.serial:
            R.serial.close()
        R.serial = None
        R.logBox = None
        R.logFile = None
        os._exit(0)
        
if __name__ == '__main__':
    app = wx.App()
    frame = ZigbeeFrame()
    frame.Show()
    app.MainLoop()
