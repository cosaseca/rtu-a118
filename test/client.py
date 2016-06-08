#!/usr/bin/python
#  -*- coding:utf-8 -*-
import threading
import sys, socket, thread, time

mLock = thread.allocate_lock()
pLock = threading.Lock()
class R():
    threadNum = 0

class TcpClient(threading.Thread):
    def __init__(self, ip="::", port="10086"):
        threading.Thread.__init__(self)
        self.ip = str(ip)
        self.port = str(port)
#        print "ip :", repr(self.ip)
#        print "port :", repr(self.port)
        pLock.acquire()
        R.threadNum += 1
        pLock.release()

    def run(self):
        for res in socket.getaddrinfo(self.ip, self.port, socket.AF_UNSPEC, socket.SOCK_STREAM):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
                s.settimeout(2)
            except socket.error, msg:
#                print self.ip, self.port, "error1"
                s = None
                continue
            try:
                s.connect(sa)
            except socket.error, msg:
#                print self.ip, self.port, "error2"
                s.close()
                s = None
                continue
            break
        if None == s:
            pLock.acquire()
#            print self.ip, self.port, "error"
            R.threadNum -= 1
            pLock.release()
            return
        self.sock = s
        s.close()
        pLock.acquire()
        print self.ip, self.port, "ok"
        R.threadNum -= 1
        pLock.release()

def main(argv):
    ip = raw_input("please input your server ip :")
    i = 50  #start
    j = 1000 #stop
    while i < j:
        if R.threadNum > 50:
            time.sleep(1)
            continue
        client = TcpClient(ip, i);
        client.start()
        i += 1
    while R.threadNum > 0:
        time.sleep(3)

if __name__ == "__main__":
    main(sys.argv)
