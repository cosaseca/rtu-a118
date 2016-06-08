#!/usr/bin/python
# -*- coding: utf-8 -*-
import socket, struct, threading, time, sys, os.path, os, select
import ctypes, re, telnetlib, base64, binascii, xxtea, traceback
import SimpleHTTPServer
import SocketServer
try:
    import xlwt
except:
    pass

class Channel():
    def __init__(self):
        self.name = 0
        self.fd = 0
        self.type = 0
        self.time = 0
        self.status = 0
    def move(x):
        return 0x1<<x
    NAME = move(1)
    FD = move(2)
    TYPE = move(3)
    STATUS = move(4)
    ADD = move(5)
    RM = move(6)
    COPY = move(7)
    FDSET = move(8)
    @staticmethod
    def addChannelAction(head, channel):
        pass
    @staticmethod
    def rmChannelAction(head, channel):
        pass
    @staticmethod
    def setChannelAction(head, channel):
        pass
    @staticmethod
    def getChannelAction(head, channel):
        pass
    @staticmethod
    def setChannelFdAction(head, fdset):
        pass
    @staticmethod
    def setChannel(head, tail, channel, action, opt):
        if((opt & (NAME)) and (opt & (TYPE)) and (opt & (STATUS))):
            for x in head:
                if x.name == channel.name and x.type == channel.type and x.status == channel.status:
                    action(x, channel)
                    return x
            return 0
        elif((opt & (NAME)) and (opt & (TYPE))):
            for x in head:
                if x.name == channel.name and x.type == channel.type:
                    action(x, channel)
                    return x
            return 0
        elif (opt & (NAME)):
            i0 = 0
            for x in head:
                if x.name == channel.name:
                    action(x, channel)
            return i0
        elif (opt & (FDSET)):
            i0 = 0
            for x in head:
                if x.fd != 0:
                    action(x, channel)
            return i0
        return 0

class R():
    def __init__(self):
        pass
    startTime = 0
    channels = [Channel() for x in range(8)]
    fdset = []
#    xxteaKey = struct.unpack("<IIII", "1234567890123456")
    #xxteaKey = struct.unpack("<IIII", "F" * 16)
    xxteaKey = "1234567890123456"
#    xxteaKey = "F" * 16
    protocolHead = "IBBM"
    imgPath = "./img/"
    
    @staticmethod
    def exit():
        os.system("kill -9 " + str(os.getpid())) #杀掉进程

    @staticmethod
    def packHead(cmd, content="", head=protocolHead):
        return struct.pack("<4sHB8sII", head, cmd, 1, "0" * 8, len(content), int(time.time())) + content

    @staticmethod
    def unPackHead(data):
        if len(data) < 23:
            return ((protocolHead, 1, 1, "0" * 8, 0, int(time.time())), "null")
        if len(data) > 23:
            return (struct.unpack("<4sHB8sII", data[0:23]), data[23:])
        return (struct.unpack("<4sHB8sII", data[0:23]), "null")

class CameraUser(threading.Thread):
    def __init__(self, addr, key = "1234567890123456", name = 123, type="cav"):
        threading.Thread.__init__(self)
        self.name = name
        self.addr = addr
        self.type = type
        self.key = key
    def run(self):
        print "user name: ", self.name
        if "c" in self.type:
            control = CameraCotrolChannel(self.addr, self.key, self.name)
            control.start()
        if "v" in self.type:
            video = CameraVideoChannel(self.addr, self.key, self.name)
            video.start()
        if "a" in self.type:
            audio = CameraAudioChannel(self.addr, self.key, self.name)
            audio.start()

class CameraCotrolChannelHandle(threading.Thread):
    def __init__(self, sock):
        threading.Thread.__init__(self)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        while self.r:
            try:
                data = sock.recv(4)
                if 4 != len(data):
                    continue
                if R.protocolHead != data:
                    continue
                data += sock.recv(23 - 4)
                if 23 != len(data):
                    continue
                head, cmd, ver, res0, length, res1 = rHead = struct.unpack("<4sHB8sII", data[0:23])
                if length < 0 or length > 1024 - 23:
                    continue
                if length != 0:
                    data += sock.recv(length)
                if length +23 != len(data):
                    continue
                print "c rHead: ", rHead, binascii.b2a_hex(data[23:])
            except:
                break

class CameraCotrolChannel(threading.Thread):
    def __init__(self, addr, key = "1234567890123456", name = 123, type = 1):
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
        self.addr = addr
        self.name = name
        print "asasdsd", key
        self.key = struct.unpack("<4I", key)
        self.r = 1
        sock.connect(addr)
        
#       连接请求
        content = ""
        data = R.packHead(140, content)
        rHead, content = self.handleChannelWithRecv(data)
        content = struct.unpack("<BIIII", content)
#       认证
        a = list(content[1:])
        xxtea.btea(a, -4, self.key)
#        print a #, "解密随机数"
        content = struct.pack("<IIII", a[0], a[1], a[2], a[3])
        data = R.packHead(142, content)
        rHead, content = self.handleChannelWithRecv(data)
        content = struct.unpack("<B", content)
        print "channel", type, "verify:", content#, "1 is success"

#       通道声明
        content = struct.pack("<IIB", type, int(self.name), type)
        data = R.packHead(150, content)
        rHead, content = self.handleChannelWithRecv(data)
#        try:
#            content = struct.unpack("<IBBI", content)
#            print "channel create: ", content
#        except:
#            print "error"
#        if 1 != content[2]:
#            self.r = 0
    def handleChannel(self, data):
        sock = self.sock
        rHead, content = R.unPackHead(data)
#        print "sHead: ", rHead#, content
        sock.send(data)
    def readn(self, n):
        sock = self.sock
        data = ""
        while n != len(data):
            data += sock.recv(n - len(data))
        return data
    def handleChannelRecv(self):
