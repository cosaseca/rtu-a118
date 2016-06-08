import threading
import time

class threadA(threading.Thread):
    def __init__(self, name="hello", sleepTime=1):
        threading.Thread.__init__(self)
        self.name = name
        self.sleepTime = sleepTime
    def run(self):
        while 1:
            print self.name
            time.sleep(self.sleepTime)
        
if __name__ == "__main__":
    a = threadA("hello A", 2)
    a.start()
    b = threadA()
    b.start()
    while 1:
        time.sleep(10)