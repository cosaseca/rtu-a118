#!/usr/bin/python
# -*- coding: utf-8 -*-
import threading
import select
import socket
from Tkinter import *

class TcpServer(threading.Thread):
    def __init__(self, ip="::", port="10086", backlog=7, handle=None):
        threading.Thread.__init__(self)
        self.ip = str(ip)
        self.port = str(port)
        self.backlog = backlog
        self.sock = None
        self.rset = []
        self.wset = []
        self.xset = []
        self.addrs = []
        self.handle = handle
        print "ip port", self.ip, self.port

    def run(self):
        s = -1
        backlog = self.backlog
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC,
                                      socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
            af, socktype, proto, canonname, sa = res
            try:
                #根据getaddrinfo()的返回信息初始化socket
                s = socket.socket(af, socktype, proto)
            except socket.error, err_msg:
                print err_msg #回显异常信息
                s = None
                continue
            try:
                #sa是(host,port)的二元组
                s.bind(sa)
                #监听客户端请求
                s.listen(backlog)
            except socket.error, err_msg:
                print err_msg
                s.close()
                s = None
                continue
            break
        self.rset.append(s)
        self.sock = s
        self.addrs.append((self.ip, self.port))
        while 1:
            rset, wset, xset = select.select(self.rset, self.wset, self.xset, 1)
            for s in rset:
                if s == self.sock:
                    c, addr = s.accept()
                    self.rset.append(c)
#                    print addr
                    self.addrs.append(addr)
                else:
                    data = s.recv(1024)
                    if not data:
#                        print s.getpeername()
                        self.addrs.remove(s.getpeername())
                        s.close()
                        self.rset.remove(s)
                    else:
                        print len(data)
                        if None != self.handle:
                            print "aaaaa"
                            self.handle(data)
                            print "AAAAA"
            print rset

class TcpClient(threading.Thread):
    def __init__(self, ip="::", port="10086"):
        threading.Thread.__init__(self)
        self.ip = str(ip)
        self.port = str(port)
        print "ip :", repr(self.ip)
        print "port :", repr(self.port)
    
    def send(self, msg):
        self.sock.send(msg)
        
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
        while(1):
            data = s.recv(1024)
            if not data:
                break
            else:
                print data


class A(Button):
    def __init__(self, root):
        self.root = root
        self.B = Button(root, text = "hello")
        self.B.pack()
    
        self.mBox = Text(root, width=10, height=10)
        self.mBox.pack()
    def play(self, text):
        self.B.config(text=text)

class App(Frame):
    def __init__(self, master=None):
        Frame.__init__(self, master, width=800, height=600)
        self.master.title("net tools")
        self.grid()
        self.grid_propagate(0)
        self.pack_propagate(0)
        self.createWigit()
        self.runFlag = 0
        self.m = None

    def runfun(e):
        print e
        if 0 == app.runFlag:
            app.runBtn.config(text="断开")
            app.runFlag = 1
            app.m = TcpClient(ip=app.ipText.get("0.0", END)[:-1],
                              port=app.portText.get("0.0", END)[:-1])
            app.m.start()
        else:
            app.runBtn.config(text="连接")
            app.runFlag = 0

    def sendmsg(e):
        app.m.send(app.sndText.get("0.0", END)[:-1])

    def type_select(e):
        print app.type_v.get()

    def createWigit(self):
        leftBox = Frame(self)
        btnBox = LabelFrame(leftBox, text="协议类型", labelanchor='nw')
        self.type_v = IntVar()
        self.type_v.set(1)
        self.udpBtn = Radiobutton(btnBox,variable = self.type_v,text = 'udp',value = 1,
                                  command=self.type_select)
        self.udpBtn.pack(anchor='w')
        self.tcpClientBtn = Radiobutton(btnBox,variable = self.type_v,text = 'tcp client',value = 2,
                                        command=self.type_select)
        self.tcpClientBtn.pack(anchor='w')
        self.tcpServerBtn = Radiobutton(btnBox,variable = self.type_v,text = 'tcp server',value = 3,
                                        command=self.type_select)
        self.tcpServerBtn.pack(anchor='w')
        btnBox.pack(anchor='w')
        
        ipBox = LabelFrame(leftBox, text="地址", labelanchor='nw')
        self.ipText = Text(ipBox, width=15, height=1)
        self.ipText.pack(anchor='w')
        ipBox.pack(anchor='w')
        
        portBox = LabelFrame(leftBox, text="端口", labelanchor='nw')
        self.portText = Text(portBox, width=15, height=1)
        self.portText.pack(anchor='w')
        portBox.pack(anchor='w')
        
        rcvCheckBox = LabelFrame(leftBox, text="接收格式", labelanchor='nw')
        self.rcvBtn = Checkbutton(rcvCheckBox, text="十六进制")
        self.rcvBtn.pack(anchor='w')
        rcvCheckBox.pack(anchor='w')
        
        sndCheckBox = LabelFrame(leftBox, text="发送格式", labelanchor='nw')
        self.sndBtn = Checkbutton(sndCheckBox, text="十六进制")
        self.sndBtn.pack(anchor='w')
        sndCheckBox.pack(anchor='w')
        
        desipBox = LabelFrame(leftBox, text="目的地址", labelanchor='nw')
        self.desipText = Text(desipBox, width=15, height=1)
        self.desipText.pack(anchor='w')
        desipBox.pack(anchor='w')
        
        desportBox = LabelFrame(leftBox, text="目的端口", labelanchor='nw')
        self.desportText = Text(desportBox, width=15, height=1)
        self.desportText.pack(anchor='w')
        desportBox.pack(anchor='w')
        
        self.runBtn = Button(leftBox, text="连接", command=self.runfun)
        self.runBtn.pack(anchor='w')
        leftBox.place(x=5, y=0)
        
        
        rightBox = Frame(self)
        rcvBox = LabelFrame(rightBox, text="接收区域", labelanchor='nw')
        self.rcvText = Text(rcvBox, width=80, height=20)
        self.rcvText.pack(anchor='w')
        rcvBox.pack(anchor='ne')
        
        sndBox = LabelFrame(rightBox, text="发送区域", labelanchor='nw')
        self.sndText = Text(sndBox, width=80, height=10)
        self.sndText.pack(anchor='w')
        sndBox.pack(anchor='ne')
        
        self.sndBtn = Button(rightBox, text="发送", command=self.sendmsg)
        self.sndBtn.pack(anchor='w')
        
        v = StringVar(rightBox)
        
        mm = OptionMenu(rightBox,v,'Python','PHP','CPP','C','Java','JavaScript','VBScript')
        mm.pack(anchor='ne')
        
        rightBox.place(x=150, y=0)

if __name__ == "__main__":
    app = App()
    app.mainloop()
#    root = Tk()
#    a = A(root)
#    server = TcpServer(handle=a.play)
#
#    B = Button(root, text ="Hello", command = server.start)
#    B.pack()
#    root.mainloop()

