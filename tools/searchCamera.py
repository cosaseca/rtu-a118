#!/usr/bin/python
# -*- coding:utf-8 -*-
import socket, struct, threading, time, sys, os, tomato

class R():
    def __init__(self):
        pass
    xxteaKey = "1234567890123456"
    xxteaKey = "F" * 16

class CameraSearchResult(threading.Thread):
    def __init__(self, sock, action = "print", upver = "1.2.3.273", macs=["080808080808", ], path = "./img", ver = "1.2.3.273", imgType = "m3s", mac = "080808080808", pver = "1.2.3.162"):
        threading.Thread.__init__(self)
        self.r = 1
        self.sock = sock
        self.tm0 = 0
        self.camera = {}
        monitor = CameraSearchMonitor(self.camera) #监测摄像头是否离线
        monitor.start()
        self.action = action
        self.path = path
        self.upver = upver
        self.imgType = imgType
        self.mac = mac
        self.macs = macs
        self.pver = pver
    def run(self):
        sock = self.sock
        packSize = 1024
        while self.r:
            data, client= sock.recvfrom(packSize) #接收数据
            if len(data) < 39:
                print repr(data)
                continue
            head, cmd, ver, res0, length, tm1, mac, port= struct.unpack("<4sHB8sII" + "12sI", data[0:39]) #数据解包
            tm2, = struct.unpack("<I", data[len(data)-4:len(data)])
            tm3 = time.time()
            if mac in self.camera.keys():
                self.camera[mac]["curtime"] = tm3 #更新时间
                continue
            info = data[39:len(data)-4]
            info = info.split("\n")
            self.camera[mac] = {"searchtime": self.tm0, "packtime": tm2, "curtime": tm3, "info": info}
            s = "up!! " + mac + " " + str(client[0]) + " " + time.ctime(self.tm0) + " " + str(tm3 - self.tm0) + " " + str(tm2) + " " + str(info) #打印摄像头信息
            curVer = info[0]
            curImgType = info[3].lower()
            if "print" == self.action:
                print self.action, s
            elif "upgrade" == self.action:
                print self.action, s, self.upver
                if curVer != self.upver:
                    channel = tomato.CameraImageUpgradeChannel((client[0], 10002), R.xxteaKey,  "./img", self.upver, curImgType, mac, curVer)
                    channel.start()
            elif "upgrade_mac" == self.action:
                print self.action, s, self.upver
                if curVer != self.upver and mac in self.macs:
                    channel = tomato.CameraImageUpgradeChannel((client[0], 10002), R.xxteaKey, "./img", self.upver, curImgType, mac, curVer)
                    channel.start()
            elif "upgrade_m2" == self.action:
                print self.action, s, self.upver
                if curVer != self.upver and "m2" == curImgType:
                    channel = tomato.CameraImageUpgradeChannel((client[0], 10002), R.xxteaKey, "./img", self.upver, curImgType, mac, curVer)
                    channel.start()
            elif "upgrade_m3s" == self.action:
                print self.action, s, self.upver
                if curVer != self.upver and "m3s" == curImgType:
                    channel = tomato.CameraImageUpgradeChannel((client[0], 10002), R.xxteaKey, "./img", self.upver, curImgType, mac, curVer)
                    channel.start()
    def setStartTime(self, tm):
        self.tm0 = tm

class CameraSearchMonitor(threading.Thread):
    def __init__(self, camera = {}, rate = 5):
        threading.Thread.__init__(self)
        self.r = 1
        self.rate = rate
        self.camera = camera
        self.timeout = 30
    def run(self):
        while self.r:
            tm0 = time.time()
            for mac in self.camera.keys():
                if tm0 - self.camera[mac]["curtime"] > self.timeout: #监测超时
                    print "down", mac, time.ctime(tm0), tm0 - self.camera[mac]["searchtime"], self.camera[mac]["info"]
                    del self.camera[mac] #删除摄像头
            time.sleep(self.rate)

