#include "con_socket.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "con_epi.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
rt_int32_t g_dev=0;
rt_int8_t bus_speed_mode=0;
rt_int8_t start_bus_speed=0;
rt_int32_t times=0;
rt_uint8_t config_local_ip[4+8]				={0xF5,0x8A,0x00,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip*/
rt_uint8_t config_socket0_local_port[2+8]	={0xF5,0x8A,0x01,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port0*/
rt_uint8_t config_socket1_local_port[2+8]	={0xF5,0x8A,0x02,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port1*/
rt_uint8_t config_socket2_local_port[2+8]	={0xF5,0x8A,0x03,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port2*/
rt_uint8_t config_socket3_local_port[2+8]	={0xF5,0x8A,0x04,0xff,0xff,0x26,0xfa,0x00,0x00};/*local port3*/
rt_uint8_t config_sub_msk[4+8]				={0xF5,0x8A,0x05,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*sub msk*/
rt_uint8_t config_gw[4+8]					={0xF5,0x8A,0x06,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*gw*/
rt_uint8_t config_mac[6+8]					={0xF5,0x8A,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*mac*/
rt_uint8_t config_socket0_ip[4+8]			={0xF5,0x8A,0x08,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip*/
rt_uint8_t config_socket1_ip[4+8]			={0xF5,0x8A,0x09,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip*/
rt_uint8_t config_socket2_ip[4+8]			={0xF5,0x8A,0x0a,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip*/
rt_uint8_t config_socket3_ip[4+8]			={0xF5,0x8A,0x0b,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip*/
rt_uint8_t config_socket0_ip6[64+8]			={0xF5,0x8A,0x0c,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 ip6*/
rt_uint8_t config_socket1_ip6[64+8]			={0xF5,0x8A,0x0d,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 ip6*/
rt_uint8_t config_socket2_ip6[64+8]			={0xF5,0x8A,0x0e,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 ip6*/
rt_uint8_t config_socket3_ip6[64+8]			={0xF5,0x8A,0x0f,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 ip6*/
rt_uint8_t config_socket0_remote_port[2+8]	={0xF5,0x8A,0x10,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 0 port*/
rt_uint8_t config_socket1_remote_port[2+8]	={0xF5,0x8A,0x11,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 1 port*/
rt_uint8_t config_socket2_remote_port[2+8]	={0xF5,0x8A,0x12,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 2 port*/
rt_uint8_t config_socket3_remote_port[2+8]	={0xF5,0x8A,0x13,0xff,0xff,0x26,0xfa,0x00,0x00};/*socket 3 port*/
rt_uint8_t config_net_protol0[1+8]			={0xF5,0x8A,0x14,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
rt_uint8_t config_net_protol1[1+8]			={0xF5,0x8A,0x15,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
rt_uint8_t config_net_protol2[1+8]			={0xF5,0x8A,0x16,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
rt_uint8_t config_net_protol3[1+8]			={0xF5,0x8A,0x17,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
rt_uint8_t config_socket_mode0[1+8]			={0xF5,0x8A,0x18,0xff,0x26,0xfa,0x00,0x00};/*server mode0*/
rt_uint8_t config_socket_mode1[1+8]			={0xF5,0x8A,0x19,0xff,0x26,0xfa,0x00,0x00};/*server mode1*/
rt_uint8_t config_socket_mode2[1+8]			={0xF5,0x8A,0x1a,0xff,0x26,0xfa,0x00,0x00};/*server mode2*/
rt_uint8_t config_socket_mode3[1+8]			={0xF5,0x8A,0x1b,0xff,0x26,0xfa,0x00,0x00};/*server mode3*/
rt_uint8_t config_uart_baud0[1+8]			={0xF5,0x8A,0x1c,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
rt_uint8_t config_uart_baud1[1+8]			={0xF5,0x8A,0x1d,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
rt_uint8_t config_uart_baud2[1+8]			={0xF5,0x8A,0x1e,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
rt_uint8_t config_uart_baud3[1+8]			={0xF5,0x8A,0x1f,0xff,0x26,0xfa,0x00,0x00};/*uart baud*/
rt_uint8_t config_local_ip6[64+8]			={0xF5,0x8A,0x20,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};/*local ip6*/
rt_uint8_t config_tcp0[1+8]					={0xF5,0x8A,0x21,0xff,0x26,0xfa,0x00,0x00};/*protol0*/
rt_uint8_t config_tcp1[1+8]					={0xF5,0x8A,0x22,0xff,0x26,0xfa,0x00,0x00};/*protol1*/
rt_uint8_t config_tcp2[1+8]					={0xF5,0x8A,0x23,0xff,0x26,0xfa,0x00,0x00};/*protol2*/
rt_uint8_t config_tcp3[1+8]					={0xF5,0x8A,0x24,0xff,0x26,0xfa,0x00,0x00};/*protol3*/
struct rt_mutex config_mutex;
static rt_bool_t flag_cnn[4]={RT_FALSE,RT_FALSE,RT_FALSE,RT_FALSE};
rt_thread_t tid_w_thread[4]={RT_NULL,RT_NULL,RT_NULL,RT_NULL};
rt_thread_t tid_r_thread[4]={RT_NULL,RT_NULL,RT_NULL,RT_NULL};
rt_device_t uart_dev[4] = {RT_NULL,RT_NULL,RT_NULL,RT_NULL};
rt_bool_t ind[4]={RT_TRUE,RT_TRUE,RT_TRUE,RT_TRUE};
rt_bool_t phy_link=RT_FALSE;
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
struct rt_semaphore usbrx_sem[2];

extern rt_int32_t _epi_send_config(rt_uint8_t *cmd,rt_int32_t len);
extern void rt_hw_led_on();
extern void rt_hw_led_off();
extern void socket_thread_start(rt_int32_t i);
extern void set_if(rt_int8_t* netif_name, rt_int8_t* ip_addr, rt_int8_t* gw_addr, rt_int8_t* nm_addr);
extern void set_if6(rt_int8_t* netif_name, rt_int8_t* ip6_addr);
extern rt_size_t _usb_init();
extern rt_int32_t _usb_write(rt_int32_t index, void *buffer, rt_int32_t size);
extern void _usb_read(rt_int32_t dev);
extern rt_int32_t _epi_write(rt_int32_t index, const void *buffer, rt_int32_t size,rt_uint8_t signal);
extern void _epi_read();
rt_int32_t baud(rt_int32_t type);
void print_config(config g);

void configlock()
{
    rt_err_t result;

    result = rt_mutex_take(&config_mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        RT_ASSERT(0);
    }
}
void configunlock()
{
    rt_mutex_release(&config_mutex);
}

rt_int32_t which_uart_dev(rt_device_t *dev,rt_device_t dev2)
{
	rt_int32_t i=0;
	for(i=0;i<4;i++)
		if(dev[i]==dev2)
			break;
	return i;
}
void IntGpioD()
{
	if(MAP_GPIOIntStatus(GPIO_PORTD_BASE, RT_TRUE)&GPIO_PIN_2)
	{		
		MAP_GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
		ind[0]=((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))
				==GPIO_PIN_2)?RT_TRUE:RT_FALSE;
		ind[1]=RT_TRUE;
		ind[2]=((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))
				==GPIO_PIN_2)?RT_TRUE:RT_FALSE;
		ind[3]=((MAP_GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)&(GPIO_PIN_2))
				==GPIO_PIN_2)?RT_TRUE:RT_FALSE;
		rt_kprintf("gpiod 2 int %d\r\n",ind[0]);
		
	}	
}
void IntGpioJ()
{
	if(MAP_GPIOIntStatus(GPIO_PORTJ_BASE, RT_TRUE)&GPIO_PIN_0)
	{		
		MAP_GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
		//rt_kprintf("gpioj 0 int \r\n");
		start_bus_speed=1;
	}	
}

/*get config data to global config zone, or get socket data to buffer*/
void default_config()
{
	struct netif * netif=netif_list;
	g_conf.config[0]=0;//CONFIG_TCP|CONFIG_IPV6|CONFIG_SERVER;
	g_conf.config[1]=CONFIG_TCP|CONFIG_IPV6;//|CONFIG_SERVER;
	g_conf.config[2]=CONFIG_TCP|CONFIG_IPV6;//|CONFIG_SERVER;
	g_conf.config[3]=CONFIG_TCP|CONFIG_IPV6;//|CONFIG_SERVER;
	memset(g_conf.remote_ip[0],'\0',16);
	strcpy(g_conf.remote_ip[0],"192.168.1.101");
	memset(g_conf.remote_ip[1],'\0',16);
	strcpy(g_conf.remote_ip[1],"192.168.1.103");
	memset(g_conf.remote_ip[2],'\0',16);
	strcpy(g_conf.remote_ip[2],"192.168.1.103");
	memset(g_conf.remote_ip[3],'\0',16);
	strcpy(g_conf.remote_ip[3],"192.168.1.103");
	memset(g_conf.local_ip,'\0',16);
	strcpy(g_conf.local_ip,"192.168.1.103");
	
	memset(g_conf.local_ip6,'\0',64);
	strcpy(g_conf.local_ip6,"fe80::1");
	set_if6("e0","fe80::1");
	memset(g_conf.remote_ip6[0],'\0',64);//fe80::2f0:cfff:fe84:5452%7
	//strcpy(g_conf.remote_ip6[0],"fe80::5867:8730:e9e6:d5c5%11");
	//strcpy(g_conf.remote_ip6[0],"fe80::9132:fea4:7252:16e%13");
	strcpy(g_conf.remote_ip6[0],"fe80::216:17ff:fe89:870b%4");
	memset(g_conf.remote_ip6[1],'\0',64);
	strcpy(g_conf.remote_ip6[1],"fe80::5867:8730:e9e6:d5c5%11");
	//strcpy(g_conf.remote_ip6[1],"fe80::9132:fea4:7252:16e%13");
	memset(g_conf.remote_ip6[2],'\0',64);
	strcpy(g_conf.remote_ip6[2],"fe80::9132:fea4:7252:16e%13");
	memset(g_conf.remote_ip6[3],'\0',64);
	strcpy(g_conf.remote_ip6[3],"fe80::9132:fea4:7252:16e%13");
	g_conf.local_port[0]=1234;
	g_conf.local_port[1]=1235;
	g_conf.local_port[2]=1236;
	g_conf.local_port[3]=1237;	
	memset(g_conf.gw,'\0',16);
	strcpy(g_conf.gw,"192.168.1.1");	
	memset(g_conf.sub_msk,'\0',16);
	strcpy(g_conf.sub_msk,"255.255.255.0");
	memset(g_conf.mac,'\0',64);	
	rt_sprintf(g_conf.mac,"%d.%d.%d.%d.%d.%d",
		netif->hwaddr[0],netif->hwaddr[1],netif->hwaddr[2],netif->hwaddr[3]
		,netif->hwaddr[4],netif->hwaddr[5]);
	g_conf.remote_port[0]=1234;
	g_conf.remote_port[1]=1235;
	g_conf.remote_port[2]=1236;
	g_conf.remote_port[3]=1237;
	set_if("e0",g_conf.local_ip,g_conf.gw,g_conf.sub_msk);
}

//0 no change ,1 local socket need reconfig ,2 all socket need reconfig
void set_config(rt_uint8_t *data,rt_int32_t ipv6_len,rt_int32_t dev)
{
	
	rt_bool_t ipv6_changed = RT_FALSE;
	rt_bool_t ipv4_changed = RT_FALSE;
	rt_int32_t i = 0;
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
				ipv4_changed=RT_TRUE;
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
					rt_kprintf("local port changed %d\r\n",dev);
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
				ipv4_changed=RT_TRUE;
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
				ipv4_changed=RT_TRUE;
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
				rt_device_control(uart_dev[dev],RT_DEVICE_CTRL_CONFIG,&config);	
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
				ipv6_changed=RT_TRUE;
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
			rt_kprintf("wrong cmd\r\n");
	}
	if(ipv4_changed)
		set_if("e0",g_conf.local_ip,g_conf.gw,g_conf.sub_msk);
	if(ipv6_changed)
		set_if6("e0",g_conf.local_ip6);
}
rt_bool_t need_reconfig(rt_int32_t dev)
{
	if(g_chang[dev].cs||g_chang[dev].lip6c||g_chang[dev].lpc||g_chang[dev].mode||
	   g_chang[dev].protol||g_chang[dev].rip4c||g_chang[dev].rip6c||g_chang[dev].rpc)
	{
		if(g_chang[dev].cs)
			rt_kprintf("%d client/server changed\r\n",dev);
		if(g_chang[dev].lip6c)
			rt_kprintf("%d local ip6 changed\r\n",dev);
		if(g_chang[dev].lpc)
			rt_kprintf("%d local port changed\r\n",dev);
		if(g_chang[dev].mode)
			rt_kprintf("%d tcp/udp changed\r\n",dev);
		if(g_chang[dev].protol)
			rt_kprintf("%d ipv4/ipv6 changed\r\n",dev);
		if(g_chang[dev].rip4c)
			rt_kprintf("%d remote ip4 changed\r\n",dev);
		if(g_chang[dev].rip6c)
			rt_kprintf("%d remote ip6 changed\r\n",dev);
		if(g_chang[dev].rpc)
			rt_kprintf("%d remote port changed\r\n",dev);
		g_chang[dev].cs = 0;g_chang[dev].lip6c = 0;g_chang[dev].lpc = 0;
		g_chang[dev].mode = 0;g_chang[dev].protol = 0;g_chang[dev].rip4c = 0;
		g_chang[dev].rip6c = 0;g_chang[dev].rpc = 0;
		return RT_TRUE;
	}
	else
		return RT_FALSE;
}
void usb_config(rt_uint8_t *data,rt_int32_t ipv6_len,rt_int32_t dev)
{
	set_config(data,ipv6_len,dev);
	print_config(g_conf);
	void *ptr1=(void *)&g_confb;
	void *ptr2=(void *)&g_conf;
	if(rt_memcmp(ptr1,ptr2,sizeof(config))!=0)
	{
		print_config(g_conf);
	}

}
rt_int8_t *send_out(rt_int32_t dev,rt_int32_t cmd,rt_int32_t *lenout)
{
	rt_int8_t *ptr=NULL;
	rt_int32_t crc=0,i=0,len;
	rt_int8_t *p=NULL;
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
void uart_rw_config(rt_int32_t dev)
{

	rt_uint8_t buf[65];
	rt_uint8_t i=0,delay=0;	
	rt_int32_t data_len,crc_len,longlen=0;
	rt_uint8_t crc[2];
	rt_int8_t ch;	
	rt_int32_t lenout;
	rt_uint8_t len=0,param;
	enum STATE_OP state=GET_F5;
	DBG("enter uart_rw_config\r\n");
	rt_uint8_t *ptr=(rt_uint8_t *)buf;
	configlock();
	while(1)
	{
		if(rt_device_read(uart_dev[dev], 0, &ch, 1)==1)
		{
		if(ch==0xf5 && state==GET_F5)
		{
			DBG("GET_F5\r\n");
			state=GET_8A_8B;
		}
		else if(ch==0x8a && state==GET_8A_8B)
		{
			DBG("GET_8A\r\n");
			data_len=0;
			longlen=0;
			state=GET_DATA;
		}
		else if(ch==0x8b && state==GET_8A_8B)
		{
			DBG("GET_8B\r\n");
			rt_device_read(uart_dev[dev], 0, &ch, 1);
			DBG("GET %d\r\n",ch);
			rt_int8_t *tmp=send_out(dev,ch,&lenout);
			if(tmp!=NULL)
			{
				rt_int32_t ii=0;
				for(ii=0;ii<lenout;ii++)
					DBG("%2x ",tmp[ii]);
				rt_device_write(uart_dev[dev], 0, (void *)tmp, lenout);
			}
			else
				DBG("some error\r\n");
			break;
		}
		else if(state==GET_DATA)
		{	
			DBG("GET_DATA %2x\r\n",ch);
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
			DBG("GET_LEN %2x\r\n",ch);
			longlen=ch;
			state=GET_DATA;
		}
		else if(ch==0xfa && state==GET_FA)
		{
			DBG("GET_FA\r\n");
			data_len--;
			crc_len=0;
			state=GET_CHECSUM;
		}
		else if(state==GET_CHECSUM)
		{
			if(crc_len!=1)
			{
				crc[crc_len++]=ch;
				DBG("GET_SUM %2x\r\n",ch);
			}
			else
			{
				crc[crc_len++]=ch;
				DBG("GET_SUM %2x\r\n",ch);
				//verify checksum
				rt_int32_t verify=0xf5+0x8a+0xfa+0x26+longlen;
				rt_kprintf("command is \r\n");
				for(i=0;i<data_len;i++)
				{
					rt_kprintf("%2x ",ptr[i]);	
					verify+=ptr[i];
				}
				rt_kprintf("crc is %2x %2x,verify is %x\r\n",crc[0],crc[1],verify);
				if(verify!=(crc[0]<<8|crc[1]))
				{
					rt_device_write(uart_dev[dev], 0, (void *)COMMAND_FAIL, strlen(COMMAND_FAIL));
				}
				else
				{
					rt_device_write(uart_dev[dev], 0, (void *)COMMAND_OK, strlen(COMMAND_OK));
					set_config(ptr,longlen,dev);					
				}
				state=GET_F5;
				break;
			}
		}
		else
		{
			while(rt_device_read(uart_dev[dev], 0, &ch, 1)==1);
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
void all_cut()
{
	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);	
	MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0); 
	MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0); 
	flag_cnn[0]=RT_FALSE;
	flag_cnn[1]=RT_FALSE;
	flag_cnn[2]=RT_FALSE;
	flag_cnn[3]=RT_FALSE;
}
void cnn_out(rt_int32_t index,rt_int32_t level)
{
	if(phy_link)
	{
		if(level&&(flag_cnn[index]==RT_FALSE))
			flag_cnn[index]=RT_TRUE;
		else if(!level&&(flag_cnn[index]==RT_TRUE))
			flag_cnn[index]=RT_FALSE;
		else
			return;
		rt_kprintf("dev %d , level %d, phy_link %d\r\n",index,level,phy_link);
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
				//if(level)
				//;//	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);
				//else
				//;//	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0);	
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
		flag_cnn[0]=RT_FALSE;
		flag_cnn[1]=RT_FALSE;
		flag_cnn[2]=RT_FALSE;
		flag_cnn[3]=RT_FALSE;
		//MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0);	
	}
}
rt_int32_t baud(rt_int32_t type)
{
	rt_kprintf("input baud %d\r\n",type);
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
	rt_kprintf("\n============================================================================>\r\n");
	rt_kprintf("local_ip :\t%s\r\n",g.local_ip);
	rt_kprintf("local_ip6 :\t%s\r\n",g.local_ip6);
	rt_kprintf("local_port0 :\t%d\r\n",g.local_port[0]);
	rt_kprintf("local_port1 :\t%d\r\n",g.local_port[1]);
	rt_kprintf("local_port2 :\t%d\r\n",g.local_port[2]);
	rt_kprintf("local_port3 :\t%d\r\n",g.local_port[3]);
	rt_kprintf("sub_msk :\t%s\r\n",g.sub_msk);
	rt_kprintf("gw :\t%s\r\n",g.gw);
	rt_kprintf("mac :\t%s\r\n",g.mac);
	rt_kprintf("remote_ip0 :\t%s\r\n",g.remote_ip[0]);
	rt_kprintf("remote_ip1 :\t%s\r\n",g.remote_ip[1]);
	rt_kprintf("remote_ip2 :\t%s\r\n",g.remote_ip[2]);
	rt_kprintf("remote_ip3 :\t%s\r\n",g.remote_ip[3]);
	rt_kprintf("remote_ip60 :\t%s\r\n",g.remote_ip6[0]);
	rt_kprintf("remote_ip61 :\t%s\r\n",g.remote_ip6[1]);
	rt_kprintf("remote_ip62 :\t%s\r\n",g.remote_ip6[2]);
	rt_kprintf("remote_ip63 :\t%s\r\n",g.remote_ip6[3]);
	rt_kprintf("remote_port0 :\t%d\r\n",g.remote_port[0]);
	rt_kprintf("remote_port1 :\t%d\r\n",g.remote_port[1]);
	rt_kprintf("remote_port2 :\t%d\r\n",g.remote_port[2]);
	rt_kprintf("remote_port3 :\t%d\r\n",g.remote_port[3]);
	rt_kprintf("IP==>\tv4_v6\r\nsocket0 :\t%s\r\nsocket1 :\t%s\r\nsocket2 :\t%s\r\nsocket3 :\t%s\r\n",
		((g.config[0]&0x01)==0)?"IPV4":"IPV6",((g.config[1]&0x01)==0)?"IPV4":"IPV6",
		((g.config[2]&0x01)==0)?"IPV4":"IPV6",((g.config[3]&0x01)==0)?"IPV4":"IPV6");
	rt_kprintf("protol==>\tudp_tcp\r\nsocket0 :\t%s\r\nsocket1 :\t%s\r\nsocket2 :\t%s\r\nsocket3 :\t%s\r\n",
		((g.config[0]&0x02)==0x02)?"TCP":"UDP",((g.config[1]&0x02)==0x02)?"TCP":"UDP",
		((g.config[2]&0x02)==0x02)?"TCP":"UDP",((g.config[3]&0x02)==0x02)?"TCP":"UDP");
	rt_kprintf("mode==>\tclient_server\r\nsocket0 :\t%s\r\nsocket1 :\t%s\r\nsocket2 :\t%s\r\nsocket3 :\t%s\r\n",
		((g.config[0]&0x04)==0x04)?"SERVER":"CLIENT",((g.config[1]&0x04)==0x04)?"SERVER":"CLIENT",
		((g.config[2]&0x04)==0x04)?"SERVER":"CLIENT",((g.config[3]&0x04)==0x04)?"SERVER":"CLIENT");
	rt_kprintf("baud :\t%d.%d.%d.%d\r\n",baud((g.config[0]&0xf8)>>3),baud((g.config[1]&0xf8)>>3),
		baud((g.config[2]&0xf8)>>3),baud((g.config[3]&0xf8)>>3));
	rt_kprintf("\n============================================================================>\r\n");
}
rt_int32_t uart_w_socket(rt_int32_t dev)
{	
	rt_int32_t len;
	rt_uint8_t *ptr;
	if(phy_link&&g_socket[dev].connected)
	{
		ptr=rt_malloc(256);
		len=rt_device_read(uart_dev[dev], 0, ptr, 256);
		if(phy_link&&(len>0)&&g_socket[dev].connected)
		{
			rt_data_queue_push(&g_data_queue[dev*2], ptr, len, RT_WAITING_FOREVER);	
		}
		else
			rt_free(ptr);
	}
	return 0;
}
static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    /* release semaphore to let finsh thread rx data */
    rt_sem_release(&(rx_sem[which_uart_dev(uart_dev,dev)]));
    return RT_EOK;
}

void uart_w_thread(void* parameter)
{
	rt_int32_t dev=((rt_int32_t)parameter)/2;
	static rt_int32_t flag[4]={0,0,0,0};
	DBG("uart_w_thread %d Enter\r\n",dev);
	while (1)
	{
		/* wait receive */
		if (rt_sem_take(&(rx_sem[dev]), RT_WAITING_FOREVER) != RT_EOK) 
			continue;
		if(ind[dev])
		{	
			if(flag[dev]==1)
			{
				print_config(g_conf);
				flag[dev]=0;	
				void *ptr1=(void *)&g_confb;
				void *ptr2=(void *)&g_conf;
				if(rt_memcmp(ptr1,ptr2,sizeof(config))!=0)
				{
					print_config(g_conf);
				}
			}
			
			/*socket data transfer,use dma*/			
			uart_w_socket(dev);
			
		}
		else
		{
			DBG("dev %d in config data flag %d\r\n",dev,flag[dev]);
			if(flag[dev]==0)
			{
				flag[dev]=1;
				void *ptr1=(void *)&g_confb;
				void *ptr2=(void *)&g_conf;
				rt_memcpy(ptr1,ptr2,sizeof(config));
			}
			uart_rw_config(dev);
		}
	}
}
void common_w_usb(void* parameter)
{
	rt_int32_t dev=((rt_int32_t)parameter)/2;
	rt_kprintf("common_w_usb %d\r\n",dev);
	while(1)
		_usb_read(dev);
}
void common_w_epi(void* parameter)
{
	rt_int32_t dev=((rt_int32_t)parameter)/2;
	rt_kprintf("common_w_epi %d\r\n",dev);
	while(1)
	{
		if (rt_sem_take(&(rx_sem[0]), RT_WAITING_FOREVER) != RT_EOK) 
			continue;
		_epi_read();
	}	
}

static void common_r(void* parameter)
{
    rt_size_t data_size;
    const void *last_data_ptr=RT_NULL;
	void *data_ptr;
	rt_int32_t dev=(rt_int32_t)parameter;
	rt_err_t r=RT_EFULL;	
    rt_kprintf("common_r %d Enter\r\n",(dev-1)/2);
	while(1)
	{
		r=rt_data_queue_pop(&g_data_queue[dev], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if(r==RT_EOK && data_size!=0 && last_data_ptr)
		{
			if(g_dev==1)
			{
				_usb_write(dev,(void *)last_data_ptr,data_size);
			}
			else if(g_dev==2)
			{
				_epi_write((dev-1)/2,last_data_ptr,data_size,0x04+(dev-1)/2);
			}
			else
				rt_device_write(uart_dev[(dev-1)/2], 0, last_data_ptr, data_size);
			if(last_data_ptr)
			{
				rt_free((void *)last_data_ptr);
				last_data_ptr=RT_NULL;
			}
		}
	}
}
void bus_speed_test(void *param)
{
	rt_uint8_t config_ip[]={0xF5,0x8A,0x00,0xc0,0xa8,0x01,0x67,0x26,0xfa,0x00,0x00};
	rt_int8_t buf[1022]={0};
	long times=102601;
	long i=0;
	memset(buf,0x38,1022);
	for(i=33;i<127;i++)
		buf[i-33]=i;
	for(i=1;i<11;i++)
	memcpy(buf+93*i+1,buf,93);
	memcpy(buf+931,buf,91);
	while(start_bus_speed==0)
		rt_thread_delay(1);
	rt_kprintf("start bus speed test\n");
	rt_hw_led_on();
	for(i=0;i<times;i++)
		_epi_write(0,buf,1022,0);
	rt_hw_led_off();
	rt_kprintf("end test\n");
	config_ip[6]=config_ip[6]+1;
	_epi_send_config(config_ip,sizeof(config_ip));
}
/*init common1,2,3,4 for 4 socket*/
rt_int32_t common_init(rt_int32_t dev)//0 uart , 1 parallel bus, 2 usb
{
	/*init common device*/
	rt_err_t result;
	rt_uint8_t common[100];
	rt_int32_t i,max_devices=4;
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
	rt_int32_t ui32Status = MAP_GPIOIntStatus(GPIO_PORTD_BASE, RT_TRUE);
	MAP_GPIOIntClear(GPIO_PORTD_BASE, ui32Status);
	//connect ind
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);//CNN1
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_2);//CNN2
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_3);//CNN3
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);//CNN4
	MAP_GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,GPIO_PIN_4);	
	MAP_GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0);
	MAP_GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_2,0);
	//init GPJ0 rt_int32_t
	MAP_GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_PIN_0);
	MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
	MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	MAP_IntEnable(INT_GPIOJ);
	MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0);
	ui32Status = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, RT_TRUE);
	MAP_GPIOIntClear(GPIO_PORTJ_BASE, ui32Status);

	default_config();
	memset(config_local_ip,0,11);config_local_ip[11]='\0';
	memset(config_sub_msk,0,11);config_sub_msk[11]='\0';
	memset(config_gw,0,11);config_gw[11]='\0';
	memset(config_socket3_ip,0,11);config_socket0_ip[11]='\0';
	memset(config_socket0_ip,0,11);config_socket1_ip[11]='\0';
	memset(config_socket1_ip,0,11);config_socket2_ip[11]='\0';
	memset(config_socket2_ip,0,11);config_socket3_ip[11]='\0';
	memset(config_socket0_local_port,0,9);config_socket0_local_port[9]='\0';
	memset(config_socket1_local_port,0,9);config_socket1_local_port[9]='\0';
	memset(config_socket2_local_port,0,9);config_socket2_local_port[9]='\0';
	memset(config_socket3_local_port,0,9);config_socket3_local_port[9]='\0';
	memset(config_socket0_remote_port,0,9);config_socket0_remote_port[9]='\0';
	memset(config_socket1_remote_port,0,9);config_socket1_remote_port[9]='\0';
	memset(config_socket2_remote_port,0,9);config_socket2_remote_port[9]='\0';
	memset(config_socket3_remote_port,0,9);config_socket3_remote_port[9]='\0';
	memset(config_mac,0,13);config_mac[13]='\0';
	memset(config_socket0_ip6,0,71);config_socket0_ip6[71]='\0';
	memset(config_socket1_ip6,0,71);config_socket1_ip6[71]='\0';
	memset(config_socket2_ip6,0,71);config_socket2_ip6[71]='\0';
	memset(config_socket3_ip6,0,71);config_socket3_ip6[71]='\0';
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
	rt_mutex_init(&config_mutex, "config_mutex", RT_IPC_FLAG_FIFO);

	if(dev==DEV_USB)
	{
		max_devices=2;
		g_dev=1;
	}
	for(i=0;i<max_devices;i++)
	{
		//config sem		
		if(dev==DEV_USB)
		{
			rt_sprintf(common,"usb_%d_rx",i);
			rt_sem_init(&(usbrx_sem[i]), common, 0, 0);
		}
		else
		{
			rt_sprintf(common,"common_%d_rx",i);
			rt_sem_init(&(rx_sem[i]), common, 0, 0);
		}
		if(dev==DEV_UART)
		{
			if(i==1)
				rt_sprintf(common,"uart%d",4);
			else
				rt_sprintf(common,"uart%d",i);
		
			DBG("To open ==>%s\r\n",common);
			//open uart ,parallel ,usb
			uart_dev[i] = rt_device_find(common);
			if (uart_dev[i] == RT_NULL)
			{
				DBG("app_common: can not find device: %s\r\n",common);
				return 0;
			}
			if (rt_device_open(uart_dev[i], RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX) == RT_EOK)
			{
				//create common_w,r thread to read data from socket write to uart,parallel,usb
				//or read from uart,parallel,usb write to socket
				rt_int8_t buf[32]="DEV Opened";
				rt_sprintf(common,"common_wx%d",i);
				tid_w_thread[i] = rt_thread_create(common,uart_w_thread, (void *)(i*2),4096, 20, 10);
				rt_sprintf(common,"common_rx%d",i);
				tid_r_thread[i] = rt_thread_create(common,common_r, (void *)(i*2+1),2048, 20, 10);
				rt_device_write(uart_dev[i], 0, buf, strlen(buf));

				rt_device_set_rx_indicate(uart_dev[i], uart_rx_ind);

				if(tid_w_thread[i]!=RT_NULL)
					rt_thread_startup(tid_w_thread[i]);			
				if(tid_r_thread[i]!=RT_NULL)
					rt_thread_startup(tid_r_thread[i]);
			}
		}
		else if(dev==DEV_BUS)
		{			
			if(bus_speed_mode==0)
			{
				if(i==0)
				{
					epi_init();
					rt_sprintf(common,"epi_%d_rx",i);
					rt_sem_init(&(rx_sem[0]), common, 0, 0);
					g_dev=2;
				}
				rt_kprintf("epi %d\r\n",i);
				rt_sprintf(common,"common_wx%d",i);
				if(i==0)
				{
					tid_w_thread[i] = rt_thread_create(common,common_w_epi, (void *)(i*2),2048, 20, 10);	
					if(tid_w_thread[i]!=RT_NULL)
						rt_thread_startup(tid_w_thread[i]);	
				}			
				rt_sprintf(common,"common_rx%d",i);
				tid_r_thread[i] = rt_thread_create(common,common_r, (void *)(i*2+1),4096, 20, 10);
				if(tid_r_thread[i]!=RT_NULL)
					rt_thread_startup(tid_r_thread[i]);
			}
			else
			{	
				if(i==0)				
				epi_init();
			}
		}
		else
		{
			//init usbbulk,read data from usb,and transfer data to 4 socket thread by 4 ctl line
			//create 4 common_r thread read data from 4 socket thread,and put data to usb,control 4 ind line
			if(i==0)
				_usb_init();
			rt_kprintf("uub %d\r\n",i);
			rt_sprintf(common,"common_wx%d",i);
			tid_w_thread[i] = rt_thread_create(common,common_w_usb, (void *)(i*2),4096, 20, 10);	
			if(tid_w_thread[i]!=RT_NULL)
				rt_thread_startup(tid_w_thread[i]);	
			if(i==0)
			{
				rt_sprintf(common,"common_rx%d",i);
				tid_r_thread[i] = rt_thread_create(common,common_r, (void *)(i*2+1),2048, 20, 10);
				if(tid_r_thread[i]!=RT_NULL)
					rt_thread_startup(tid_r_thread[i]);
			}
		}
	}
	print_config(g_conf);
	if(bus_speed_mode==0)
	{
		if(dev==DEV_USB||dev==DEV_BUS)
		{
			g_chang[0].cs=0;g_chang[0].lip6c=0;g_chang[0].lpc=0;g_chang[0].mode=0;
			g_chang[0].protol=0;g_chang[0].rip4c=0;g_chang[0].rip6c=0;g_chang[0].rpc=0;
			socket_thread_start(0);
		}	
		else
		{
			for(i=0;i<4;i++)
			{
				g_chang[i].cs=0;g_chang[i].lip6c=0;g_chang[i].lpc=0;g_chang[i].mode=0;
				g_chang[i].protol=0;g_chang[i].rip4c=0;g_chang[i].rip6c=0;g_chang[i].rpc=0;
				socket_thread_start(i);
			}
		}
		DBG("common_init ok\r\n");
	}
	else
		rt_thread_startup(rt_thread_create("bus_speed",bus_speed_test, 0,4096, 20, 10));
	return 1;
}
