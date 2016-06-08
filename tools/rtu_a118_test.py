#!/usr/bin/python
# -*- coding:utf-8 -*-

import threading
import socket
import struct
import binascii
import time
from Tkinter import *
import tkFont

class App(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    
    def runBtnHandle(self):
        print "123"
    
    def printOption(self, event):
        print self.typev.get()
    
    def run(self):
        root = Tk()
        i = 0
        j = 0
        k = 0
        ft = tkFont.Font(size=12)

        header = Frame(root)
        
        self.typev = StringVar(header)
#        self.typev.set('Python')
        om = OptionMenu(header,self.typev,'Python','PHP','CPP','C','Java','JavaScript','VBScript')
        om.grid(row=i, column=j)
        om.bind('<Button-1>', self.printOption)
        
        j += 1
        Label(header, width=4, height=1, text="地址").grid(row=i, column=j)
        
        j += 1
        self.ipText = Text(header, width = 32, height = 1)
        self.ipText.grid(row=i, column=j)
        
        j += 1
        Label(header, width=4, height=1, text="端口").grid(row=i, column=j)
        
        j += 1
        self.portText = Text(header, width = 32, height = 1)
        self.portText.grid(row=i, column=j)
        
        j += 1
        Button(header, text ="运行", command = self.runBtnHandle).grid(row=i, column=j)
        header.grid(row=k, column=0)
        
        body = Frame(root)
        i = 0
        j = 0
        k += 1
        self.rcvText = Text(body, width = 80, height = 10)
        self.rcvText.grid(row=i, column=j)
        body.grid(row=k, column=0)
        
        body1 = Frame(root)
        i = 0
        j = 0
        k += 1
        self.sndText = Text(body1, width = 80, height = 10)
        self.sndText.grid(row=i, column=j)
        body1.grid(row=k, column=0)
        
        i = 0
        j = 0
        k += 1
        footer = Frame(root)
        
        Label(footer, width=4, height=1, text="地址").grid(row=i, column=j)
        
        j += 1
        self.ipText = Text(footer, width = 32, height = 1)
        self.ipText.grid(row=i, column=j)
        
        j += 1
        Label(footer, width=4, height=1, text="端口").grid(row=i, column=j)
        
        j += 1
        self.portText = Text(footer, width = 32, height = 1)
        self.portText.grid(row=i, column=j)
        
        j += 1
        Button(footer, text ="发送", command = self.runBtnHandle).grid(row=i, column=j)
        footer.grid(row=k, column=0)
        
        root.mainloop()

if __name__ == "__main__":
    app = App()
    app.run()