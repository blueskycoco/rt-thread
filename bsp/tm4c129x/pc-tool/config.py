import serial  
import time
import sys
import binascii
config_local_ipv6 = [0xF5,0x8A,0x20,
		0x12,0x66,0x65,0x38,0x30,0x3a,0x3a,0x31,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0x26,0xfa,0x00,0x00]
config_remote_ipv6 = [0xF5,0x8A,0x0c,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0x26,0xfa,0x00,0x00]
config_local_ip 	= [0xF5,0x8A,0x00,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00]
config_local_port 	= [0xF5,0x8A,0x01,0xff,0xff,0x26,0xfa,0x00,0x00]
config_sub_msk 		= [0xF5,0x8A,0x05,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00]
config_gw 			= [0xF5,0x8A,0x06,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00]
config_mac 			= [0xF5,0x8A,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00]
config_remote_ip 	= [0xF5,0x8A,0x08,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00]
config_remote_port 	= [0xF5,0x8A,0x10,0xff,0xff,0x26,0xfa,0x00,0x00]
config_net_protol 	= [0xF5,0x8A,0x14,0xff,0x26,0xfa,0x00,0x00]
config_socket_mode 	= [0xF5,0x8A,0x18,0xff,0x26,0xfa,0x00,0x00]
config_uart_baud 	= [0xF5,0x8A,0x1c,0xff,0x26,0xfa,0x00,0x00]
config_tcp 			= [0xF5,0x8A,0x21,0xff,0x26,0xfa,0x00,0x00]
config_print		= [0xF5,0x8B,0x00]
def op_print():
	config_print[2] = int(sys.argv[3])
	t.write(config_print)
	print binascii.b2a_hex(t.read(int(sys.argv[4])))
def op_set_remote_ipv6():
	for i in range(0,len(sys.argv[4])):
#		config_remote_ipv6[i+3]=binascii.a2b_hex(binascii.b2a_hex(sys.argv[3])[2*i:2*i+2])
		config_remote_ipv6[i+4]=ord(sys.argv[4][i])
	config_remote_ipv6[3]=len(sys.argv[4])
	config_remote_ipv6[2]=0x0c+int(sys.argv[3])
	index1=0
	for i in range(0,len(config_remote_ipv6)-2):
		index1=int(config_remote_ipv6[i])+index1
	config_remote_ipv6[len(config_remote_ipv6)-2]=(index1>>8)&0xff
	config_remote_ipv6[len(config_remote_ipv6)-1]=(index1)&0xff
	print config_remote_ipv6
	t.write(config_remote_ipv6)
	print binascii.b2a_hex(t.read(int(3)))

def op_set_local_ipv6():
	for i in range(0,len(sys.argv[3])):
#		config_local_ipv6[i+3]=binascii.a2b_hex(binascii.b2a_hex(sys.argv[3])[2*i:2*i+2])
		config_local_ipv6[i+4]=ord(sys.argv[3][i])
	config_local_ipv6[3]=len(sys.argv[3])
	index1=0
	for i in range(0,len(config_local_ipv6)-2):
		index1=int(config_local_ipv6[i])+index1
	config_local_ipv6[len(config_local_ipv6)-2]=(index1>>8)&0xff
	config_local_ipv6[len(config_local_ipv6)-1]=(index1)&0xff
	print config_local_ipv6
	t.write(config_local_ipv6)
	print binascii.b2a_hex(t.read(int(3)))
def op_setbaud():
	config_uart_baud[2] = 0x1c+int(sys.argv[3])
	config_uart_baud[3] = int(sys.argv[4])
	index1=0
	for i in range(0,6):
		index1=int(config_uart_baud[i])+index1
	config_uart_baud[6]=(index1>>8)&0xff
	config_uart_baud[7]=(index1)&0xff
	t.write(config_uart_baud)
	print config_uart_baud
	print binascii.b2a_hex(t.read(int(3)))
def op_setmode():
	config_socket_mode[2] = 0x18+int(sys.argv[3])
	config_socket_mode[3] = int(sys.argv[4])
	index1=0
	for i in range(0,6):
		index1=int(config_socket_mode[i])+index1
	config_socket_mode[6]=(index1>>8)&0xff
	config_socket_mode[7]=(index1)&0xff
	t.write(config_socket_mode)
	print config_socket_mode
	print binascii.b2a_hex(t.read(int(3)))

def op_setprotol():
	config_net_protol[2] = 0x14+int(sys.argv[3])
	config_net_protol[3] = int(sys.argv[4])
	index1=0
	for i in range(0,6):
		index1=int(config_net_protol[i])+index1
	config_net_protol[6]=(index1>>8)&0xff
	config_net_protol[7]=(index1)&0xff
	t.write(config_net_protol)
	print config_net_protol
	print binascii.b2a_hex(t.read(int(3)))
def op_lport():
	config_local_port[2] = 0x01+int(sys.argv[3])
	config_local_port[3] = (int(sys.argv[4])>>8)&0xff
	config_local_port[4] = int(sys.argv[4])&0xff
	index1=0
	for i in range(0,7):
		index1=int(config_local_port[i])+index1
	config_local_port[7]=(index1>>8)&0xff
	config_local_port[8]=(index1)&0xff
	t.write(config_local_port)
	print config_local_port
	print binascii.b2a_hex(t.read(int(3)))
