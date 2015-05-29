import socket, sys
import time

class MiniClient:
    h = ''
    p = ''
    m = ''
    c = int(0)
    d = int(0)
    
    def __init__(self, host, port, mode):
        self.h = host
        self.p = int(port)
        self.m = mode
    def tcpC4(self):
        tcpT4Client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print "Done........"
        tcpT4Client.connect((self.h, self.p))
        print "TCP IPv4 TCP mode connecting..."
        while True:
            #time.sleep(1)
            tcpT4Client.send('sdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhg')
            buf = tcpT4Client.recv(1024)
            print "Receive  ", buf
            self.c = self.c + len(buf)
            self.d = self.d + len('sdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgsdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgfSDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhgfhgf123456789ssdadsadfASDSDAFSDFASFDASDFJHKLSAJDFLKJASLKDJFLKSAJDFLJSALKFasdfsafdsfdasfdJKLASDJLFgfjfjhgjhgjhgjghgfhg')
            print "Received length = ", self.c, ",Sent length = ", self.d, " "

    def udpC4(self):
        udpT4Client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print "UDP TCP IPv4 Mode connecting..."
        while True:
            udpT4Client.sendto("hello", (self.h, self.p))
            udpT4Data, udpT6ServerInfo = udpT4Client.recvfrom(1024)
            #print "Receive  ", udpT4Data
            self.c = self.c + len(udpT4Data)
            #time.sleep(0.0001)
            self.d = self.d + len("hello")
            print "Received length = ", self.c, "Sent length = ", self.d+5
                
    def tcpC6(self):
        tcpT4Client = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        print "Done........"
        tcpT4Client.connect((self.h, self.p))
        print "TCP IPv6 TCP mode connecting..."
        while True:
            tcpT4Client.send('hello')
            self.d = self.d + len('hello')
            buf = tcpT4Client.recv(1024)
            print "Receive  ", buf
            self.c = self.c + len(buf)
            print "Received length = ", self.c, "Sent length = ", self.d
        
    def udpC6(self):
        udpU6Client = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        udpU6Client.bind(('fe80::5867:8730:e9e6:d5c5%11', self.p))
        print "UDP TCP IPv6 Mode connecting..."
        while True:
            udpT4Data, udpT6ServerInfo = udpU6Client.recvfrom(1024)			
            udpU6Client.sendto("hello", (self.h, self.p))
            self.d = self.d + len('hello')
            #time.sleep(0.0001)
            print "Receive  ", udpT4Data
            self.c = self.c + len(udpT4Data)
            print "Received length = ", self.c, "Sent length = ", self.d
            
if __name__ == "__main__":
    x = MiniClient(sys.argv[1], sys.argv[2], sys.argv[3])
    if x.m == 't4':
        x.tcpC4()
    elif x.m == 't6':
        x.tcpC6()
    elif x.m == 'u4':
        x.udpC4()
    else:
        x.udpC6()
