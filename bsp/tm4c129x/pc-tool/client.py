import socket, sys
import time

class MiniClient:
    h = ''
    p = ''
    m = ''
    file_out = ''
    file_in = ''
    c = int(0)
    d = int(0)
    
    def __init__(self, host, port, mode, file_out):
        self.h = host
        self.p = int(port)
        self.m = mode
        self.file_out = file_out
    def tcpC4(self):
        tcpT4Client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        fout = open(self.file_out,'rb')
        print "Open file ",self.file_out, "done........"
        tcpT4Client.connect((self.h, self.p))
        print "TCP IPv4 TCP mode connecting..."
        timeb=time.time()
        print "Curr time ", timeb, " ", time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
        while True:
            #time.sleep(1)
            out = fout.read(1022)
            if not out:
				fout.close()
                #print "Received length = ", self.c, ",Sent length = ", self.d, " "
				break
            tcpT4Client.send(out)
            self.d = self.d + len(out)
            #print "Sent ", self.d
        print "End  time ", time.time(), " ", time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
        print "Sent length = ", self.d
        print time.time()-timeb, "Second, Speed ", self.d/((time.time()-timeb)*1024*1024), "MBytes/s"

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
        fout = open(self.file_out,'rb')
        print "Open file ",self.file_out, "done........"
        tcpT4Client.connect((self.h, self.p))
        print "TCP IPv6 TCP mode connecting..."
        timeb=time.time()
        print "Curr time ", timeb
        while True:
            #time.sleep(1)
            out = fout.read(1022)
            if not out:
				fout.close()
                #print "Received length = ", self.c, ",Sent length = ", self.d, " "
				break
            tcpT4Client.send(out)
            self.d = self.d + len(out)
        print "Received length = ", self.c, ",Sent length = ", self.d, "time ", time.time()
        print time.time()-timeb, "Second, Speed ", self.d/((time.time()-timeb)*1024*1024), "MBytes/s"
        
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
    x = MiniClient(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    if x.m == 't4':
        x.tcpC4()
    elif x.m == 't6':
        x.tcpC6()
    elif x.m == 'u4':
        x.udpC4()
    else:
        x.udpC6()