class CameraSearchServer(threading.Thread):
    def __init__(self, action = "print", upver = "1.2.3.273", macs=["080808080808", ], path = "./img", ver = "1.2.3.273", imgType = "m3s", mac = "080808080808", pver = "1.2.3.162", addr = ('', 10001), rate = 3):
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #创建套接字
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1) #设置广播选项
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) #设置地址复用选项
        sock.bind(addr)
        self.addr = addr
        self.sock = sock
        
        self.r = 1
        self.result = CameraSearchResult(sock, action, upver, macs) #搜索结果统计
        self.result.start()
        self.rate = rate
    def run(self):
        sock = self.sock
        head = "IBBM"
        resv = "0" * 8
        length = 0
        word = 1
        version = 1
        destAddr = ("255.255.255.255", 10000) #广播地址
        while self.r:
            tm0 = time.time()
            data = struct.pack("<4sHB8sII", head, 100, version, resv, length, tm0) #打包搜索命令
            self.result.setStartTime(tm0)
            try:
                sock.sendto(data, destAddr)
                time.sleep(self.rate) #睡眠一段时间
            except:
                break

'''
进入终端
在searchCamera.py所在目录下运行 python ./searchCamera.py 然后回车
打印结果
up B8B94E007D5A 192.168.139.22 Fri Jan 17 11:14:34 2014 2.28528285027 443243409.022 ['1.2.3.200m', 'CM1', '0.0.4.0', 'M2', '']
up B8A94E000006 192.168.139.21 Fri Jan 17 11:14:37 2014 1.61818289757 443243431.355 ['1.2.3.200m', 'CM1', '0.0.4.0', 'M3S', '']

若有摄像头掉钱就会打印
down B8A94E000006 Fri Jan 17 13:42:33 2014 36.0022978783 ['1.2.3.162', 'CM1', '1.1.40', 'M2', '']
'''

def main(argv):
    helpInfo = '''
python searchCamera.py s[earch]
python searchCamera.py up[grade]      version
python searchCamera.py up[grade]_mac  version mac1 [mac2 ...]
python searchCamera.py up[grade]_m3s  version
python searchCamera.py up[grade]_m2   version
python searchCamera.py up[grade]_bell version
'''
    if 1 == len(argv):
        print helpInfo
    elif "search" == argv[1] or "s" == argv[1]:
        server = CameraSearchServer()
        server.run()
    elif "upgrade" == argv[1] or "up" == argv[1]:
        server = CameraSearchServer("upgrade", argv[2], argv[3:])
        server.run()
    elif "upgrade_mac" == argv[1] or "up_mac" == argv[1]:
        server = CameraSearchServer("upgrade_mac", argv[2], argv[3:])
        server.run()
    elif "upgrade_m3s" == argv[1] or "up_m3s" == argv[1]:
        server = CameraSearchServer("upgrade_m3s", argv[2], argv[3:])
        server.run()
    elif "upgrade_m2" == argv[1] or "up_m2" == argv[1]:
        server = CameraSearchServer("upgrade_m2", argv[2], argv[3:])
        server.run()
    elif "upgrade_bell" == argv[1] or "up_bell" == argv[1]:
        server = CameraSearchServer("upgrade_bell", argv[2], argv[3:])
        server.run()
    os.system("kill -9 " + str(os.getpid())) #杀掉进程

if __name__ == "__main__":
    main(sys.argv)

'''
相关协议
数据头部定义（23bytes）
    头部（4bytes字符） 命令号(2bytes整形) 协议版本(1byte整型) 保留段(8bytes字符) 数据体长度(4bytes整型) 时间戳(4bytes整型)
    head(IBBM)       cmd              version(1)        resv(00000000)    length              time
数据体
    自定义
    
UDP搜索协议 cmd ＝ 100, 数据体为空
    返回cmd ＝ 101, 数据体中包含摄像头信息, 如mac地址、摄像头类型、固件版本等
'''

