#include "con_socket.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#define CONFIG_BIN 1 //test socket 1
#define CONFIG_IT 0 //test  config 1

#if 0
void IntGpioD()
{
	if(MAP_GPIOIntStatus(GPIO_PORTD_BASE, true)&GPIO_PIN_2)
	{			
		MAP_GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
		if(MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)!=GPIO_PIN_2)
			MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);
		else
			MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);
		//ind[3]=((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))==0)?RT_TRUE:RT_FALSE;
		rt_kprintf("gpiod 2 int \r\n");
	}	
}

#else
unsigned char config_local_ip[4+8]				={0xF5,0x8A,0x00,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip*/
unsigned char config_socket0_local_port[2+8]	={0xF5,0x8A,0x01,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port0*/
unsigned char config_socket1_local_port[2+8]	={0xF5,0x8A,0x02,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port1*/
unsigned char config_socket2_local_port[2+8]	={0xF5,0x8A,0x03,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port2*/
unsigned char config_socket3_local_port[2+8]	={0xF5,0x8A,0x04,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port3*/
unsigned char config_sub_msk[4+8]				={0xF5,0x8A,0x05,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*sub msk*/
unsigned char config_gw[4+8]					={0xF5,0x8A,0x06,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*gw*/
unsigned char config_mac[6+8]					={0xF5,0x8A,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*mac*/
unsigned char config_socket0_ip[4+8]			={0xF5,0x8A,0x08,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip*/
unsigned char config_socket1_ip[4+8]			={0xF5,0x8A,0x09,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip*/
unsigned char config_socket2_ip[4+8]			={0xF5,0x8A,0x0a,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip*/
unsigned char config_socket3_ip[4+8]			={0xF5,0x8A,0x0b,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip*/
unsigned char config_socket0_ip6[64+8]			={0xF5,0x8A,0x0c,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip6*/
unsigned char config_socket1_ip6[64+8]			={0xF5,0x8A,0x0d,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip6*/
unsigned char config_socket2_ip6[64+8]			={0xF5,0x8A,0x0e,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip6*/
unsigned char config_socket3_ip6[64+8]			={0xF5,0x8A,0x0f,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip6*/
unsigned char config_socket0_remote_port[2+8]	={0xF5,0x8A,0x10,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 port*/
unsigned char config_socket1_remote_port[2+8]	={0xF5,0x8A,0x11,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 port*/
unsigned char config_socket2_remote_port[2+8]	={0xF5,0x8A,0x12,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 port*/
unsigned char config_socket3_remote_port[2+8]	={0xF5,0x8A,0x13,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 port*/
unsigned char config_net_protol0[1+8]			={0xF5,0x8A,0x14,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
unsigned char config_net_protol1[1+8]			={0xF5,0x8A,0x15,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
unsigned char config_net_protol2[1+8]			={0xF5,0x8A,0x16,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
unsigned char config_net_protol3[1+8]			={0xF5,0x8A,0x17,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
unsigned char config_socket_mode0[1+8]			={0xF5,0x8A,0x18,0xff,0x26,0xfa,0x00,0x00};/*server mode0*/
unsigned char config_socket_mode1[1+8]			={0xF5,0x8A,0x19,0xff,0x26,0xfa,0x00,0x00};/*server mode1*/
unsigned char config_socket_mode2[1+8]			={0xF5,0x8A,0x1a,0xff,0x26,0xfa,0x00,0x00};/*server mode2*/
unsigned char config_socket_mode3[1+8]			={0xF5,0x8A,0x1b,0xff,0x26,0xfa,0x00,0x00};/*server mode3*/
unsigned char config_uart_baud0[1+8]			={0xF5,0x8A,0x1c,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
unsigned char config_uart_baud1[1+8]			={0xF5,0x8A,0x1d,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
unsigned char config_uart_baud2[1+8]			={0xF5,0x8A,0x1e,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
unsigned char config_uart_baud3[1+8]			={0xF5,0x8A,0x1f,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
unsigned char config_local_ip6[64+8]			={0xF5,0x8A,0x20,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip6*/
unsigned char config_tcp0[1+8]					={0xF5,0x8A,0x21,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
unsigned char config_tcp1[1+8]					={0xF5,0x8A,0x22,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
unsigned char config_tcp2[1+8]					={0xF5,0x8A,0x23,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
unsigned char config_tcp3[1+8]					={0xF5,0x8A,0x24,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
struct rt_mutex mconfigutex;




int times=0;


#define COMMAND_FAIL "Command crc fail"
#define COMMAND_OK "Command exec OK"

void print_config(config g);

unsigned char get_config[35];//			={0xF5,0x8B,0x0f,0xff,0xff,0xff,0xff,0x27,0xfa,0x00,0x00};/*get config 0xf5,0x8b,********0x27,0xfa,0x00,0x00*/
rt_thread_t tid_common_w[4]={RT_NULL,RT_NULL,RT_NULL,RT_NULL},tid_common_r[4]={RT_NULL,RT_NULL,RT_NULL,RT_NULL};
rt_device_t common_dev[4] = {RT_NULL,RT_NULL,RT_NULL,RT_NULL};
bool ind[4]={RT_TRUE,RT_TRUE,RT_TRUE,RT_TRUE};
bool phy_link=false;
enum STATE_OP{
	GET_F5,
	GET_8A_8B,
	GET_LEN,
	GET_DATA,
	GET_26,
	GET_FA,
	GET_CHECSUM
};
struct rt_semaphore rx_sem[4];
struct rt_semaphore usbrx_sem[4];

void configlock()
{
    rt_err_t result;

    result = rt_mutex_take(&mconfigutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        RT_ASSERT(0);
    }
}
void configunlock()
{
    rt_mutex_release(&mconfigutex);
}

int which_common_dev(rt_device_t *dev,rt_device_t dev2)
{
	int i=0;
	for(i=0;i<4;i++)
		if(dev[i]==dev2)
		{
			//DBG("Uart %d 's setting\r\n",i);
			break;
		}
	return i;
}
void IntGpioD()
{
	if(MAP_GPIOIntStatus(GPIO_PORTD_BASE, true)&GPIO_PIN_2)
	{		
		MAP_GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
		ind[0]=RT_TRUE;//((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))==GPIO_PIN_2)?RT_TRUE:RT_FALSE;
		ind[1]=RT_TRUE;
		ind[2]=RT_TRUE;
		ind[3]=RT_TRUE;
		rt_kprintf("gpiod 2 int %d\r\n",ind[0]);
		
	}	
}/*
void IntGpioB()
{
	if(MAP_GPIOIntStatus(GPIO_PORTB_BASE, true)&GPIO_PIN_4)
	{
		MAP_GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_4);
		ind[3]=((MAP_GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4)&(GPIO_PIN_4))==0)?RT_TRUE:RT_FALSE;
		rt_kprintf("gpiob 4 int %d\r\n",ind[3]);
	}	
}
*/
/*get config data to global config zone, or get socket data to buffer*/
void default_config()
{
	struct netif * netif=netif_list;
	if(((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))==GPIO_PIN_2)?RT_TRUE:RT_FALSE)
	{//server mode high level
		g_conf.config[0]=CONFIG_TCP|CONFIG_SERVER;
		g_conf.config[1]=CONFIG_TCP|CONFIG_SERVER;
		g_conf.config[2]=CONFIG_TCP|CONFIG_SERVER;
		g_conf.config[3]=CONFIG_TCP|CONFIG_SERVER;
		memset(g_conf.remote_ip[0],'\0',16);
		strcpy(g_conf.remote_ip[0],"192.168.2.6");
		memset(g_conf.remote_ip[1],'\0',16);
		strcpy(g_conf.remote_ip[1],"192.168.2.6");
		memset(g_conf.remote_ip[2],'\0',16);
		strcpy(g_conf.remote_ip[2],"192.168.2.6");
		memset(g_conf.remote_ip[3],'\0',16);
		strcpy(g_conf.remote_ip[3],"192.168.2.6");
		memset(g_conf.local_ip,'\0',16);
		strcpy(g_conf.local_ip,"192.168.2.32");	
	}
	else
	{//client mode
		g_conf.config[0]=CONFIG_TCP;//|CONFIG_SERVER;
		g_conf.config[1]=CONFIG_TCP;//|CONFIG_SERVER;
		g_conf.config[2]=CONFIG_TCP;//|CONFIG_SERVER;
		g_conf.config[3]=CONFIG_TCP;//|CONFIG_SERVER;
		memset(g_conf.remote_ip[0],'\0',16);
		strcpy(g_conf.remote_ip[0],"192.168.2.32");
		memset(g_conf.remote_ip[1],'\0',16);
		strcpy(g_conf.remote_ip[1],"192.168.2.32");
		memset(g_conf.remote_ip[2],'\0',16);
		strcpy(g_conf.remote_ip[2],"192.168.2.32");
		memset(g_conf.remote_ip[3],'\0',16);
		strcpy(g_conf.remote_ip[3],"192.168.2.32");
		memset(g_conf.local_ip,'\0',16);
		strcpy(g_conf.local_ip,"192.168.2.6");	
	}
	
	memset(g_conf.remote_ip6[0],'\0',64);
	strcpy(g_conf.remote_ip6[0],"fe80::5867:8730:e9e6:d5c5%11");
	memset(g_conf.remote_ip6[1],'\0',64);
	strcpy(g_conf.remote_ip6[1],"fe80::5867:8730:e9e6:d5c5%11");
	memset(g_conf.remote_ip6[2],'\0',64);
	strcpy(g_conf.remote_ip6[2],"fe80::5867:8730:e9e6:d5c5%11");
	memset(g_conf.remote_ip6[3],'\0',64);
	strcpy(g_conf.remote_ip6[3],"fe80::5867:8730:e9e6:d5c5%11");
	g_conf.local_port[0]=1234;
	g_conf.local_port[1]=1235;
	g_conf.local_port[2]=1236;
	g_conf.local_port[3]=1237;
	
	
	memset(g_conf.local_ip6,'\0',64);
	strcpy(g_conf.local_ip6,"fe80::1");
	
	memset(g_conf.gw,'\0',16);
	strcpy(g_conf.gw,"192.168.2.1");	
	memset(g_conf.sub_msk,'\0',16);
	strcpy(g_conf.sub_msk,"255.255.255.0");
	memset(g_conf.mac,'\0',64);	
	rt_sprintf(g_conf.mac,"%d.%d.%d.%d.%d.%d",netif->hwaddr[0],netif->hwaddr[1],netif->hwaddr[2],netif->hwaddr[3],netif->hwaddr[4],netif->hwaddr[5]);
	g_conf.remote_port[0]=1234;
	g_conf.remote_port[1]=1235;
	g_conf.remote_port[2]=1236;
	g_conf.remote_port[3]=1237;
	set_if6("e0","fe80::1");
	set_if("e0",g_conf.local_ip,g_conf.gw,g_conf.sub_msk);
	
}
#if CONFIG_BIN
void set_config(rt_uint8_t *data,int ipv6_len,int dev)//0 no change ,1 local socket need reconfig ,2 all socket need reconfig
{
	
	bool ipv6_changed=false,ipv4_changed=false;
	int i;
	switch(data[0])
	{
		case 0:
		{
			//set local ipv4 ip
			rt_uint8_t *tmp=rt_malloc(16);
			rt_memset(tmp,'\0',16);
			rt_sprintf(tmp,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			if(memcmp(tmp,g_conf.local_ip,16)!=0)
			{
				ipv4_changed=true;
				rt_memset(g_conf.local_ip,'\0',16);
				rt_sprintf(g_conf.local_ip,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			}
			rt_free(tmp);
		}
		break;
		case 1:
		case 2:
		case 3:
		case 4:
		{
			//set local port
			if((data[0]-1)==dev)
			{
				if(g_conf.local_port[data[0]-1]!=(data[1]<<8|data[2]))
				{
					g_conf.local_port[data[0]-1]=(data[1]<<8|data[2]);
					g_chang[dev].lpc=1;
					rt_kprintf("local port changed %d\n",dev);
				}
			}
		}
		break;
		case 5:
		{
			//set sub msk
			rt_uint8_t *tmp=rt_malloc(16);
			rt_memset(tmp,'\0',16);
			rt_sprintf(tmp,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			if(memcmp(tmp,g_conf.sub_msk,16)!=0)
			{
				ipv4_changed=true;
				rt_memset(g_conf.sub_msk,'\0',16);
				rt_sprintf(g_conf.sub_msk,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			}
			rt_free(tmp);
		}
		break;
		case 6:
		{
			//set gateway
			rt_uint8_t *tmp=rt_malloc(16);
			rt_memset(tmp,'\0',16);
			rt_sprintf(tmp,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			if(memcmp(tmp,g_conf.gw,16)!=0)
			{
				ipv4_changed=true;
				rt_memset(g_conf.gw,'\0',16);
				rt_sprintf(g_conf.gw,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
			}
			rt_free(tmp);
		}
		break;
		case 7:
		{
			//set mac
			
		}
		break;
		case 8:
		case 9:
		case 10:
		case 11:
		{
			//set remote ipv4 ip
			if((data[0]-8)==dev)
			{
				rt_uint8_t *tmp=rt_malloc(16);
				rt_memset(tmp,'\0',16);
				rt_sprintf(tmp,"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
				if(memcmp(tmp,g_conf.remote_ip[data[0]-8],16)!=0)
				{
					rt_memset(g_conf.remote_ip[data[0]-8],'\0',16);
					rt_sprintf(g_conf.remote_ip[data[0]-8],"%d.%d.%d.%d",data[1],data[2],data[3],data[4]);
					g_chang[dev].rip4c=1;
				}
				rt_free(tmp);
			}
		}
		break;
		case 12:
		case 13:			
		case 14:
		case 15:
		{
			//set remote ipv6 ip
			if((data[0]-12)==dev)
			{
				rt_uint8_t *tmp=rt_malloc(ipv6_len);
				rt_memset(tmp,'\0',ipv6_len);
				for(i=0;i<ipv6_len;i++)
					tmp[i]=data[i+1];
				if(memcmp(tmp,g_conf.remote_ip6[data[0]-12],ipv6_len)!=0 || ipv6_len!=sizeof(g_conf.remote_ip6[data[0]-12]))
				{
					rt_memset(g_conf.remote_ip6[data[0]-12],'\0',64);
					for(i=0;i<ipv6_len;i++)
						g_conf.remote_ip6[data[0]-12][i]=data[i+1];
					g_chang[dev].rip6c=1;
				}
				rt_free(tmp);
			}
		}
		break;
		case 16:
		case 17:
		case 18:
		case 19:
		{
			//set remote port
			if((data[0]-16)==dev)
			{
				if(g_conf.remote_port[data[0]-16]!=(data[1]<<8|data[2]))
				{
					g_conf.remote_port[data[0]-16]=(data[1]<<8|data[2]);
					g_chang[dev].rpc=1;
				}
			}
		}
		break;
		case 20:
		case 21:
		case 22:
		case 23:
		{
			//set net protol ipv4 or ipv6
			if((data[0]-20)==dev)
			{
				if(data[1])
				{	
					if((g_conf.config[data[0]-20]&CONFIG_IPV6)!=CONFIG_IPV6)
					{
						g_conf.config[data[0]-20]|=CONFIG_IPV6;
						g_chang[dev].protol=1;
					}
				}
				else
				{
					if((g_conf.config[data[0]-20]&CONFIG_IPV6)==CONFIG_IPV6)
					{
						g_conf.config[data[0]-20]&=~CONFIG_IPV6;
						g_chang[dev].protol=1;
					}
				}
			}
		}
		break;
		case 24:
		case 25:
		case 26:
		case 27:
		{
			//set server or client mode
			if((data[0]-24)==dev)
			{
				if(data[1])
				{
					if((g_conf.config[data[0]-24]&CONFIG_SERVER)!=CONFIG_SERVER)
					{
						g_conf.config[data[0]-24]|=CONFIG_SERVER;
						g_chang[dev].cs=1;
					}
				}
				else
				{
					if((g_conf.config[data[0]-24]&CONFIG_SERVER)==CONFIG_SERVER)
					{
						g_conf.config[data[0]-24]&=~CONFIG_SERVER;
						g_chang[dev].cs=1;
					}
				}
			}
		}
		break;
		case 28:
		case 29:
		case 30:
		case 31:
		{
			//set uart baud
			if((data[0]-28)==dev)
			{
				g_conf.config[data[0]-28]=((g_conf.config[data[0]-28]&0x07)|(data[1]<<3));
				struct serial_configure config;
				config.baud_rate=baud(g_conf.config[data[0]-28]>>3);
				config.bit_order = BIT_ORDER_LSB;
				config.data_bits = DATA_BITS_8;
				config.parity	 = PARITY_NONE;
				config.stop_bits = STOP_BITS_1;
				config.invert	 = NRZ_NORMAL;
				config.bufsz	 = RT_SERIAL_RB_BUFSZ;
				rt_device_control(common_dev[dev],RT_DEVICE_CTRL_CONFIG,&config);	
			}
		}
		break;
		case 32:
		{
			//set local ipv6 ip
			rt_uint8_t *tmp=rt_malloc(ipv6_len);
			rt_memset(tmp,'\0',ipv6_len);
			for(i=0;i<ipv6_len;i++)
				tmp[i]=data[i+1];
			if(memcmp(tmp,g_conf.local_ip6,ipv6_len)!=0 || ipv6_len!=sizeof(g_conf.local_ip6))
			{
				ipv6_changed=true;
				rt_memset(g_conf.local_ip6,'\0',64);
				for(i=0;i<ipv6_len;i++)
					g_conf.local_ip6[i]=data[i+1];
				g_chang[dev].lip6c=1;
			}
			rt_free(tmp);
		}
		break;
		case 33:
		case 34:
		case 35:
		case 36:
		{//set tcp or udp
			if((data[0]-33)==dev)
			{
				if(data[1])
				{
					if((g_conf.config[data[0]-33]&CONFIG_TCP)!=CONFIG_TCP)
					{
						g_conf.config[data[0]-33]|=CONFIG_TCP;
						g_chang[dev].mode=1;
					}
				}
				else
				{
					if((g_conf.config[data[0]-33]&CONFIG_TCP)==CONFIG_TCP)
					{					
						g_conf.config[data[0]-33]&=~CONFIG_TCP;
						g_chang[dev].mode=1;
					}
				}
			}
		}
		break;
		default:
			rt_kprintf("wrong cmd\n");
	}
	if(ipv4_changed)
		set_if("e0",g_conf.local_ip,g_conf.gw,g_conf.sub_msk);
	if(ipv6_changed)
		set_if6("e0",g_conf.local_ip6);
}
#endif
bool need_reconfig(int dev)
{
#if CONFIG_BIN
	if(g_chang[dev].cs||g_chang[dev].lip6c||g_chang[dev].lpc||g_chang[dev].mode||
	   g_chang[dev].protol||g_chang[dev].rip4c||g_chang[dev].rip6c||g_chang[dev].rpc)
	{
		if(g_chang[dev].cs)
			rt_kprintf("%d client/server changed\n",dev);
		if(g_chang[dev].lip6c)
			rt_kprintf("%d local ip6 changed\n",dev);
		if(g_chang[dev].lpc)
			rt_kprintf("%d local port changed\n",dev);
		if(g_chang[dev].mode)
			rt_kprintf("%d tcp/udp changed\n",dev);
		if(g_chang[dev].protol)
			rt_kprintf("%d ipv4/ipv6 changed\n",dev);
		if(g_chang[dev].rip4c)
			rt_kprintf("%d remote ip4 changed\n",dev);
		if(g_chang[dev].rip6c)
			rt_kprintf("%d remote ip6 changed\n",dev);
		if(g_chang[dev].rpc)
			rt_kprintf("%d remote port changed\n",dev);
		//rt_uint8_t *ptr=(rt_uint8_t *)(&g_chang[dev]);
		//rt_memset(ptr,0,sizeof(change));
		g_chang[dev].cs=g_chang[dev].lip6c=g_chang[dev].lpc=g_chang[dev].mode=g_chang[dev].protol=g_chang[dev].rip4c=g_chang[dev].rip6c=g_chang[dev].rpc=0;
		return true;
	}
	else
		return false;
#else
	return false;
#endif
}
#if CONFIG_BIN
char *send_out(int dev,int cmd,int *lenout)
{
	#if 0
	unsigned char config_local_ip[4+7]				={0xF5,0x8A,0x00,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip*/
	unsigned char config_socket0_local_port[2+7]	={0xF5,0x8A,0x01,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port0*/
	unsigned char config_socket1_local_port[2+7]	={0xF5,0x8A,0x02,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port1*/
	unsigned char config_socket2_local_port[2+7]	={0xF5,0x8A,0x03,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port2*/
	unsigned char config_socket3_local_port[2+7]	={0xF5,0x8A,0x04,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port3*/
	unsigned char config_sub_msk[4+7]				={0xF5,0x8A,0x05,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*sub msk*/
	unsigned char config_gw[4+7]					={0xF5,0x8A,0x06,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*gw*/
	unsigned char config_mac[6+7]					={0xF5,0x8A,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*mac*/
	unsigned char config_socket0_ip[4+7]			={0xF5,0x8A,0x08,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip*/
	unsigned char config_socket1_ip[4+7]			={0xF5,0x8A,0x09,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip*/
	unsigned char config_socket2_ip[4+7]			={0xF5,0x8A,0x0a,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip*/
	unsigned char config_socket3_ip[4+7]			={0xF5,0x8A,0x0b,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip*/
	unsigned char config_socket0_ip6[64+7]			={0xF5,0x8A,0x0c,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip6*/
	unsigned char config_socket1_ip6[64+7]			={0xF5,0x8A,0x0d,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip6*/
	unsigned char config_socket2_ip6[64+7]			={0xF5,0x8A,0x0e,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip6*/
	unsigned char config_socket3_ip6[64+7]			={0xF5,0x8A,0x0f,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip6*/
	unsigned char config_socket0_remote_port[2+7]	={0xF5,0x8A,0x10,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 port*/
	unsigned char config_socket1_remote_port[2+7]	={0xF5,0x8A,0x11,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 port*/
	unsigned char config_socket2_remote_port[2+7]	={0xF5,0x8A,0x12,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 port*/
	unsigned char config_socket3_remote_port[2+7]	={0xF5,0x8A,0x13,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 port*/
	unsigned char config_net_protol0[1+7]			={0xF5,0x8A,0x14,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
	unsigned char config_net_protol1[1+7]			={0xF5,0x8A,0x15,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
	unsigned char config_net_protol2[1+7]			={0xF5,0x8A,0x16,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
	unsigned char config_net_protol3[1+7]			={0xF5,0x8A,0x17,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
	unsigned char config_socket_mode0[1+7]			={0xF5,0x8A,0x18,0xff,0x26,0xfa,0x00,0x00};/*server mode0*/
	unsigned char config_socket_mode1[1+7]			={0xF5,0x8A,0x19,0xff,0x26,0xfa,0x00,0x00};/*server mode1*/
	unsigned char config_socket_mode2[1+7]			={0xF5,0x8A,0x1a,0xff,0x26,0xfa,0x00,0x00};/*server mode2*/
	unsigned char config_socket_mode3[1+7]			={0xF5,0x8A,0x1b,0xff,0x26,0xfa,0x00,0x00};/*server mode3*/
	unsigned char config_uart_baud0[1+7]			={0xF5,0x8A,0x1c,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
	unsigned char config_uart_baud1[1+7]			={0xF5,0x8A,0x1d,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
	unsigned char config_uart_baud2[1+7]			={0xF5,0x8A,0x1e,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
	unsigned char config_uart_baud3[1+7]			={0xF5,0x8A,0x1f,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
	unsigned char config_local_ip6[64+7]			={0xF5,0x8A,0x20,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip6*/
	unsigned char config_tcp0[1+7]					={0xF5,0x8A,0x21,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
	unsigned char config_tcp1[1+7]					={0xF5,0x8A,0x22,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
	unsigned char config_tcp2[1+7]					={0xF5,0x8A,0x23,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
	unsigned char config_tcp3[1+7]					={0xF5,0x8A,0x24,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
	#endif
	char *ptr=NULL;
	int crc=0,i=0,len;
	char *p=NULL;
	switch(cmd)
	{
		case 0:
			{
				ptr=config_local_ip;
				p=g_conf.local_ip;
				ptr[3]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[4]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[5]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[6]=atoi(p);
				len=4;
			}
			break;
		case 1:
			{
				if(dev==(cmd-1))
				ptr=config_socket0_local_port;
			}
		case 2:
			{
				if(dev==(cmd-1))
				ptr=config_socket1_local_port;
			}
		case 3:
			{
				if(dev==(cmd-1))
				ptr=config_socket2_local_port;
			}
		case 4:
			{
				if(dev==(cmd-1))
					ptr=config_socket3_local_port;
				if(ptr!=NULL)
				{
					ptr[3]=(g_conf.local_port[cmd-1]>>8)&0xff;
					ptr[4]=g_conf.local_port[cmd-1]&0xff;
					len=2;
				}				
			}			
			break;
		case 5:
			{
				ptr=config_sub_msk;
				p=g_conf.sub_msk;
				ptr[3]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[4]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[5]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[6]=atoi(p);
				len=4;
			}
			break;
		case 6:
			{
				ptr=config_gw;
				p=g_conf.gw;
				ptr[3]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[4]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[5]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[6]=atoi(p);
				len=4;
			}
			break;
		case 7:
			{
				ptr=config_mac;				
				p=g_conf.mac;
				ptr[3]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[4]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[5]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[6]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[7]=atoi(p);
				while(*p!='.')
					p++;
				p++;
				ptr[8]=atoi(p);
				len=6;
			}
			break;	
		case 8:
			if(dev==(cmd-8))				
			ptr=config_socket0_ip;
		case 9:
			if(dev==(cmd-8))
			ptr=config_socket1_ip;
		case 10:
			if(dev==(cmd-8))
			ptr=config_socket2_ip;
		case 11:
			{
				if(dev==(cmd-8))
				ptr=config_socket3_ip;
				if(ptr!=NULL)
				{
					p=g_conf.remote_ip[cmd-8];
					ptr[3]=atoi(p);
					while(*p!='.')
						p++;
					p++;
					ptr[4]=atoi(p);
					while(*p!='.')
						p++;
					p++;
					ptr[5]=atoi(p);
					while(*p!='.')
						p++;
					p++;
					ptr[6]=atoi(p);
					len=4;
				}
			}
			break;
		case 12:
			if(dev==(cmd-12))
			ptr=config_socket0_ip6;
		case 13:
			if(dev==(cmd-12))
			ptr=config_socket1_ip6;
		case 14:
			if(dev==(cmd-12))
			ptr=config_socket2_ip6;
		case 15:
			{
				if(dev==(cmd-12))
				ptr=config_socket3_ip6;
				if(ptr!=NULL)
				{
					p=g_conf.remote_ip6[cmd-12];
					for(i=0;i<sizeof(g_conf.remote_ip6[cmd-12]);i++)
						ptr[3+i]=p[i];
					len=64;
				}
			}
			break;
		case 16:
			if(dev==(cmd-16))
			ptr=config_socket0_remote_port;
		case 17:
			if(dev==(cmd-16))
			ptr=config_socket1_remote_port;
		case 18:
			if(dev==(cmd-16))
			ptr=config_socket2_remote_port;
		case 19:
			{
				if(dev==(cmd-16))
					ptr=config_socket3_remote_port;
				if(ptr!=NULL)
				{
					ptr[3]=(g_conf.remote_port[cmd-16]>>8)&0xff;
					ptr[4]=g_conf.remote_port[cmd-16]&0xff;
					len=2;
				}
			}
			break;
		case 20:
			if(dev==(cmd-20))
			ptr=config_net_protol0;
		case 21:
			if(dev==(cmd-20))
			ptr=config_net_protol1;
		case 22:
			if(dev==(cmd-20))
			ptr=config_net_protol2;
		case 23:
			{
				if(dev==(cmd-20))
				ptr=config_net_protol3;
				if(ptr!=NULL)
				{
					ptr[3]=(g_conf.config[cmd-20]&CONFIG_IPV6)?1:0;
					len=1;
				}
			}
			break;
		case 24:
			if(dev==(cmd-24))
			ptr=config_socket_mode0;
		case 25:
			if(dev==(cmd-24))
			ptr=config_socket_mode1;
		case 26:
			if(dev==(cmd-24))
			ptr=config_socket_mode2;
		case 27:
			{
				if(dev==(cmd-24))
				ptr=config_socket_mode3;
				if(ptr!=NULL)
				{
					ptr[3]=(g_conf.config[cmd-24]&CONFIG_SERVER)?1:0;
					len=1;
				}
			}
			break;
		case 28:
			if(dev==(cmd-28))
			ptr=config_uart_baud0;
		case 29:
			if(dev==(cmd-28))
			ptr=config_uart_baud1;
		case 30:
			if(dev==(cmd-28))
			ptr=config_uart_baud2;
		case 31:
			{
				if(dev==(cmd-28))
				ptr=config_uart_baud3;
				if(ptr!=NULL)
				{
					ptr[3]=g_conf.config[cmd-28]>>3;
					len=1;
				}
			}
			break;
		case 32:
			{
				ptr=config_local_ip6;
				if(ptr!=NULL)
				{
					p=g_conf.local_ip6;
					for(i=0;i<sizeof(g_conf.local_ip6);i++)
						ptr[3+i]=p[i];
					len=64;
				}
			}
			break;
		case 33:
			if(dev==(cmd-33))
			ptr=config_tcp0;
		case 34:
			if(dev==(cmd-33))
			ptr=config_tcp1;
		case 35:
			if(dev==(cmd-33))
			ptr=config_tcp2;
		case 36:
			{
				if(dev==(cmd-33))
				ptr=config_tcp3;
				if(ptr!=NULL)
				{
					ptr[3]=(g_conf.config[cmd-33]&CONFIG_TCP)?1:0;
					len=1;
				}
			}
			break;

		default:
			break;
	}
	if(ptr!=NULL)
	{
		ptr[0]=0xf5;ptr[1]=0x8b;ptr[2]=cmd;
		ptr[len+3]=0x26;ptr[len+4]=0xfa;
		for(i=0;i<len+5;i++)
			crc=crc+ptr[i];
		ptr[len+5]=(crc>>8)&0xff;
		ptr[len+6]=(crc)&0xff;
		*lenout=len+7;
	}
	return ptr;
}
void common_rw_config(int dev)
{

	rt_uint8_t buf[65];
	rt_uint8_t i=0,delay=0;	
	int data_len,crc_len,longlen=0;
	unsigned char crc[2];
	char ch;	
	int lenout;
	rt_uint8_t len=0,param;
	enum STATE_OP state=GET_F5;
	DBG("enter common_rw_config\r\n");
	rt_uint8_t *ptr=(rt_uint8_t *)buf;
	configlock();
	while(1)
	{
		if(rt_device_read(common_dev[dev], 0, &ch, 1)==1)
		{
		if(ch==0xf5 && state==GET_F5)
		{
			DBG("GET_F5\n");
			state=GET_8A_8B;
		}
		else if(ch==0x8a && state==GET_8A_8B)
		{
			DBG("GET_8A\n");
			data_len=0;
			longlen=0;
			state=GET_DATA;
		}
		else if(ch==0x8b && state==GET_8A_8B)
		{
			DBG("GET_8B\n");
			rt_device_read(common_dev[dev], 0, &ch, 1);
			DBG("GET %d\n",ch);
			char *tmp=send_out(dev,ch,&lenout);
			if(tmp!=NULL)
			{
				int ii=0;
				for(ii=0;ii<lenout;ii++)
					DBG("%2x ",tmp[ii]);
				rt_device_write(common_dev[dev], 0, (void *)tmp, lenout);
			}
			else
				DBG("some error\n");
			break;
		}
		else if(state==GET_DATA)
		{	
			DBG("GET_DATA %2x\n",ch);
			*(ptr+data_len)=ch;
			if(data_len==0)
			{
				if(ch==0x0c || ch==0x0d || ch==0x0e || ch==0x0f || ch==0x20)
					state=GET_LEN;
 			}
			else
			{
				if(ch==0x26&&(data_len>longlen))
					state=GET_FA;
 			}
			
			data_len++;
		}
		else if(state==GET_LEN)
		{
			DBG("GET_LEN %2x\n",ch);
			longlen=ch;
			state=GET_DATA;
		}
		else if(ch==0xfa && state==GET_FA)
		{
			DBG("GET_FA\n");
			data_len--;
			crc_len=0;
			state=GET_CHECSUM;
		}
		else if(state==GET_CHECSUM)
		{
			if(crc_len!=1)
			{
				crc[crc_len++]=ch;
				DBG("GET_SUM %2x\n",ch);
			}
			else
			{
				crc[crc_len++]=ch;
				DBG("GET_SUM %2x\n",ch);
				//verify checksum
				int verify=0xf5+0x8a+0xfa+0x26+longlen;
				rt_kprintf("command is \n");
				for(i=0;i<data_len;i++)
				{
					rt_kprintf("%2x ",ptr[i]);	
					verify+=ptr[i];
				}
				rt_kprintf("crc is %2x %2x,verify is %x\n",crc[0],crc[1],verify);
				if(verify!=(crc[0]<<8|crc[1]))
				{
					rt_device_write(common_dev[dev], 0, (void *)COMMAND_FAIL, strlen(COMMAND_FAIL));
				}
				else
				{
					rt_device_write(common_dev[dev], 0, (void *)COMMAND_OK, strlen(COMMAND_OK));
					set_config(ptr,longlen,dev);					
				}
				state=GET_F5;
				break;
			}
		}
		else
		{
			state=GET_F5;			
			break;
		}
	}
		else
			{
				rt_thread_delay(1);
				delay++;
				if(delay>10)
					break;
			}
	}
	configunlock();
	return ;
}
#endif

static bool flag_cnn[4]={false,false,false,false};
void all_cut()
{
	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);	
	MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0); 
	MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0); 
	flag_cnn[0]=false;
	flag_cnn[1]=false;
	flag_cnn[2]=false;
	flag_cnn[3]=false;
}
void cnn_out(int index,int level)
{
	
	
	if(phy_link)
	{
		if(level&&(flag_cnn[index]==false))
			flag_cnn[index]=true;
		else if(!level&&(flag_cnn[index]==true))
			flag_cnn[index]=false;
		else
			return;
		rt_kprintf("dev %d , level %d, phy_link %d\n",index,level,phy_link);
		switch(index)
		{
			case 0:
				if(level)
					MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0);
				else
					MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);
				break;
			case 1:
				if(level)
					MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,GPIO_PIN_5);
				else
					MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0);	
				break;
			case 2:
				if(level)
					MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,GPIO_PIN_2);
				else
					MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0);	
				break;
			case 3:
				if(level)
				;//	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);
				else
				;//	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0);	
				break;
			default:
				break;
		}
	}
	else
	{
		MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);	
		MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0);	
		MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0);	
		flag_cnn[0]=false;
		flag_cnn[1]=false;
		flag_cnn[2]=false;
		flag_cnn[3]=false;
		//MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0);	
	}
}
int baud(int type)
{
	rt_kprintf("intput %d\n",type);
	switch(type)
	{
		case 0:
			return 115200;
		case 1:
			return 128000;
		case 2:
			return 256000;
		case 3:
			return 460800;
		case 4:
			return 921600;
		case 5:
			return 1000000;
		case 6:
			return 2000000;
		case 7:
			return 4000000;
		case 8:
			return 6000000;
		}
	return 0;
}
void print_config(config g)
{
	rt_kprintf("\n============================================================================>\n");
	rt_kprintf("local_ip %s\n",g.local_ip);
	rt_kprintf("local_ip6 %s\n",g.local_ip6);
	rt_kprintf("local_port0 %d\n",g.local_port[0]);
	rt_kprintf("local_port1 %d\n",g.local_port[1]);
	rt_kprintf("local_port2 %d\n",g.local_port[2]);
	rt_kprintf("local_port3 %d\n",g.local_port[3]);
	rt_kprintf("sub_msk %s\n",g.sub_msk);
	rt_kprintf("gw %s\n",g.gw);
	rt_kprintf("mac %s\n",g.mac);
	rt_kprintf("remote_ip0 %s\n",g.remote_ip[0]);
	rt_kprintf("remote_ip1 %s\n",g.remote_ip[1]);
	rt_kprintf("remote_ip2 %s\n",g.remote_ip[2]);
	rt_kprintf("remote_ip3 %s\n",g.remote_ip[3]);
	rt_kprintf("remote_ip60 %s\n",g.remote_ip6[0]);
	rt_kprintf("remote_ip61 %s\n",g.remote_ip6[1]);
	rt_kprintf("remote_ip62 %s\n",g.remote_ip6[2]);
	rt_kprintf("remote_ip63 %s\n",g.remote_ip6[3]);
	rt_kprintf("remote_port0 %d\n",g.remote_port[0]);
	rt_kprintf("remote_port1 %d\n",g.remote_port[1]);
	rt_kprintf("remote_port2 %d\n",g.remote_port[2]);
	rt_kprintf("remote_port3 %d\n",g.remote_port[3]);
	rt_kprintf("IP socket0 %s socket1 %s socket2 %s socket3 %s\n",((g.config[0]&0x01)==0)?"IPV4":"IPV6",((g.config[1]&0x01)==0)?"IPV4":"IPV6",((g.config[2]&0x01)==0)?"IPV4":"IPV6",((g.config[3]&0x01)==0)?"IPV4":"IPV6");
	rt_kprintf("protol socket0 %s socket1 %s socket2 %s socket3 %s\n",((g.config[0]&0x02)==0x02)?"TCP":"UDP",((g.config[1]&0x02)==0x02)?"TCP":"UDP",((g.config[2]&0x02)==0x02)?"TCP":"UDP",((g.config[3]&0x02)==0x02)?"TCP":"UDP");
	rt_kprintf("mode socket0 %s socket1 %s socket2 %s socket3 %s\n",((g.config[0]&0x04)==0x04)?"SERVER":"CLIENT",((g.config[1]&0x04)==0x04)?"SERVER":"CLIENT",((g.config[2]&0x04)==0x04)?"SERVER":"CLIENT",((g.config[3]&0x04)==0x04)?"SERVER":"CLIENT");
	rt_kprintf("baud %d.%d.%d.%d\n",baud((g.config[0]&0xf8)>>3),baud((g.config[1]&0xf8)>>3),baud((g.config[2]&0xf8)>>3),baud((g.config[3]&0xf8)>>3));
	rt_kprintf("\n============================================================================>\n");
}
int common_w_socket(int dev)
{	
	int len;
	rt_uint8_t /*common_buf[2048],*/*ptr;
	ptr=rt_malloc(2048);
	len=rt_device_read(common_dev[dev], 0, ptr, 2048);
	#if CONFIG_IT	
	if(phy_link&&(len>0)&&g_socket[dev].connected)
	{
		int i;
		for(i=0;i<10;i++)
		{
			char *tmp=rt_malloc(len);
			rt_memcpy(tmp,ptr,len);
			rt_data_queue_push(&g_data_queue[dev*2], tmp, len, RT_WAITING_FOREVER);	
		}
	}
	
	rt_free(ptr);
	#else
	if(phy_link&&(len>0)&&g_socket[dev].connected)
		rt_data_queue_push(&g_data_queue[dev*2], ptr, len, RT_WAITING_FOREVER);	
	else
		rt_free(ptr);
	#endif
	return 0;
}
static rt_err_t common_rx_ind(rt_device_t dev, rt_size_t size)
{
    /* release semaphore to let finsh thread rx data */
	//DBG("dev %d common_rx_ind %d\r\n",which_common_dev(common_dev,dev),size);
    rt_sem_release(&(rx_sem[which_common_dev(common_dev,dev)]));
    return RT_EOK;
}

void common_w(void* parameter)
{
	int dev=((int)parameter)/2;
	static int flag[4]={0,0,0,0};
	DBG("common_w %d Enter\r\n",dev);
	while (1)
	{
		/* wait receive */
		if (rt_sem_take(&(rx_sem[dev]), RT_WAITING_FOREVER) != RT_EOK) continue;
		#if CONFIG_BIN
		//DBG("to read in_low %d\r\n",ind_low(dev));
		if(ind[dev])
		{	
			//DBG("dev %d in socket data flag %d\n",dev,flag);
			if(flag[dev]==1)
			{
				print_config(g_conf);
				flag[dev]=0;	
				void *ptr1=(void *)&g_confb;
				void *ptr2=(void *)&g_conf;
				if(rt_memcmp(ptr1,ptr2,sizeof(config))!=0)
				{
					#if CONFIG_IT					
					common_w_socket(dev);
					times=0;
					#endif
					print_config(g_conf);
				}
			}
			
			/*socket data transfer,use dma*/
			#if !CONFIG_IT
			common_w_socket(dev);
			#else
			char cha;
			while(rt_device_read(common_dev[dev], &cha, &cha, 1)==1);
			#endif
		}
		else
		{
			DBG("dev %d in config data flag %d\n",dev,flag[dev]);
			if(flag[dev]==0)
			{
				flag[dev]=1;
				void *ptr1=(void *)&g_confb;
				void *ptr2=(void *)&g_conf;
				rt_memcpy(ptr1,ptr2,sizeof(config));
			}
			common_rw_config(dev);
			
		}
		#else
		common_w_socket(dev);
		#endif
	}
}
void common_w_usb(void* parameter)
{
	int dev=((int)parameter)/2;
	rt_kprintf("common_w_usb %d\n",dev);
	while(1)
	{
		if (rt_sem_take(&(rx_sem[dev]), RT_WAITING_FOREVER) != RT_EOK) continue;
		_usb_read(dev);
	}
	
}

static void common_r(void* parameter)
{
    rt_size_t data_size;
    const void *last_data_ptr;
	void *data_ptr;
	int dev=(int)parameter;
	rt_err_t r;
	while(1)
	{
		r=rt_data_queue_pop(&g_data_queue[dev], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		//rt_data_queue_pop(&(g_data_queue[dev]), &last_data_ptr, &data_size, 0);
       // if (rt_data_queue_peak(&(g_data_queue[dev]), &data_ptr, &data_size) == RT_EOK)
       // {
			if(r==RT_EOK && data_size!=0 && last_data_ptr)
			{		
				//if(dev==1)
				{
					#if CONFIG_IT
					if(times<=10){
					#endif
					//dillon rt_device_write(common_dev[(dev-1)/2], 0, last_data_ptr, data_size);
					//rt_kprintf("write index %d,%d\n",dev,data_size);
					_usb_write(dev,last_data_ptr,data_size);
					#if CONFIG_IT
					times++;}
					#endif
				}
				//else
					//rt_data_queue_push(&g_data_queue[dev-1], last_data_ptr, data_size, RT_WAITING_FOREVER);
				//if(dev==1)
				rt_free(last_data_ptr);
			}
        //}
		//else
			//rt_thread_delay(1);
		//if(last_data_ptr)
		//	rt_free(last_data_ptr);
	}
}

/*init common1,2,3,4 for 4 socket*/
int common_init(int dev)//0 uart , 1 parallel bus, 2 usb
{
	/*init common device*/
	rt_err_t result;
	rt_uint8_t common[10];
	int i,max_devices=4;
	/*read config data from internal flash*/
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	MAP_SysCtlDelay(1);
	//config select
	MAP_GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_2);
	MAP_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);//ind0
	MAP_GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_BOTH_EDGES);
	MAP_IntEnable(INT_GPIOD);
	MAP_GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
	int ui32Status = MAP_GPIOIntStatus(GPIO_PORTD_BASE, true);
	MAP_GPIOIntClear(GPIO_PORTD_BASE, ui32Status);
	//connect ind
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);//CNN1
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_2);//CNN2
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_3);//CNN3
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);//CNN4
	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);	
	MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0);	
	MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0);	
	default_config();
	memset(config_local_ip,0,11);config_local_ip[11]='\0';
	memset(config_sub_msk,0,11);config_local_ip[11]='\0';
	memset(config_gw,0,11);config_local_ip[11]='\0';
	memset(config_socket3_ip,0,11);config_local_ip[11]='\0';
	memset(config_socket0_ip,0,11);config_local_ip[11]='\0';
	memset(config_socket1_ip,0,11);config_local_ip[11]='\0';
	memset(config_socket2_ip,0,11);config_local_ip[11]='\0';
	memset(config_socket0_local_port,0,9);config_local_ip[11]='\0';
	memset(config_socket1_local_port,0,9);config_local_ip[11]='\0';
	memset(config_socket2_local_port,0,9);config_local_ip[11]='\0';
	memset(config_socket3_local_port,0,9);config_local_ip[11]='\0';
	memset(config_socket0_remote_port,0,9);config_local_ip[11]='\0';
	memset(config_socket1_remote_port,0,9);config_local_ip[11]='\0';
	memset(config_socket2_remote_port,0,9);config_local_ip[11]='\0';
	memset(config_socket3_remote_port,0,9);config_local_ip[11]='\0';
	memset(config_mac,0,13);config_local_ip[11]='\0';
	memset(config_socket0_ip6,0,71);config_local_ip[11]='\0';
	memset(config_socket1_ip6,0,71);config_local_ip[11]='\0';
	memset(config_socket2_ip6,0,71);config_local_ip[11]='\0';
	memset(config_socket3_ip6,0,71);config_local_ip[11]='\0';
	memset(config_local_ip6,0,71);config_local_ip6[71]='\0';
	memset(config_net_protol0,0,8);config_net_protol0[8]='\0';
	memset(config_net_protol1,0,8);config_net_protol1[8]='\0';
	memset(config_net_protol2,0,8);config_net_protol2[8]='\0';
	memset(config_net_protol3,0,8);config_net_protol3[8]='\0';
	memset(config_socket_mode0,0,8);config_socket_mode0[8]='\0';
	memset(config_socket_mode1,0,8);config_socket_mode1[8]='\0';
	memset(config_socket_mode2,0,8);config_socket_mode2[8]='\0';
	memset(config_socket_mode3,0,8);config_socket_mode3[8]='\0';
	memset(config_uart_baud0,0,8);config_uart_baud0[8]='\0';
	memset(config_uart_baud1,0,8);config_uart_baud1[8]='\0';
	memset(config_uart_baud2,0,8);config_uart_baud2[8]='\0';
	memset(config_uart_baud3,0,8);config_uart_baud3[8]='\0';
	memset(config_tcp0,0,8);config_tcp0[8]='\0';
	memset(config_tcp1,0,8);config_tcp1[8]='\0';
	memset(config_tcp2,0,8);config_tcp2[8]='\0';
	memset(config_tcp3,0,8);config_tcp3[8]='\0';
	rt_mutex_init(&mconfigutex, "config_mutex", RT_IPC_FLAG_FIFO);
/*
	for(i=3;i<67;i++)
	{
		config_socket0_ip6[i]=0xff;
		config_socket1_ip6[i]=0xff;
		config_socket2_ip6[i]=0xff;
		config_socket3_ip6[i]=0xff;
		config_local_ip6[i]=0xff;
	}
	config_socket0_ip6[67]=0x26;
	config_socket1_ip6[67]=0x26;
	config_socket2_ip6[67]=0x26;
	config_socket3_ip6[67]=0x26;
	config_local_ip6[67]=0x26;
	config_socket0_ip6[68]=0xfa;
	config_socket1_ip6[68]=0xfa;
	config_socket2_ip6[68]=0xfa;
	config_socket3_ip6[68]=0xfa;
	config_local_ip6[68]=0xfa;
*/
	if(dev==DEV_USB)
		max_devices=5;
	for(i=0;i<max_devices;i++)
	{
		//config sem
		rt_sprintf(common,"common_%d_rx",i);
		rt_sem_init(&(rx_sem[i]), common, 0, 0);
		rt_sprintf(common,"usb_%d_rx",i);
		rt_sem_init(&(usbrx_sem[i]), common, 0, 0);
		if(dev==DEV_UART)
		{
			if(i==1)
				rt_sprintf(common,"uart%d",4);
			else
				rt_sprintf(common,"uart%d",i);
		
			DBG("To open ==>%s\n",common);
			//open uart ,parallel ,usb
			common_dev[i] = rt_device_find(common);
			if (common_dev[i] == RT_NULL)
			{
				DBG("app_common: can not find device: %s\n",common);
				return 0;
			}
			if (rt_device_open(common_dev[i], RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) == RT_EOK)
			{
				//create common_w,r thread to read data from socket write to uart,parallel,usb
				//or read from uart,parallel,usb write to socket
				rt_sprintf(common,"common_wx%d",i);
				tid_common_w[i] = rt_thread_create(common,common_w, (void *)(i*2),4096, 20, 10);
				rt_sprintf(common,"common_rx%d",i);
				tid_common_r[i] = rt_thread_create(common,common_r, (void *)(i*2+1),2048, 20, 10);

				rt_device_set_rx_indicate(common_dev[i], common_rx_ind);

				if(tid_common_w[i]!=RT_NULL)
					rt_thread_startup(tid_common_w[i]);			
				if(tid_common_r[i]!=RT_NULL)
					rt_thread_startup(tid_common_r[i]);
			}
		}
		else if(dev==DEV_BUS)
		{

		}
		else
		{
			//init usbbulk,read data from usb,and transfer data to 4 socket thread by 4 ctl line
			//create 4 common_r thread read data from 4 socket thread,and put data to usb,control 4 ind line
			if(i==0)
			_usb_init();
			rt_kprintf("uub %d\n",i);
			rt_sprintf(common,"common_wx%d",i);
			tid_common_w[i] = rt_thread_create(common,common_w_usb, (void *)(i*2),4096, 20, 10);	
			if(tid_common_w[i]!=RT_NULL)
				rt_thread_startup(tid_common_w[i]);	
			if(i!=4){
			rt_sprintf(common,"common_rx%d",i);
			tid_common_r[i] = rt_thread_create(common,common_r, (void *)(i*2+1),2048, 20, 10);
				
			if(tid_common_r[i]!=RT_NULL)
				rt_thread_startup(tid_common_r[i]);
				}
		}
	}
	print_config(g_conf);
	//rt_thread_delay(300);	
	for(i=0;i<4;i++)
	{
		g_chang[i].cs=g_chang[i].lip6c=g_chang[i].lpc=g_chang[i].mode=g_chang[i].protol=g_chang[i].rip4c=g_chang[i].rip6c=g_chang[i].rpc=0;
		socket_thread_start(i);
	}
	//rt_thread_delay(100);
	//list_mem1();	
	//list_tcps1();
	//list_thread();
	
	DBG("common_init ok\n");
	//list_mem1();	
	return 1;
}
#endif