def op_rport():
	config_remote_port[2] = 0x10+int(sys.argv[3])
	config_remote_port[3] = (int(sys.argv[4])>>8)&0xff
	config_remote_port[4] = int(sys.argv[4])&0xff
	index1=0
	for i in range(0,7):
		index1=int(config_remote_port[i])+index1
	config_remote_port[7]=(index1>>8)&0xff
	config_remote_port[8]=(index1)&0xff
	t.write(config_remote_port)
	print config_remote_port
	print binascii.b2a_hex(t.read(int(3)))
def op_setmac():
	index1=0
	index2=sys.argv[3].find('-')
	config_mac[3]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('-',index2+1)
	config_mac[4]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('-',index2+1)
	config_mac[5]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('-',index2+1)
	config_mac[6]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('-',index2+1)
	config_mac[7]=int(sys.argv[3][index1:index2])
	index1=index2+1
	config_mac[8]=int(sys.argv[3][index1:len(sys.argv[3])])
	index1=0
	for i in range(0,11):
		index1=int(config_mac[i])+index1
	config_mac[11]=(index1>>8)&0xff
	config_mac[12]=(index1)&0xff
	t.write(config_mac)
	print config_mac
	print binascii.b2a_hex(t.read(int(3)))
def op_setgw():
	index1=0
	index2=sys.argv[3].find('.')
	config_gw[3]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_gw[4]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_gw[5]=int(sys.argv[3][index1:index2])
	index1=index2+1
	config_gw[6]=int(sys.argv[3][index1:len(sys.argv[3])])
	index1=0
	for i in range(0,9):
		index1=int(config_gw[i])+index1
	config_gw[9]=(index1>>8)&0xff
	config_gw[10]=(index1)&0xff
	t.write(config_gw)
	print config_gw
	print binascii.b2a_hex(t.read(int(3)))
def op_setsubmsk():
	index1=0
	index2=sys.argv[3].find('.')
	config_sub_msk[3]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_sub_msk[4]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_sub_msk[5]=int(sys.argv[3][index1:index2])
	index1=index2+1
	config_sub_msk[6]=int(sys.argv[3][index1:len(sys.argv[3])])
	index1=0
	for i in range(0,9):
		index1=int(config_sub_msk[i])+index1
	config_sub_msk[9]=(index1>>8)&0xff
	config_sub_msk[10]=(index1)&0xff
	t.write(config_sub_msk)
	print config_sub_msk
	print binascii.b2a_hex(t.read(int(3)))
def op_settcp():
	config_tcp[2] = 0x21+int(sys.argv[3])
	config_tcp[3] = int(sys.argv[4])
	index1=0
	for i in range(0,6):
		index1=int(config_tcp[i])+index1
	config_tcp[6]=(index1>>8)&0xff
	config_tcp[7]=(index1)&0xff
	t.write(config_tcp)
	print config_tcp
	print binascii.b2a_hex(t.read(int(3)))
def op_local_ip4addr():
	config_local_ip[2]=0x00
	index1=0
	index2=sys.argv[3].find('.')
	config_local_ip[3]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_local_ip[4]=int(sys.argv[3][index1:index2])
	index1=index2+1
	index2=sys.argv[3].find('.',index2+1)
	config_local_ip[5]=int(sys.argv[3][index1:index2])
	index1=index2+1
	config_local_ip[6]=int(sys.argv[3][index1:len(sys.argv[3])])
	config_local_ip[7]=0x26
	config_local_ip[8]=0xfa
	index1=0
	for i in range(0,9):
		index1=int(config_local_ip[i])+index1
	config_local_ip[9]=(index1>>8)&0xff
	config_local_ip[10]=(index1)&0xff
	print config_local_ip
	t.write(config_local_ip)
	print binascii.b2a_hex(t.read(3))
	
def op_remote_ip4addr():
	config_remote_ip[2]=0x08+int(sys.argv[3])
	index1=0
	index2=sys.argv[4].find('.')
	config_remote_ip[3]=int(sys.argv[4][index1:index2])
	index1=index2+1
	index2=sys.argv[4].find('.',index2+1)
	config_remote_ip[4]=int(sys.argv[4][index1:index2])
	index1=index2+1
	index2=sys.argv[4].find('.',index2+1)
	config_remote_ip[5]=int(sys.argv[4][index1:index2])
	index1=index2+1
	config_remote_ip[6]=int(sys.argv[4][index1:len(sys.argv[4])])
	config_remote_ip[7]=0x26
	config_remote_ip[8]=0xfa
	index1=0
	for i in range(0,9):
		index1=int(config_remote_ip[i])+index1
	config_remote_ip[9]=(index1>>8)&0xff
	config_remote_ip[10]=(index1)&0xff
	print config_remote_ip
	t.write(config_remote_ip)
	print binascii.b2a_hex(t.read(3))
	
operator = {'lipaddr':op_local_ip4addr,'ripaddr':op_remote_ip4addr,'read':op_print,'tcp':op_settcp,'submsk':op_setsubmsk,
			'gw':op_setgw,'mac':op_setmac,'rport':op_rport,'lport':op_lport,'protol':op_setprotol,'mode':op_setmode,
			'baud':op_setbaud,'lipv6':op_set_local_ipv6,'ripv6':op_set_remote_ipv6}  
  
def f(o):  
    operator.get(o)()
	
d = int(0)
t = serial.Serial(sys.argv[1],115200)
print "Excute ",sys.argv[2]
f(sys.argv[2])
t.close()