'''
UDP是OSI参考模型中一种无连接的传输层协议，它主要用于不要求分组顺序到达的传输中，分组传输顺序的检查与排序由应用层完成[1]，提供面向事务的简单不可靠信息传送服务。
UDP 协议基本上是IP协议与上层协议的接口。UDP协议适用端口分别运行在同一台设备上的多个应用程序。

UDP协议的全称是用户数据包协议[2]，在网络中它与TCP协议一样用于处理数据包，是一种无连接的协议。在OSI模型中，在第四层——传输层，处于IP协议的上一层。
UDP有不提供数据包分组、组装和不能对数据包进行排序的缺点，也就是说，当报文发送之后，是无法得知其是否安全完整到达的。
UDP用来支持那些需要在计算机之间传输数据的网络应用。包括网络视频会议系统在内的众多的客户/服务器模式的网络应用都需要使用UDP协议。
UDP协议从问世至今已经被使用了很多年，虽然其最初的光彩已经被一些类似协议所掩盖，但是即使是在今天UDP仍然不失为一项非常实用和可行的网络传输层协议。

与所熟知的TCP（传输控制协议）协议一样，UDP协议直接位于IP（网际协议）协议的顶层。根据OSI（开放系统互连）参考模型，UDP和TCP都属于传输层协议。
UDP协议的主要作用是将网络数据流量压缩成数据包的形式。一个典型的数据包就是一个二进制数据的传输单位。每一个数据包的前8个字节用来包含报头信息，剩余字节则用来包含具体的传输数据。

在选择使用协议的时候，选择UDP必须要谨慎。在网络质量令人十分不满意的环境下，UDP协议数据包丢失会比较严重。
但是由于UDP的特性：它不属于连接型协议，因而具有资源消耗小，处理速度快的优点，所以通常音频、视频和普通数据在传送时使用UDP较多，因为它们即使偶尔丢失一两个数据包，
也不会对接收结果产生太大影响。比如我们聊天用的ICQ和QQ就是使用的UDP协议。

组播地址
在IP地址空间中，有的IP地址不能为设备分配的，有的IP地址不能用在公网，有的IP地址只能在本机使用，诸如此类的特殊IP地址众多：
注意它和广播的区别。从224.0.0.0到239.255.255.255都是这样的地址。224.0.0.1特指所有主机， 224.0.0.2特指所有路由器。这样的地址多用于一些特定的程序以及多媒体程序。
如果你的主机开启了IRDP（Internet路由发现协议，使用组播功能）功能，那么你的主机路由表中应该有这样一条路由。
169.254.x.x
如果你的主机使用了DHCP功能自动获得一个IP地址，那么当你的DHCP服务器发生故障，或响应时间太长而超出了一个系统规定的时间，Windows系统会为你分配这样一个地址。
如果你发现你的主机IP地址是一个诸如此类的地址，很不幸，十有八九是你的网络不能正常运行了。

受限广播地址
广播通信是一对所有的通信方式。若一个IP地址的2进制数全为1，也就是255.255.255.255，则这个地址用于定义整个互联网。如果设备想使IP数据报被整个Internet所接收，
就发送这个目的地址全为1的广播包，但这样会给整个互联网带来灾难性的负担。因此网络上的所有路由器都阻止具有这种类型的分组被转发出去，使这样的广播仅限于本地网段。

广播地址
直接广播地址
一个网络中的最后一个地址为直接广播地址，也就是HostID全为1的地址。主机使用这种地址把一个IP数据报发送到本地网段的所有设备上，路由器会转发这种数据报到特定网络上的所有主机。
注意：这个地址在IP数据报中只能作为目的地址。另外，直接广播地址使一个网段中可分配给设备的地址数减少了1个。

源IP地址
若IP地址全为0，也就是0.0.0.0，则这个IP地址在IP数据报中只能用作源IP地址，这发生在当设备启动时但又不知道自己的IP地址情况下。
在使用DHCP分配IP地址的网络环境中，这样的地址是很常见的。用户主机为了获得一个可用的IP地址，就给DHCP服务器发送IP分组，并用这样的地址作为源地址，
目的地址为255.255.255.255（因为主机这时还不知道DHCP服务器的IP地址）。

NetID为0的
当某个主机向同一网段上的其他主机发送报文时就可以使用这样的地址，分组也不会被路由器转发。
比如12.12.12.0/24这个网络中的一台主机12.12.12.2/24在与同一网络中的另一台主机12.12.12.8/24通信时，目的地址可以是0.0.0.8。

环回地址
127网段的所有地址都称为环回地址，主要用来测试网络协议是否工作正常的作用。比如使用ping
127.0.0.1就可以测试本地TCP/IP协议是否已正确安装。另外一个用途是当客户进程用环回地址发送报文给位于同一台机器上的服务器进程，
比如在浏览器里输入127.1.2.3，这样可以在排除网络路由的情况下用来测试IIS是否正常启动。

专用地址
IP地址空间中，有一些IP地址被定义为专用地址，这样的地址不能为Internet网络的设备分配，只能在企业内部使用，因此也称为私有地址。
若要在Internet网上使用这样的地址，必须使用网络地址转换或者端口映射技术。
这些专有地址是：
10/8 地址范围：10.0.0.0到10.255.255.255 共有2的24次方个地址
172.16/12 地址范围：172.16.0.0至172.31.255.255 共有2的20次方个地址
192.168/16 地址范围：192.168.0.0至192.168.255.255 共有2的16次方个地址
详细参考 ip百度百科 UDP百度百科
'''