#        sock = self.sock
#        data = sock.recv(4)
        data = self.readn(4)
        if 4 != len(data):
            print "4 != len(data)", repr(data)
            return ("null_head", "null")
        if R.protocolHead != data:
            print "R.protocolHead != data", repr(data)
            return ("not_IBBM", "null")
#        data += sock.recv(23 - 4)
        data += self.readn(23 - 4)
        if 23 != len(data):
            print "23 != len(data)", repr(data)
            return ("error_head", "null")
        rHead, content = R.unPackHead(data)
        head, cmd, ver, res0, length, res1 = rHead
        if length <= 0 or length > 100 * 1024 - 23:
            print "length <= 0 or length > 100 * 1024 - 23", repr(data)
            return ("error_len", "null")
#        while len(data) < 23 + length:
##            print len(data)
#            data += sock.recv(23 + length - len(data))
        data += self.readn(length)

        rHead, content = R.unPackHead(data)
#        print "rHead: ", rHead#, content
        return (rHead, content)
    def handleChannelWithRecv(self, data):
        sock = self.sock
        rHead, content = R.unPackHead(data)
#        print "sHead: ", rHead#, content
        sock.send(data)
        data = sock.recv(4)
        if 4 != len(data):
            print "4 != len(data)", repr(data)
            return ("null_head", "null")
        if R.protocolHead != data:
            print "R.protocolHead != data", repr(data)
            return
        data += sock.recv(23 - 4)
        if 23 != len(data):
            print "23 != len(data)", repr(data)
            return ("error_head", "null")
        rHead, content = R.unPackHead(data)
        head, cmd, ver, res0, length, res1 = rHead
        if length <= 0 or length > 100 * 1024 - 23:
            print "length <= 0 or length > 100 * 1024 - 23", repr(data)
            return ("error_len", "null")
        while len(data) < 23 + length:
            data += sock.recv(23 + length - len(data))
        
        rHead, content = R.unPackHead(data)
#        print "rHead: ", rHead#, content
        return (rHead, content)
    def run(self):
        sock = self.sock
        addr = self.addr
        
        handle = CameraCotrolChannelHandle(sock)
        handle.start()
#        time.sleep(3)

#        content = struct.pack("<IBB", 1, 1, 2)
#        data = R.packHead(306, content)
#        ret = self.handleChannelWithRecv(data)
#        if -1 == ret:
#            print "error"
#        print "ret ret", ret
#        rHead, content = ret
#        content = struct.unpack("<IBB", content)
#        print content
#电机测试
#相对运动
#        content = struct.pack("<IHBHB", 0, 1, 10, 1, 10)
#        data = R.packHead(503, content)
#        self.handleChannel(data)
#绝对运动
#        content = struct.pack("<IHH", 0, 2, 2)
#        data = R.packHead(500, content)
#        self.handleChannel(data)

        i = 0
        j = 0
        han = 50
        van = 10
        while self.r:
            try:
                content = struct.pack("<IBB", 1, 1, 1)
                data = R.packHead(154, content)
                self.handleChannel(data)
#电机测试
#相对运动
                if 1: #电机测试 相对运动
                    content = struct.pack("<IhBhB", 0, han, 1, van, 1)
                    data = R.packHead(503, content)
                    self.handleChannel(data)
                    i += 1
                    if i > 8:
                        i = 0
                        han = -han
                    j += 1
                    if j > 6:
                        j = 0;
                        van = -van
                time.sleep(1)
            except Exception, e:
                print e
                print traceback.format_exc()
                R.exit()
                break

#升级程序
class CameraImageUpgradeChannel(CameraCotrolChannel):
    def __init__(self, addr, key = "1234567890123456", path = "./img", ver = "1.2.3.273", imgType = "m3s", mac = "080808080808", pver = "1.2.3.162", name = 123, type = 1):
        CameraCotrolChannel.__init__(self, addr, key, name, type)
        self.path = path
        self.ver = ver
        self.imgType = imgType
        self.mac = mac
        self.pver = pver
    def run(self):
        sock = self.sock
        addr = self.addr
        while self.r:
            try:
                content = ""
                data = R.packHead(242, content)
                rHead, content = self.handleChannelWithRecv(data)
#                content = struct.unpack("<BIIII", content)
                print repr(rHead), repr(content)

                imagePath = self.path + "/" + self.ver + "." + self.imgType + ".img"
                imgSize = os.path.getsize(imagePath)
                #print imgSize%(1024*64)
                #print imgSize/(1024*64)
                
                imgFile = os.open(imagePath, os.O_RDONLY)
                packData = 1
                packLen = 0
                packIndex = imgSize/(1024*64)
                if imgSize%(1024*64) > 0:
                    packIndex += 1
                while 1:
                    packData = os.read(imgFile, 1024*64)
                    if packData == "":
                        break
                    packIndex -= 1
                    
                    content = struct.pack("=ii", 0, packIndex)
                    content += packData
                    data = R.packHead(244, content)
                    rHead, content = self.handleChannelWithRecv(data)
                    print self.mac, repr(rHead), repr(content), packIndex
                os.close(imgFile)
                
                rHead, content = self.handleChannelRecv()
                print self.mac, repr(rHead), repr(content)
                time.sleep(1)
                
                content = struct.pack("=i", 0)
                data = R.packHead(240, content)
                rHead, content = self.handleChannelWithRecv(data)
                print self.mac, self.imgType, self.pver + "-->" + self.ver, repr(rHead), repr(content)
                print "+"*100
                break
        
            except Exception, e:
                print e
                print traceback.format_exc()
                R.exit()
                break

