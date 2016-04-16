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
    def serverT6(self):
        tcpT6Server = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        print "Server Socket Created......."
        fout = open(self.file_out,'wb')
        print "Open file ",self.file_out, "done........"
        tcpT6Server.bind((self.h, self.p))
        print "TCP IPv6 Server Mode, Wating for connecting......."
        tcpT6Server.listen(5)
        while True:
            clientSock, clientaddr = tcpT6Server.accept()
            print "Connected from: ", clientSock.getpeername() 
            while True:
		buf = clientSock.recv(1430)
		self.c = self.c + len(buf)
            	print "Received length = ", self.c
            	fout.write(buf)
            	fout.flush()
    def tcpC6(self):
        tcpT4Client = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        fout = open(self.file_out,'wb')
        print "Open file ",self.file_out, "done........"
        tcpT4Client.connect((self.h, self.p))
        print "TCP IPv6 Client Mode, connected to server,waiting from data ..."
        timeb=time.time()
        print "Curr time ", timeb
        while True:
            buf = tcpT4Client.recv(1430)
            self.c = self.c + len(buf)
            print "Received length = ", self.c
            fout.write(buf)
            fout.flush()
        print "Received length = ", self.c, ",Sent length = ", self.d, "time ", time.time()
        print time.time()-timeb, "Second, Speed ", self.d/((time.time()-timeb)*1024*1024), "MBytes/s"
        
    def udpC6(self):
        udpU6Client = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        udpU6Client.bind((self.h, self.p))
        print "UDP IPv6 Mode connected,waiting from data ..."
        while True:
            udpT4Data, udpT6ServerInfo = udpU6Client.recvfrom(1430)
            self.c = self.c + len(udpT4Data)
            print "Received length = ", self.c
            
if __name__ == "__main__":
    x = MiniClient(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    if x.m == 's6':
        x.serverT6()
    elif x.m == 'c6':
        x.tcpC6()
    else:
        x.udpC6()
