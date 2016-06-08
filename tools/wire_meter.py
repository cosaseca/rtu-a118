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
import zigbee_data

class R():
    serial = None
    
    name = "COM1"
    
    baud = 9600
    
    frameSize = (900, 500)
    
    logFilePath = "log"
    
    configPath = "wire_meter_config"
    
    logFile = None
    
    server = None
    
    panel = None
    
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
        R.log("send msg len ", rc, len(data))

class Meter():
    elecStartTime = time.time()
    elecIndex = 0
    
    data = {
        "actuator": [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x84, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x01, 0x02, 0x2E, 0x21, 0x3D, 0x01, 0x6B, 0x00, 0x1A, 0x01, 0x01, 0x1A, 0x00, 0x00, 0x9D, 0xF4, 0x00],
        "elec_param": [0xFFFF for x in range(120)],
    }
        

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
        
    def rcvActuatorHandler(self, head):
        a0 = self.serial.read(12)
        a1 = ""
        if 12 != len(a0):
            return False
        b0 = list(struct.unpack("B" * len(a0), a0))
        if 0x01 == b0[1] or 0xEE != b0[-1]:
            a1 = self.serial.read(3)
            if 3 != len(a1):
                return False
            b0 += list(struct.unpack("B" * len(a1), a1))
        b0_sum = head
        for i in b0[:-3]:
            b0_sum += i
        b0_sum &= 0xFFFF
        R.log(hex(b0_sum), hex((b0[-2]) | b0[-3]<<8))
        if b0_sum != ((b0[-2]) | (b0[-3]<<8)):
            R.log("rcv Actuator error", binascii.hexlify(a0 + a1))
            R.log("b0_sum error")
            return False
        if 0x01 == b0[1]:
            data = Meter.data["actuator"]
            data_end = [0xEE, 0xEE]
            value = b0[2]<<24 | b0[3]<<16 | b0[4]<<8 | b0[5]
            R.panel.valueBox17.SetValue(str(value))
        elif 0x02 == b0[1]:
            data = [0x01]
            data_end = [0xEE, 0xEE]
            value = b0[2]<<24 | b0[3]<<16 | b0[4]<<8 | b0[5]
            value /= 1000
            R.panel.valueBox1.SetValue(str(value))
            value = b0[6]
            R.panel.valueBox2.SetValue(str(value))
            value = b0[7]
            R.panel.valueBox3.SetValue(str(value))
            value = b0[8]
            R.panel.valueBox4.SetValue(str(value))
        else:
            return False
        data_len = len(data)
        data = [0xCC, b0[0], b0[1], (data_len&0xFF00)>>8, data_len&0xFF] + data
        data_sum = 0
        for i in data:
            data_sum += i
        data += [(data_sum&0xFF00)>>8, data_sum&0xFF] + data_end
        self.serial.write(struct.pack("B" * len(data), *data))
        R.log("rcv Actuator ", binascii.hexlify(a0 + a1))
        R.log("snd Actuator ", "".join(["0" * (2 - len(hex(x)[2:])) + hex(x)[2:] for x in data]))
            
    def readHoldingRegisters(self, slave):
        a0 = self.serial.read(6) #读取第6个字节数据，数据
        if(len(a0) != 6):
            R.log("len(a0) != 6")
            return False
        b0 = struct.unpack("B" * len(a0), a0)
        b0_sum = self.crc16(slave, 0x03, *(b0[:-2]))
        if b0[-2] != b0_sum[0] or b0[-1] != b0_sum[1]:
            R.log("rcv ElectricalParam b0_sum error")
            return False
        register_start = b0[0]<<8 | b0[1]
        register_len = b0[2]<<8 | b0[3]
        if register_start < 0 or register_len <= 0 or register_len + register_start > len(Meter.data["elec_param"]):
            R.log(register_start, register_len, len(Meter.data["elec_param"]))
            return False
        if 67 == register_start and 4 == register_len:
            if Meter.elecIndex >= 200:
                Meter.data["elec_param"][67] = 0xFFFF
                Meter.data["elec_param"][70] = 0xFFFF
            else:
                Meter.data["elec_param"][67] = zigbee_data.current[2*Meter.elecIndex]<<8 | zigbee_data.current[2*Meter.elecIndex + 1]
                Meter.data["elec_param"][70] = zigbee_data.power[2*Meter.elecIndex]<<8 | zigbee_data.power[2*Meter.elecIndex + 1]
            Meter.elecIndex += 1
        data0 = struct.pack(">" + "H"*register_len, *(Meter.data["elec_param"][register_start:register_start + register_len]))
        data0 = struct.unpack("B" * register_len * 2, data0)
        data = [slave, 0x03, (register_len<<1)&0xFF] + list(data0)
        data_sum = self.crc16(*data)
        data += data_sum
        #self.serial.write(data)
        self.serial.write(struct.pack("B" * len(data), *data))
        
    def writeSingleRegister(self, slave):
        a0 = self.serial.read(6) #读取第6个字节数据，数据
        if(len(a0) != 6):
            R.log("len(a0) != 6")
            return False
        b0 = struct.unpack("B" * len(a0), a0)
        b0_sum = self.crc16(slave, 0x06, *(b0[:-2]))
        if b0[-2] != b0_sum[0] or b0[-1] != b0_sum[1]:
            R.log("rcv ElectricalParam b0_sum error")
            return False
        register_no = b0[0]<<8 | b0[1]
        register_val = b0[2]<<8 | b0[3]
        if register_no < 0 or register_no > len(Meter.data["elec_param"]):
            R.log(register_no, len(Meter.data["elec_param"]))
            return False
        if 29 == register_no and 1 == register_val:
            config = json.dumps(Meter.data)
            file = open(R.configPath, "w")
            file.write(config)
            file.close()
            register_val = 0
        Meter.data["elec_param"][register_no] = register_val
        data = [slave, 0x06] + list(b0)
        #self.serial.write(data)
        self.serial.write(struct.pack("B" * len(data), *data))
        
    def writeRegisters(self, slave):
        fun_no = 0x10
        a0 = self.serial.read(5) #读取第6个字节数据，数据
        if(len(a0) != 5):
            R.log("len(a0) != 5")
            return a0
        r_start, r_len, b_len = struct.unpack(">HHB", a0) #">"网络字节序，H 16位的无符号整数，B 8位的无符号整数
        
        a0 = self.serial.read(b_len + 2)
        if(len(a0) != b_len + 2):
            R.log("len(a0) != b_len + 2")
            return a0
        a1 = struct.unpack("B" * b_len, a0[0:b_len])
        crc_l, crc_h = struct.unpack("B" * 2, a0[b_len:b_len+2])
        crc16_sum = self.crc16(slave, fun_no, (r_start>>8)&0xFF, r_start&0xFF, (r_len>>8)&0xFF, r_len&0xFF, b_len, *a1) #crc16校验值
        #R.log(crc16_sum, crc_l, crc_h)
        if crc16_sum[0] != crc_l or crc16_sum[1] != crc_h: #判断校验是否正确
            R.log("crc16_sum[0] != crc_l or crc16_sum[1] != crc_h")
            return a0
        r_values = list(struct.unpack(">" + "H" * r_len, a0[0:b_len]))
        Meter.data["elec_param"][r_start:r_start + r_len] = r_values
        data = [slave, fun_no, (r_start>>8)&0xFF, r_start&0xFF, (r_len>>8)&0xFF, r_len&0xFF] #打包
        data += self.crc16(*data)
        #self.serial.write(data)
        self.serial.write(struct.pack("B" * len(data), *data))
        
    def rcvElectricalParamHandler(self, slave):
        a0 = self.serial.read(1)
        if 1 != len(a0):
            return False
        a0, = struct.unpack("B", a0)
        R.log("ElectricalParam", slave, a0)
        if 0x03 == a0:
            self.readHoldingRegisters(slave)
        elif 0x06 == a0:
            self.writeSingleRegister(slave)
        elif 0x10 == a0:
            self.writeRegisters(slave)
        else:
            return False
    def rcvHandler(self):
        a0 = self.serial.read(1)
        if 1 != len(a0):
            return False
        a0, = struct.unpack("B", a0)
        if 0xCC == a0:
            return self.rcvActuatorHandler(a0)
        else:
            return self.rcvElectricalParamHandler(a0)
                    
    def run(self):
        while R.serial:
            self.rcvHandler()
            tm = time.time()
            if tm - Meter.elecStartTime > 60:
                Meter.elecStartTime = tm
                Meter.elecIndex = 0
            R.log("select-----------------------------------")
            