class CameraVideoChannel(CameraCotrolChannel):
    def __init__(self, addr, key = "1234567890123456", name = 123, type = 2):
        CameraCotrolChannel.__init__(self, addr, key, name, type)
    def run(self):
        sock = self.sock
        addr = self.addr
        file = open(str(time.time()) + "helloVideo.h264", "w")
        tm = time.time()
        cLen = 0
        while self.r:
            try:
                print "v",
                rHead, content = self.handleChannelRecv()
                if "null" != content:
                    print len(content)
                    file.write(content[9:])
                cLen += len(content) + 23
                if time.time() - tm > 10:
                    print cLen/10
                    tm = time.time()
                    cLen = 0
        
            
            except Exception, e:
                print e
                print traceback.format_exc()
                file.close()
                R.exit()
                break

class CameraAudioChannel(CameraCotrolChannel):
    def __init__(self, addr, key = "1234567890123456", name = 123, type = 3):
        CameraCotrolChannel.__init__(self, addr, key, name, type)
    def run(self):
        sock = self.sock
        addr = self.addr
        file = open(str(time.time()) + "helloAudio.pcm", "w")
        while self.r:
            try:
                print "a",
                rHead, content = self.handleChannelRecv()
                if "null" != content:
                    if 151 != rHead[1]:
                        print len(content)
                        print struct.unpack("<IIIBI", content[0:17])
                        file.write(content[17:])
                else:
                    print "error", time.time()
            except Exception, e:
                print e
                print traceback.format_exc()
                file.close()
                R.exit()
                break

class ImageMake(threading.Thread):
    def __init__(self, version = "0.0.0.0", type = "m3s", sourceDir = "/root/m3s_tj_sdk/source", appDir = "/mnt/hgfs/tomato/m2/iCamerav1.6.tea", setupAudioFile = "/mnt/hgfs/tomato/m2/document/testnew.wav"):
        threading.Thread.__init__(self)
        self.type = type
        self.version = version
        self.sourceDir = sourceDir
        self.setupAudioFile = setupAudioFile
        self.appDir = appDir
    def run(self):
        print "+" * 100
        print "start"
        print "+" * 100
        time0 = time.time()
        info = ""
        if self.type == "m2":
            info += "cp ./config_m2 " + self.sourceDir + "/vendors/Ralink/RT5350/default_config;"
        elif self.type == "m3s":
            info += "cp ./config_m3s " + self.sourceDir + "/vendors/Ralink/RT5350/default_config;"
        elif self.type == "Bell":
            info += "cp ./configBell " + self.sourceDir + "/vendors/Ralink/RT5350/default_config;"
        info += "cp " + self.setupAudioFile + " " + self.sourceDir + "/vendors/Ralink/RT5350/111littile.wav;"
        info += "rm " + self.sourceDir + "/vendors/Ralink/RT5350/info.txt;"
        ret = os.system(info)
        if 0 != ret:
            R.exit()
        file = os.open(self.sourceDir + "/vendors/Ralink/RT5350/info.txt", os.O_CREAT | os.O_WRONLY)
        info = "firmware=" + self.version + "\n"
        info += "camera_type=CM1\n"
        info += "hardware=0.0.4.0\n"
        if self.type != "m2" and self.type != "m3s":
            info += "camera_model=" + self.type + "\n"
        else:
            info += "camera_model=" + (self.type).upper() + "\n"
        os.write(file, info)
        os.close(file)
        info = ""
        info += "cp ./rcS " + self.sourceDir + "/vendors/Ralink/RT5350/rcS;"
        info += "cp ./inittab " + self.sourceDir + "/vendors/Ralink/RT5350/inittab;"
        info += "cd " + self.appDir + ";make clean;make;"
        ret = os.system(info)
        print "+" * 100
        print self.appDir, ret
        print "application make ok, time: " + str(time.time() - time0) + "s"
        print "+" * 100
        if 0 != ret:
            R.exit()
        time.sleep(5)
        info = "cd " + self.sourceDir + ";make dep;make;"
        ret = os.system(info)
        print "+" * 100
        print self.sourceDir, self.setupAudioFile, ret
        print "image make ok, time: " + str(time.time() - time0) + "s"
        print "version: " + self.version + " type: " + self.type
        print "+" * 100
        if 0 != ret:
            R.exit()
        info = ""
        info += "cp /tftpboot/root_uImage " + "/mnt/hgfs/tomato/m2/document/python/img/" + self.version + "." + (self.type).lower() + ".img;"
        info += "cp /tftpboot/root_uImage " + "/root/sharednfs/" + self.version + "." + self.type + ".img;"
        info += "cp /tftpboot/root_uImage " + "/var/www/html/" + self.version + "." + (self.type).lower() + ".img;"
        os.system(info);
        time.sleep(3)

class TcpWorker(threading.Thread):
    def __init__(self, client, addr):
        threading.Thread.__init__(self)
        self.client = client
        self.addr = addr
        self.r = 1
    def run(self):
        maxBufSize = 1024
        client = self.client
        while(self.r):
            data = client.recv(maxBufSize)
            print data
            client.send(data)
            break

class TcpServer(threading.Thread):
    def __init__(self, ip, port):
        maxBlock = 5
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
        self.sock.bind((ip, port))
        self.sock.listen(maxBlock)
    def run(self):
        client , addr = self.sock.accept()
        worker = TcpWorker(client, addr)
        worker.start()
        
