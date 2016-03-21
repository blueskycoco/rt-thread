import serial  
import time
import sys

d = int(0)
fout = open(sys.argv[2],'rb')
t = serial.Serial(sys.argv[1],115200)
print "Open file ",sys.argv[2], "done........"
timeb=time.time()
print "Curr time ", timeb, " ", time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
while True:
	#time.sleep(1)
	out = fout.read(1022)
	if not out:
		fout.close()
		break
	n = t.write(out)
	d = d + len(out)
print "Send length = ", d , "time ", time.time(), " ", time.strftime("%Y-%m-%d %H:%M:%S",time.localtime())
print time.time()-timeb, "Second, Speed ", d/((time.time()-timeb)), "Bytes/s"