#---------------------------------------------------------------------------
class MainPanel(wx.Panel):
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)
        R.panel = self
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
        self.saveParaBtn = wx.Button(self,-1,u"设置参数", pos=(x, y),
                              size=(60,28),
                              style=wx.TE_MULTILINE)
        x += 60 + 5
        self.logCheckBox = wx.CheckBox(self, -1, u"日志", pos=(x, y + 5), size=(60, -1), style=wx.NO_BORDER)
        
        self.Bind(wx.EVT_COMBOBOX_DROPDOWN, self.comScanHandler, self.nameListBox)
        self.Bind(wx.EVT_BUTTON, self.startServerBtnHandler, self.startServerBtn)
        self.Bind(wx.EVT_CHECKBOX, self.logCheckBoxHandler, self.logCheckBox)
        self.Bind(wx.EVT_BUTTON, self.saveParaBtnHandler, self.saveParaBtn)
        
        ##------------------------------------------------------------------------------
        ##添加可选项
        x = 5
        y = 5
        
        wx.StaticText(self, -1, u"执行器", pos=(x, y + 4))
        y += 30
        x += 40
        wx.StaticText(self, -1, u"日配注", pos=(x, y + 4))
        self.valueBox1 = wx.TextCtrl(self,-1, "", pos=(x + 40, y),
                              size=(70,-1), style=wx.TE_READONLY)  
        x += 110 + 10
        wx.StaticText(self, -1, u"调节精度", pos=(x, y + 4))
        self.valueBox2 = wx.TextCtrl(self,-1, "", pos=(x + 55, y),
                              size=(70,-1), style=wx.TE_READONLY) 

        x += 125 + 10
        wx.StaticText(self, -1, u"自动调节", pos=(x, y + 4))
        self.valueBox3 = wx.TextCtrl(self,-1, "", pos=(x + 55, y),
                              size=(40,-1), style=wx.TE_READONLY) 
                              
        x += 95 + 10
        wx.StaticText(self, -1, u"起始计算时间", pos=(x, y + 4))
        self.valueBox4 = wx.TextCtrl(self,-1, "", pos=(x + 75, y),
                              size=(40,-1), style=wx.TE_READONLY)
                              
        x = 45
        y += 30
        wx.StaticText(self, -1, u"总流量", pos=(x, y + 4))
        self.valueBox5 = wx.TextCtrl(self,-1, "1", pos=(x + 40, y),
                              size=(70,-1))
        x += 110 + 10                      
        wx.StaticText(self, -1, u"瞬时流量", pos=(x, y + 4))
        self.valueBox6 = wx.TextCtrl(self,-1, "1", pos=(x + 55, y),
                              size=(70,-1))
                              
        x += 125 + 10                      
        wx.StaticText(self, -1, u"单向连续调节次数", pos=(x, y + 4))
        self.valueBox7 = wx.TextCtrl(self,-1, "1", pos=(x + 100, y),
                              size=(50,-1))
        x += 150 + 10                      
        wx.StaticText(self, -1, u"流量计通信状态", pos=(x, y + 4))
        self.valueBox8 = wx.TextCtrl(self,-1, "1", pos=(x + 90, y),
                              size=(50,-1))
        x = 45
        y += 30                   
        wx.StaticText(self, -1, u"流量计型号", pos=(x, y + 4))
        self.valueBox9 = wx.TextCtrl(self,-1, "1", pos=(x + 70, y),
                              size=(50,-1))
                              
        x += 120 + 10                      
        wx.StaticText(self, -1, u"当前角度", pos=(x, y + 4))
        self.valueBox10 = wx.TextCtrl(self,-1, "1", pos=(x + 50, y),
                              size=(50,-1))
                              
        x += 100 + 10                      
        wx.StaticText(self, -1, u"版本号", pos=(x, y + 4))
        self.valueBox11 = wx.TextCtrl(self,-1, "1.1.1.1", pos=(x + 40, y),
                              size=(100,-1))
                              
        x += 140 + 10                      
        wx.StaticText(self, -1, u"当前累计调节次数", pos=(x, y + 4))
        self.valueBox12 = wx.TextCtrl(self,-1, "1", pos=(x + 100, y),
                              size=(50,-1))
                              
        x = 45
        y += 30                   
        wx.StaticText(self, -1, u"自动调节标志位", pos=(x, y + 4))
        self.valueBox13 = wx.TextCtrl(self,-1, "1", pos=(x + 90, y),
                              size=(50,-1))
                              
        x += 140 + 10                      
        wx.StaticText(self, -1, u"阀门关闭状态标志", pos=(x, y + 4))
        self.valueBox14 = wx.TextCtrl(self,-1, "1", pos=(x + 100, y),
                              size=(50,-1))
                              
        x += 150 + 10                      
        wx.StaticText(self, -1, u"单向连续调节角度", pos=(x, y + 4))
        self.valueBox15 = wx.TextCtrl(self,-1, "1", pos=(x + 110, y),
                              size=(80,-1))
                              
        x = 45
        y += 30                   
        wx.StaticText(self, -1, u"系统累计上电时间", pos=(x, y + 4))
        self.valueBox16 = wx.TextCtrl(self,-1, "1", pos=(x + 100, y),
                              size=(100,-1))
        x += 210
        wx.StaticText(self, -1, u"下发时间", pos=(x, y + 4))
        self.valueBox17 = wx.TextCtrl(self,-1, "1", pos=(x + 60, y),
                              size=(100,-1), style=wx.TE_READONLY)
                              
        ########################-------------电参
        x = 5
        y += 30 
        #wx.StaticText(self, -1, u"有线电参 : 使用modbus修改(30写1保存)", pos=(x, y + 4))
        wx.StaticText(self, -1, u"有线电参", pos=(x, y + 4))
        
        ############第一列
        y += 30
        x += 40
        wx.StaticText(self, -1, u"电压量程", pos=(x, y + 4))
        x += 55
        self.valueBox31 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))  
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"电流量程", pos=(x, y + 4))
        x += 55
        self.valueBox32 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"正向有功功耗", pos=(x, y + 4))
        x += 78
        self.valueBox33 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"反向有功功耗", pos=(x, y + 4))
        x += 78
        self.valueBox34 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
        ############第二列
        y += 30
        x = 45
        
        wx.StaticText(self, -1, u"A相电压", pos=(x, y + 4))
        x += 55
        self.valueBox35 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
        x += 70 + 10
        wx.StaticText(self, -1, u"B相电压", pos=(x, y + 4))
        x += 55
        self.valueBox36 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))  
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"C相电压", pos=(x, y + 4))
        x += 55
        self.valueBox37 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"A相电流", pos=(x, y + 4))
        x += 55
        self.valueBox38 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"B相电流", pos=(x, y + 4))
        x += 55
        self.valueBox39 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
        ############第三列                      
        y += 30
        x = 45
        wx.StaticText(self, -1, u"C相电流", pos=(x, y + 4))
        x += 55
        self.valueBox40 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))  
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"总有功功率", pos=(x, y + 4))
        x += 65
        self.valueBox41 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"总无功功率", pos=(x, y + 4))
        x += 65
        self.valueBox42 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"总视在功率", pos=(x, y + 4))
        x += 65
        self.valueBox43 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
        ############第四列                      
        y += 30
        x = 45
        wx.StaticText(self, -1, u"功率因数", pos=(x, y + 4))
        self.valueBox44 = wx.TextCtrl(self,-1, "10", pos=(x + 55, y),
                              size=(70,-1)) 
        
        x += 125 + 10                      
        wx.StaticText(self, -1, u"A项有功功率", pos=(x, y + 4))
        x += 75
        self.valueBox45 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"B项有功功率", pos=(x, y + 4))
        x += 75
        self.valueBox46 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
                              
        x += 70 + 10                      
        wx.StaticText(self, -1, u"C项有功功率", pos=(x, y + 4))
        x += 75
        self.valueBox47 = wx.TextCtrl(self,-1, "10", pos=(x, y),
                              size=(70,-1))
        ##------------------------------------------------------------------------------
        
    def msg(self, *msg):
        rc = -1
        if "change" == msg[0]:
            R.log(msg)
            value = (msg[3]<<24) | (msg[4]<<16) | (msg[5]<<8) | (msg[6])
            self.valueBox1.SetValue(str(value/1000))
            value = msg[7]
            self.valueBox2.SetValue(str(value))
            value = msg[8]
            self.valueBox3.SetValue(str(value))
            value = msg[9]
            self.valueBox4.SetValue(str(value))
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
            
    def saveParaBtnHandler(self, event):
        #print Meter.data["actuator"]
        value = int(self.valueBox5.GetValue())
        data = [value>>56&0xFF, value>>48&0xFF, value>>40&0xFF, value>>32&0xFF, value>>24&0xFF, value>>16&0xFF, value>>8&0xFF, value&0xFF]
        value = int(self.valueBox6.GetValue())
        data += [value>>24&0xFF, value>>16&0xFF, value>>8&0xFF, value&0xFF]
        value = int(self.valueBox7.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox8.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox9.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox10.GetValue())
        data += [value&0xFF]
        value = str(self.valueBox11.GetValue()).split(".")
        data += [int(value[0]), int(value[1]), int(value[2]), int(value[3])]
        value = int(self.valueBox12.GetValue())
        data += [value>>8&0xFF, value&0xFF]
        value = int(self.valueBox13.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox14.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox15.GetValue())
        data += [value&0xFF]
        value = int(self.valueBox16.GetValue())
        data += [value>>24&0xFF, value>>16&0xFF, value>>8&0xFF, value&0xFF, 0x00]
        Meter.data["actuator"] = data
        #print Meter.data["actuator"]
        
        #####################设置电参
        data = [0xFFFF for x in range(120)]
        data[2] = int(self.valueBox31.GetValue())
        data[3] = int(self.valueBox32.GetValue())
        value = int(self.valueBox33.GetValue())
        data[12:13] = [value>>8, value&0xFF]
        value = int(self.valueBox34.GetValue())
        data[14:15] = [value>>8, value&0xFF]
        
        data[64] = int(self.valueBox35.GetValue())
        data[65] = int(self.valueBox36.GetValue())
        data[66] = int(self.valueBox37.GetValue())
        data[67] = int(self.valueBox38.GetValue())
        data[68] = int(self.valueBox39.GetValue())
        data[69] = int(self.valueBox40.GetValue())
        data[70] = int(self.valueBox41.GetValue())
        data[71] = int(self.valueBox42.GetValue())
        data[72] = int(self.valueBox43.GetValue())
        data[73] = int(self.valueBox44.GetValue())
        
        data[78] = int(self.valueBox45.GetValue())
        data[79] = int(self.valueBox46.GetValue())
        data[80] = int(self.valueBox47.GetValue())
        Meter.data["elec_param"] = data
        
        config = json.dumps(Meter.data)
        file = open(R.configPath, "w")
        file.write(config)
        file.close()
        
    
class MainFrame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, u'有线仪表', 
                size=R.frameSize)
        self.Center()
        self.Bind(wx.EVT_CLOSE, self.close, self)
        win = MainPanel(self, R)
        
        
    def close(self, event):
        if R.serial:
            R.serial.close()
        R.serial = None
        R.logFile = None
        
        config = json.dumps(Meter.data)
        file = open(R.configPath, "w")
        file.write(config)
        file.close()
        sys.exit(0)


#---------------------------------------------------------------------------
            
if __name__ == "__main__":
    try:
        file = open(R.configPath, "r")
        config = file.read()
        Meter.data = json.loads(config)
        file.close()
    except:
        pass
    Meter.data["elec_param"][29] = 0
    app = wx.App()
    frame = MainFrame()
    frame.Show()
    app.MainLoop()