class HttpWorker(TcpWorker):
    def __init__(self, client, addr):
        TcpWorker.__init__(self, client, addr)
        
    def run(self):
        maxBufSize = 1024
        client = self.client
        file = open("abc.flv")
        data = client.recv(maxBufSize)
        print data
        content = "<a href=\"#\">hello world!</a>"
        data = "HTTP/1.0 200 OK\r\n"
        data += "Date:Mon,31Dec200104:25:57GMT\r\n"
        data += "Server:Apache/1.3.14(Unix)\r\n"
        data += "Content-type:text/html\r\n"
        data += "Content-length:"
        data += str(len(content)) + "\r\n\r\n"
        data += content
        #client.send(file.read(1024))
        data = client.send(data)
        print data
        while(self.r):
            data = client.recv(maxBufSize)
            print data
            content = "<a href=\"#\">hello world!</a>"
            data = "HTTP/1.0 200 OK\r\n"
            data += "Date:Mon,31Dec200104:25:57GMT\r\n"
            data += "Server:Apache/1.3.14(Unix)\r\n"
            data += "Content-type:text/html\r\n"
            data += "Content-length:"
            data += str(len(content)) + "\r\n\r\n"
            data += content
            #client.send(file.read(1024))
            data = client.send(data)
            print data
            time.sleep(1)
            #break
        
class HttpServer(TcpServer):
    def __init__(self, ip, port):
        TcpServer.__init__(self, ip, port)
    def run(self):
        client , addr = self.sock.accept()
        worker = HttpWorker(client, addr)
        worker.start()
        
