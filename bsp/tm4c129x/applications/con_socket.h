#ifndef CON_SOCKET_H
#define CON_SOCKET_H
#include <rtthread.h>
#include <board.h>
#include <components.h>
#include <rtdevice.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

typedef struct {
	rt_uint8_t local_ip[16];
	rt_uint8_t local_ip6[64];
	rt_uint16_t local_port[4];
	rt_uint8_t sub_msk[16];
	rt_uint8_t gw[16];
	rt_uint8_t mac[64];
	rt_uint8_t remote_ip[4][16];
	rt_uint8_t remote_ip6[4][64];
	rt_uint16_t remote_port[4];
	rt_uint8_t config[4];//bit0 ipv4 or ipv6 	,bit1 tcp or udp , bit2 server or client ,bit 3 to bit 7 uart baud
}config,*pconfig;
typedef struct {
	rt_uint8_t lip6c;	//local ip6 addr changed
	rt_uint8_t rip4c;	//remote ip4 addr changed
	rt_uint8_t rip6c;	//remote ip6 addr changed
	rt_uint8_t lpc;		//local port changed
	rt_uint8_t rpc;		//remote port changed
	rt_uint8_t protol;  //ipv4 or ipv6 changed
	rt_uint8_t mode;    //tcp or udp changed
	rt_uint8_t cs;		//client or server
}change;
change g_chang[4];
config g_conf,g_confb;
#define CONFIG_IPV6 			0x01
#define CONFIG_TCP 				0x02
#define CONFIG_SERVER 			0x04
#define CONFIG_BAUD_115200 		0x08
#define CONFIG_BAUD_460800 		0x10
#define CONFIG_BAUD_921600 		0x20
#define CONFIG_BAUD_2000000 	0x40
#define CONFIG_BAUD_4000000 	0x80
#define CONFIG_BAUD_6000000 	0x88
#define DEV_UART 0
#define DEV_BUS 1
#define DEV_USB 2


void socket_init();
struct rt_data_queue *g_data_queue;
typedef struct socket_type
{
	struct sockaddr_in6 server_addr6;
	struct sockaddr_in6 client_addr6;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sockfd;
	int clientfd;
	char *recv_data;
	bool connected;
}socket_t,*psocket_t;

socket_t g_socket[4];
void cnn_out(int index,int level);
void socket_ctl(bool open,int i);

//void socket_send(int index,rt_uint8_t *data,int len);
#define debug 1
#if debug
#define DBG rt_kprintf
#else
#define DBG 
#endif
#endif
