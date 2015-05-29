import socket, sys
import time
class MiniServer:
    h = ''
    p = ''
    m = ''
    c = int(0)
    d = int(0)
    def __init__(self, host, port, mode):
        self.h = host
        self.p = int(port)
        self.m = mode
    def serverT4(self):
        tcpT4Server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print "Server Socket Created......."
        tcpT4Server.bind((self.h, self.p))
        print "Wating for connecting......."
        tcpT4Server.listen(5)
        while True:
            clientSock, clientaddr = tcpT4Server.accept()
            print "Connected from: ", clientSock.getpeername()             
            while True:
                clientSock.send('Congratulations........')
                buf = clientSock.recv(1024)
                self.c = self.c + len(buf)
                self.d = self.d + len('Congratulations........')
                print "Received length = ", self.c, ",Sent length = ", self.d
               #clientSock.close()
    
    def udpT4(self):
        udpT4Server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print "UDP TCP IPv4 Mode Start....."
        udpT4Server.bind((self.h, self.p))
        print "UDP Server Start"
        while True:
            udpT4Server.sendto('Congratulations........',('192.168.1.32', self.p))
            udpT4Data, udpT4ServerInfo = udpT4Server.recvfrom(1024)
            print "Receive from ", udpT4ServerInfo, " and The Data send from The Client is :", udpT4Data
            self.c = self.c + len(udpT4Data)
            self.d = self.d + len('Congratulations........')
            print "Received length = ", self.c, ",Sent length = ", self.d

    def serverT6(self):
        tcpT6Server = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        print "Server Socket Created......."
        tcpT6Server.bind((self.h, self.p))
        print "Wating for connecting......."
        tcpT6Server.listen(5)
        while True:
            clientSock, clientaddr = tcpT6Server.accept()
            print "Connected from: ", clientSock.getpeername() 
            while True:
			clientSock.send('Congratulations........')
			buf = clientSock.recv(1024)
			self.c = self.c + len(buf)
			self.d = self.d + len('Congratulations........')
			print "Received length = ", self.c, ",Sent length = ", self.d
            #clientSock.close()
    def udpT6(self):
        udpT6Server = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        print "UDP IPv6 Mode Start....."
        udpT6Server.bind((self.h, self.p))
        print "UDP Server Start"
        while True:
            udpT6Server.sendto('Congratulations........',('fe80::1', 261))
            udpT4Data, udpT6ServerInfo = udpT6Server.recvfrom(1024)
            print "Receive from ", udpT6ServerInfo, " and The Data send from The Client is :", udpT4Data
            self.c = self.c + len(udpT4Data)
            #time.sleep(0.001)
            self.d = self.d + len('Congratulations........')
            print "Received length = ", self.c, ",Sent length = ", self.d

if __name__ == "__main__":
    x = MiniServer(sys.argv[1], sys.argv[2], sys.argv[3])
    if x.m == 't4':
        x.serverT4()
    elif x.m == 't6':
        x.serverT6()
    elif x.m == 'u4':
        x.udpT4()
    else:
        x.udpT6()