class UdpClient(threading.Thread):
    def __init__(self, ip, port):
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((ip, port))
        mreq = struct.pack("=4sl", socket.inet_aton("224.0.1.255"), socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.sock = sock
        self.r = 1
        self.ip = ip
        self.port = port
    def run(self):
        sock = self.sock
        ip = self.ip
        port = self.port
        #file = open("a.txt", "w")
        while(self.r):
            #data = sock.recv(1024*60)
            #print repr(data[0:20]), len(data)
            #head = struct.unpack(">2BHIII16B", data[0:32])
            #print head
            #file.write(str(head))
            sock.sendto("hello", ("224.0.1.255", 10086))
            data = sock.recv(1024)
            print data
#            print port
            time.sleep(1)

class CameraUDPServerTest(threading.Thread):
    def __init__(self, addr):
        threading.Thread.__init__(self)
        self.addr = addr
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(addr)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        while(self.r):
            data, client = sock.recvfrom(packSize)
            head, cmd, ver, res0, length, res1 = struct.unpack("<4sHB8sII", data[0:23])
            if "IDCM" == head:
                if 100 == cmd:
                    body = struct.pack("<12sI", "F"*12, 10002)
                    body += "1.2.3.162\n"
                    body += "CM1\n"
                    body += "1.1.40\n"
                    body += "M2\n"
                    tm = time.time()
                    body += struct.pack("<I", tm)
                    data = struct.pack("<4sHB8sII", head, 101, ver, res0, len(body), res1)
                    data += body
                    sock.sendto(data, client)
                    print ('F' * 12) + " sock.sendto:", client, tm

class CameraUDPServerTest(threading.Thread):
    def __init__(self, addr):
        threading.Thread.__init__(self)
        self.addr = addr
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(addr)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        while(self.r):
            data, client = sock.recvfrom(packSize)
            head, cmd, ver, res0, length, res1 = struct.unpack("<4sHB8sII", data[0:23])
            if "IBBM" == head:
                if 100 == cmd:
                    body = struct.pack("<12sI", "F"*12, 10002)
                    body += "1.2.3.162\n"
                    body += "CM1\n"
                    body += "1.1.40\n"
                    body += "M2\n"
                    tm = time.time()
                    body += struct.pack("<I", tm)
                    data = struct.pack("<4sHB8sII", head, 101, ver, res0, len(body), res1)
                    data += body
                    sock.sendto(data, client)
                    print ('F' * 12) + " sock.sendto:", client, tm

class CameraUDPServerGroupTest(threading.Thread):
    def __init__(self, addr):
        threading.Thread.__init__(self)
        self.addr = addr
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        mreq = struct.pack("=4sl", socket.inet_aton("224.0.1.255"), socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        sock.bind(addr)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        R.startTime = time.time()
        while(self.r):
            data, client = sock.recvfrom(packSize)
            head, cmd, ver, res0, length, res1 = struct.unpack("<4sHB8sII", data[0:23])
            if "IBBM" == head:
                if 100 == cmd:
                    body = struct.pack("<12sI", "F"*12, 10002)
                    body += "1.2.3.162\n"
                    body += "CM1\n"
                    body += "1.1.40\n"
                    body += "M2\n"
                    tm = time.time()
                    body += struct.pack("<I", tm)
                    data = struct.pack("<4sHB8sII", head, 101, ver, res0, len(body), res1)
                    data += body
                    sock.sendto(data, ("224.0.1.255", 10001))
                    print ('F' * 12) + " sock.sendto:", client, tm

class CameraUDPServerSearchGroupTest(threading.Thread):
    def __init__(self, addr):
        threading.Thread.__init__(self)
        self.addr = addr
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        mreq = struct.pack("=4sl", socket.inet_aton("224.0.1.255"), socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        sock.bind(addr)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        R.startTime = time.time()
        while(self.r):
            data, client = sock.recvfrom(packSize)
            head, cmd, ver, res0, length, res1 = struct.unpack("<4sHB8sII", data[0:23])
            print struct.unpack("<4sHB8sII", data[0:23]), (time.time() - R.startTime)

class CameraSearchResult(threading.Thread):
    def __init__(self, sock, file):
        threading.Thread.__init__(self)
        self.r = 1
        self.sock = sock
        self.tm0 = 0
        self.file = file
    def run(self):
        sock = self.sock
        packSize = 1024
        file = self.file
#        print "MAC          s_time        0 c_time     r_time        ip"
        while 1:
            data, client= sock.recvfrom(packSize)
            #print len(data)
            head, cmd, ver, res0, length, tm1, mac, port= struct.unpack("<4sHB8sII" + "12sI", data[0:39])
            #print data[len(data)-4, len(data)]
            tm2, = struct.unpack("<I", data[len(data)-4:len(data)])
            tm3 = time.time()
            #print mac, tm0, tm1, tm2, tm3, client[0]
            s = mac + " " + str(self.tm0) + " " + str(tm1) + " " + str(tm2) + " " + str(tm3) + " " + str(client[0])
            file.write(s + "\r\n")
            print s
    def setStartTime(self, tm):
        self.tm0 = tm

class CameraSearch(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.r = 1
        self.ip = ''
        self.port = 10000
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
    def run(self):
        sock = self.sock
        file = open("udpTest0", "a")
#        file.write("\r\n")
        result = CameraSearchResult(sock, file)
        result.start()
        head = "IBBM"
        packSize = 1024
        while self.r:
            tm0 = time.time()
            data = struct.pack("<4sHB8sII", head, 100, 1, "00000000", 0, tm0)
            result.setStartTime(tm0)
            print "\r\n"
            file.write("\r\n")
            print "MAC          s_time        0 c_time     r_time        ip"
            sock.sendto(data, ("255.255.255.255", self.port))
            time.sleep(10)
        file.close()
#        print "MAC          s_time        0 c_time     r_time        ip"
#        file = open("udpTest0", "a")
#        file.write("\r\n")
#        while self.r:
#            data, client= sock.recvfrom(packSize)
#            #print len(data)
#            head, cmd, ver, res0, length, tm1, mac, port= struct.unpack("<4sHB8sII" + "12sI", data[0:39])
#            #print data[len(data)-4, len(data)]
#            tm2, = struct.unpack("<I", data[len(data)-4:len(data)])
#            tm3 = time.time()
#            #print mac, tm0, tm1, tm2, tm3, client[0]
#            s = mac + " " + str(tm0) + " " + str(tm1) + " " + str(tm2) + " " + str(tm3) + " " + str(client[0])
#            #file.write(s + "\r\n")
#            print s
        #file.close()



class CameraUDPQQ(threading.Thread):
    def __init__(self, addr):
        threading.Thread.__init__(self)
        self.addr = addr
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(addr)
        self.sock = sock
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        port = self.addr[1]
        while(self.r):
            data = sock.recvfrom(1024)
            print repr(data)


class UdpServer(threading.Thread):
    def __init__(self, ip, port):
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((ip, port))
        #print socket.IP_ADD_MEMBERSHIP, socket.IPPROTO_IP
        mreq = struct.pack("=4sl", socket.inet_aton("224.0.1.255"), socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        self.sock = sock
        self.port = port
        self.r = 1
    def run(self):
        sock = self.sock
        port = self.port
        #file = open("a.txt", "w")
        while(self.r):
            data = sock.recv(1024)
            #print repr(data[0:20]), len(data)
#            head = struct.unpack(">2BHIII16B", data[0:32])
            print data
            sock.sendto("world", ("224.0.1.255", 10087))
            #file.write(str(head))

class FlvReader(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        self.file = open("abc.flv")
        data = self.file.read(400)
        print len(data)
        data = struct.unpack(">50I", data[0:200])
        file = open("b.txt", "w")
        file.write(str(data))
        #while(1):
            #self.read()
            
class HttpClient(threading.Thread):
    def __init__(self, ip, port):
        threading.Thread.__init__(self)
        self.ip = ip
        self.port = port
    def postTest(self):
        content = "name=" + base64.b64encode("hello world!")
        data = "POST /index.php HTTP/1.1\r\n"
        data += "Host: localhost\r\n"
        data += "Content-Type: application/x-www-form-urlencoded\r\n"
        data += "Content-Length: " + str(len(content)) + "\r\n\r\n"
        data += content
        return data
    def getTest(self):
        content = "name=" + base64.b64encode("hello world!")
        data = "GET /index.php?" + content + " HTTP/1.1\r\n"
        data += "Host: localhost\r\n"
        data += "Content-Type: application/x-www-form-urlencoded\r\n"
        data += "Content-Length: 0\r\n\r\n"
        return data
    def run(self):
        ip = self.ip
        port = self.port
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.connect((ip, port))
        sock.send(self.getTest())
        data = sock.recv(1024)
        r = re.compile("^Content-Length: ([0-9]*)", re.MULTILINE)
        result = r.search(data)
        if result:
            length = int(data[result.start(1):result.end(1)])
            r = re.compile("\r\n\r\n", re.MULTILINE)
            result = r.search(data)
            if result:
                contentLength = len(data) - int(result.end())
                #print contentLength
                if contentLength < length:
                    data += sock.recv(contentLength)
                else:
                    r = re.compile("^HTTP/1.1 200 OK")
                    result = r.search(data)
                    if result:
                        print "OK"
        print data

class Filter(threading.Thread):
    def __init__(self, mac):
        threading.Thread.__init__(self)
        self.mac = mac
    def run(self):
        r = re.compile("^" + self.mac)
        file = open("udpTest0", "r")
        file2 = open(self.mac, "w")
        for line in file:
            m = r.search(line)
            if m:
                print line
                file2.write(line)
        file.close()
        file2.close()

class SysInfoExcelFilter(threading.Thread):
    def __init__(self, sumsheet, book, fileName, sheetNum):
        threading.Thread.__init__(self)
        self.fileName = fileName
        self.sheetName = self.fileName[0:len(self.fileName)-4]
        self.book = book
        self.sumsheet = sumsheet
        self.sheetNum = sheetNum
    def writeSum(self, sheet, fun, i, (x0, x1)):
        sheet.write(i, 1, fun)
        sheet.write(i, 2, xlwt.Formula(fun + "(" + self.sheetName + "!A1:A" + str(x1) + ")"))
        sheet.write(i, 3, xlwt.Formula(fun + "(" + self.sheetName + "!B1:B" + str(x1) + ")"))
        sheet.write(i, 4, xlwt.Formula(fun + "(" + self.sheetName + "!C1:C" + str(x1) + ")"))
        sheet.write(i, 5, xlwt.Formula(fun + "(" + self.sheetName + "!D1:D" + str(x1) + ")"))
        sheet.write(i, 6, xlwt.Formula(fun + "(" + self.sheetName + "!E1:E" + str(x1) + ")"))
        sheet.write(i, 7, xlwt.Formula(fun + "(" + self.sheetName + "!F1:F" + str(x1) + ")"))
        sheet.write(i, 8, xlwt.Formula(fun + "(" + self.sheetName + "!G1:G" + str(x1) + ")"))
        sheet.write(i, 9, xlwt.Formula(fun + "(" + self.sheetName + "!H1:H" + str(x1) + ")"))
        sheet.write(i, 10, xlwt.Formula(fun + "(" + self.sheetName + "!I1:I" + str(x1) + ")"))
        sheet.write(i, 11, xlwt.Formula(fun + "(" + self.sheetName + "!J1:J" + str(x1) + ")"))
    
    def run(self):
        r = re.compile("^Mem: (\d.*)K used, (\d.*)K free, 0K shrd, 0K buff, (\d.*)K cached")
        r1 = re.compile("^CPU: *(\d.*)% usr *(\d.*)% sys *(\d.*)% nice *(\d.*)% idle *(\d.*)% io *(\d.*)% irq *(\d.*)% *softirq")
        file = open(self.fileName, "r")
        i = 0
        sheet = self.book.add_sheet(self.fileName[0:len(self.fileName)-4])
        sheet.write(i, 0, "Mem")
        sheet.write(i, 1, "free")
        sheet.write(i, 2, "cached")
        sheet.write(i, 3, "usr")
        sheet.write(i, 4, "sys")
        sheet.write(i, 5, "nice")
        sheet.write(i, 6, "idle")
        sheet.write(i, 7, "io")
        sheet.write(i, 8, "irq")
        sheet.write(i, 9, "softirq")
        i += 1
        for line in file:
            m = r.search(line)
            m1 = r1.search(line)
            if m:
                s =  m.group(1) + "," + m.group(2) + "," + m.group(3)
                sheet.write(i, 0, int(m.group(1)))
                sheet.write(i, 1, int(m.group(2)))
                sheet.write(i, 2, int(m.group(3)))
#                i += 1
#                print s
            elif m1:
                s =  m1.group(1) + "," + m1.group(2) + "," + m1.group(3) + "," + m1.group(4) + "," + m1.group(5) + "," + m1.group(6) + "," + m1.group(7)
#                print s
                sheet.write(i, 3, int(m1.group(1)))
                sheet.write(i, 4, int(m1.group(2)))
                sheet.write(i, 5, int(m1.group(3)))
                sheet.write(i, 6, int(m1.group(4)))
                sheet.write(i, 7, int(m1.group(5)))
                sheet.write(i, 8, int(m1.group(6)))
                sheet.write(i, 9, int(m1.group(7)))
                i += 1
        file.close()
        j = i
        k = 6 * int(self.sheetNum)
        self.sumsheet.write(k, 0, self.sheetName)
        self.writeSum(self.sumsheet, "AVERAGE", k, (0, j))
        k += 1
        self.writeSum(self.sumsheet, "VAR", k, (0, j))
        k += 1
        self.writeSum(self.sumsheet, "MAX", k, (0, j))
        k += 1
        self.writeSum(self.sumsheet, "MIN", k, (0, j))
        print i

class SysInfoToExcel(threading.Thread):
    def __init__(self, name="sysInfo.xls"):
        threading.Thread.__init__(self)
        self.name = name
    def run(self):
        wbk = xlwt.Workbook()
        sheet = wbk.add_sheet(u"统计")
        info = wbk.add_sheet(u"说明")
        s = u'''
            统计结果在sumsum表格中
            sys_VideoCapture_m2表示开启VideoCapture线程在m2中的测试结果
            sys_200m_m3s表示开启到m线程后在m3s中的测试结果（详见线程排序）
            详细测试结果见sys_*表格
            '''
        s += u'''
            线程排序
            "DFLAGS += -D_CreateVideoCaptureThread  #1
            DFLAGS += -D_CreateAudioCaptureThread  #2 a
            DFLAGS += -D_CreateAudioAlarmThread     #3 b
            DFLAGS += -D_VideoAlarm_Thread_Creat   #4 c
            DFLAGS += -D_Creataudio_send_thread      #5 d
            DFLAGS += -D_Creatvideo_send_thread      #6 e
            DFLAGS += -D_createHttpsServerThread     #7 f
            DFLAGS += -D_NetworkSet_Thread_Creat   #8 g
            DFLAGS += -D_NewsChannel_threadStart    #9 h
            DFLAGS += -D_udpServer_threadStart          #12 k
            #DFLAGS += -D_CreateUpnpThread             #13 l
            DFLAGS += -D_flash_wr_thread_creat          #14 m
            DFLAGS += -D_Create_MFI_Thread             #15 n
            DFLAGS += -D_ioctl_thread_creat                #16 o
            DFLAGS += -D_Create_NTP_Thread            #18 p
            DFLAGS += -D_create_tutk_server_thread   #17 q
            DFLAGS += -D_SerialPorts_Thread_Creat    #11 j r
            DFLAGS += -D_CreateSpeakerThread         #10 i s"
            '''
        
        s += u'''
            数据源
            "Mem: 15016K used, 13400K free, 0K shrd, 0K buff, 7296K cached
            CPU:  13% usr   8% sys   0% nice  77% idle   0% io   0% irq   0% softirq
            Load average: 0.57 0.49 0.23
            PID  PPID USER     STAT   VSZ %MEM %CPU COMMAND
            854   852 admin    S    12596  44%  14% iCamera
            856   852 admin    S    12596  44%   7% iCamera
            916   913 admin    R     1432   5%   1% top
            869   852 admin    S    12596  44%   0% iCamera
            853   852 admin    S    12596  44%   0% iCamera
            864   852 admin    S    12596  44%   0% iCamera
            872   852 admin    S    12596  44%   0% iCamera
            868   852 admin    S    12596  44%   0% iCamera
            862   852 admin    S    12596  44%   0% iCamera
            861   852 admin    S    12596  44%   0% iCamera
            873   852 admin    S    12596  44%   0% iCamera
            865   852 admin    S    12596  44%   0% iCamera
            840     1 admin    S    12596  44%   0% iCamera
            910   852 admin    S    12596  44%   0% iCamera
            855   852 admin    S    12596  44%   0% iCamera
            860   852 admin    S    12596  44%   0% iCamera
            867   852 admin    S    12596  44%   0% iCamera 
            859   852 admin    S    12596  44%   0% iCamera 
            912   852 admin    S    12596  44%   0% iCamera 
            858   852 admin    S    12596  44%   0% iCamera "
            '''
        info.write(0, 0, s)
        r = re.compile(".*txt")
        sheetNum = 0
        for name in os.listdir("."):
            m = r.search(name)
            if m:
#                print name
                info = SysInfoExcelFilter(sheet, wbk, name, sheetNum)
                info.run()
                sheetNum += 1
        wbk.save(self.name)


class SysInfoFilter(threading.Thread):
    def __init__(self, fileName):
        threading.Thread.__init__(self)
        self.fileName = fileName
    def run(self):
        r = re.compile("^Mem: (\d.*)K used, (\d.*)K free, 0K shrd, 0K buff, (\d.*)K cached")
        r1 = re.compile("^CPU: *(\d.*)% usr *(\d.*)% sys *(\d.*)% nice *(\d.*)% idle *(\d.*)% io *(\d.*)% irq *(\d.*)% *softirq")
        file = open(self.fileName, "r")
        file1 = open(self.fileName + "filter_Mem.txt", "w")
        file2 = open(self.fileName + "filter_CPU.txt", "w")
        for line in file:
            m = r.search(line)
            m1 = r1.search(line)
            if m:
                s =  m.group(1) + "," + m.group(2) + "," + m.group(3)
                print s
                file1.write(s + "\r\n")
            elif m1:
                s =  m1.group(1) + "," + m1.group(2) + "," + m1.group(3) + "," + m1.group(4) + "," + m1.group(5) + "," + m1.group(6) + "," + m1.group(7)
                print s
                file2.write(s + "\r\n")
        file.close()
        file1.close()
        file2.close()

class SysInfoFilterLogSystem(threading.Thread):
    def __init__(self, fileName):
        threading.Thread.__init__(self)
        self.fileName = fileName
    def run(self):
        r = re.compile("^http response head HTTP/1.1 200 OK")
        r1 = re.compile("^http request head POST /index.php\?mac=AAAAAAAA")
        file = open(self.fileName, "r")
        i = 0.00
        j = 0.00
        for line in file:
            m = r.search(line)
            m1 = r1.search(line)
            if m:
                i += 1
            elif m1:
                j += 1
        print j, i, i/j
        file.close()
        
class SystemCPUInfo(threading.Thread):
    def __init__(self, host):
        threading.Thread.__init__(self)
        self.host = host
        
    def run(self):
        finish = "# "
        tn = telnetlib.Telnet(self.host)
        #tn.set_debuglevel(1)

        tn.read_until("login: ")
        tn.write("admin" + "\n")
        tn.read_until("Password: ")
        tn.write("admin\n")
        fileName = time.strftime("%H.%M.%S", time.localtime())
        file = open("cpu_info_" + fileName + ".txt", "w")
        for i in range(100):
            tn.write('cat /proc/stat\n') 
            a = tn.read_until(finish)
            file.write(a)
            print a
            time.sleep(5)
        tn.close()
        file.close()
        
class CPUInfoFilter(threading.Thread):
    def __init__(self, fileName):
        threading.Thread.__init__(self)
        self.fileName = fileName
        
    def run(self):
        r1 = re.compile("cpu  (\d*) (\d*) (\d*) (\d*) (\d*) (\d*) (\d*) (\d*)")
        r2 = re.compile("ctxt (\d*)")
        r3 = re.compile("btime (\d*)")
        r4 = re.compile("processes (\d*)")
        r5 = re.compile("procs_running (\d*)")
        r6 = re.compile("procs_blocked (\d*)")
        r7 = re.compile("#")
        file = open(self.fileName, "r")
        s = ""
        for line in file:
            m1 = r1.search(line)
            m2 = r2.search(line)
            m3 = r3.search(line)
            m4 = r4.search(line)
            m5 = r5.search(line)
            m6 = r6.search(line)
            m7 = r7.search(line)
            if m1:
                s += m1.group(1) + "," + m1.group(2) + "," +  m1.group(3) + "," + m1.group(4) + "," + m1.group(5)+ "," + m1.group(6) + "," + m1.group(7) + "," + m1.group(8)
                #print m1.group(1), m1.group(2), m1.group(3), m1.group(4), m1.group(5), m1.group(6), m1.group(7), m1.group(8)
            elif m2:
                s += "," + m2.group(1)
                #print m2.group(1)
            elif m3:
                s += "," + m3.group(1)
                #print m3.group(1)
            elif m4:
                s += "," + m4.group(1)
                #print m4.group(1)
            elif m5:
                s += "," + m5.group(1)
                #print m5.group(1)
            elif m6:
                s += "," + m6.group(1)
                #print m6.group(1)
            elif m7:
                s += "\r\n"
                print s
                s = ""
            else:
                pass
                #print "else"
def help():
    s = '''
python tomato.py camera           模拟摄像头
python tomato.py camerag          组播试验
python tomato.py camerags         组播试验
python tomato.py udpPrint         UDP打印
python tomato.py qq               一个测试
python tomato.py search           搜索摄像头
python tomato.py search_loop      一直搜索摄像头
python tomato.py filter           过滤
python tomato.py control ip       建立控制通道
python tomato.py video ip         建立视频通道
python tomato.py audio ip         建立音频通道
python tomato.py cameraUser ip name [type] 建立用户
python tomato.py user ip key [name [type]] 建立用户
python tomato.py filterSysInfo    系统信息过滤
python tomato.py SysInfoToExcel   系统信息道出excel
python tomato.py SysInfoFilterLogSystem
python tomato.py upgrade ip
python tomato.py make version [type]
python tomato.py make_isc3 version [type]
'''
    print s

def main(argv):
    #server = HttpServer('', 10086)
    #server.run()
    #client = UdpClient('127.0.0.1', 5004)
    #client.run()
    #reader = FlvReader()
    #reader.run()
    #client = HttpClient("10.0.88.29", 80)
    #client.run()r
    #server = UdpServer("0.0.0.0", 10086)
    #server.start()
    #client = UdpClient("0.0.0.0", 10087)
    #client.run()
    #camera = CameraUDPServerTest(("0.0.0.0", 10000))
    #camera.run()
    if len(argv) < 2:
        help()
        return
    if "camera" == argv[1]:
        camera = CameraUDPServerTest(("0.0.0.0", 10000))
        camera.run()
    elif "camerag" == argv[1]:
        camera = CameraUDPServerGroupTest(("0.0.0.0", 10000))
        camera.run()
    elif "camerags" == argv[1]:
        camera = CameraUDPServerSearchGroupTest(("0.0.0.0", 10001))
        camera.run()
    elif "udpPrint" == argv[1]:
        client = CameraUDPPrintTest(("0.0.0.0", 10088), argv[2])
        client.run()
    elif "qq" == argv[1]:
        client = CameraUDPQQ(("0.0.0.0", 10088))
        client.run()
    elif "search" == argv[1]:
        client = CameraSearch()
        client.run()
    elif "search_loop" == argv[1]:
        while 1:
            client = CameraSearch()
            client.start()
            time.sleep(10)
            client.close()
    elif "filter" == argv[1]:
        filter = Filter(argv[2])
        filter.run()
    elif "control" == argv[1]:
        control = CameraCotrolChannel((argv[2], 10002))
        control.run()
    elif "video" == argv[1]:
        video = CameraVideoChannel((argv[2], 10002))
        video.run()
    elif "audio" == argv[1]:
        audio = CameraAudioChannel((argv[2], 10002))
        audio.run()
    elif "cameraUser" == argv[1]:
        if len(argv) == 5:
            user = CameraUser((argv[2], 10002), int(argv[3]), argv[4])
        else:
            user = CameraUser((argv[2], 10002), int(argv[3]))
        user.run()
        while 1:
            try:
                time.sleep(100)
            except:
                break
    elif "user" == argv[1]:
        if len(argv) == 6:
            user = CameraUser((argv[2], 10002), argv[3], int(argv[4]), argv[5])
        elif len(argv) == 4:
            user = CameraUser((argv[2], 10002), argv[3])
        else:
            user = CameraUser((argv[2], 10002))
        user.run()
        while 1:
            try:
                time.sleep(100)
            except:
                break
    elif "filterSysInfo" == argv[1]:
        filter = SysInfoFilter(argv[2])
        filter.run()
    elif "SysInfoFilterLogSystem" == argv[1]:
        filter = SysInfoFilterLogSystem(argv[2])
        filter.run()
    elif "SysInfoToExcel" == argv[1]:
        info = SysInfoToExcel()
        info.run()
    elif "httpServer" == argv[1]:
        PORT = 8080
        Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
        httpd = SocketServer.TCPServer(("", PORT), Handler)
        os.system("ifconfig")
        print "serving at port", PORT
        httpd.serve_forever()
    elif "upgrade" == argv[1]:
        channel = CameraImageUpgradeChannel((argv[2], 10002), R.xxteaKey)
        channel.run()
    elif "make" == argv[1]:
        tool = ImageMake(argv[2], "m2")
        tool.run()
        time.sleep(5)
        tool = ImageMake(argv[2], "m3s")
        tool.run()
    elif "make_isc3" == argv[1]:
        tool = ImageMake(argv[2], "m3s", "/root/RT288x_SDK/source_mi", "/root/iCamera_MI_V1.0")
        tool.run()
        R.exit()
    elif "make_log" == argv[1]:
        tool = ImageMake(argv[2], "m3s", "/root/RT288x_SDK/source", "/mnt/hgfs/tomato/m2/iCamerav2.0\ selflog")
        tool.run()
        tool = ImageMake(argv[2], "m2", "/root/RT288x_SDK/source", "/mnt/hgfs/tomato/m2/iCamerav2.0\ selflog")
        tool.run()
        R.exit()
    elif "cpu_info" == argv[1]:
        info = SystemCPUInfo(argv[2])
        info.run()
    elif "cpu_filter" == argv[1]:
        filter = CPUInfoFilter(argv[2])
        filter.run()
        
if __name__ == "__main__":
    main(sys.argv)
