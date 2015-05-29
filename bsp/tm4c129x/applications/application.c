/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2014, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-07-18     ArdaFu       the first version for TM4C129X
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <components.h>

#ifdef RT_USING_LWIP
#include "drv_eth.h"
#endif
#include "led.h"
#include "con_socket.h"

#include <lwip/netdb.h>
#include <lwip/sockets.h>

char buf[]="123456789";
extern void set_if6(char* netif_name, char* ip6_addr);
extern void netio_init(void);

/* led thread entry */
static void led_thread_entry(void* parameter)
{
	//rt_hw_led_init();
	const void *data_ptr;
    rt_size_t data_size;
    const void *last_data_ptr;
	while(1)
	{
		rt_hw_led_on();
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		rt_hw_led_off();
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		//list_thread1();
		//list_mem1();
		//list_tcps1();
	}
}
#if 0
struct rt_semaphore rx_sem;

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    /* release semaphore to let finsh thread rx data */
	//DBG("uart_rx_ind %d\r\n",size);
    rt_sem_release(&rx_sem);
    return RT_EOK;
}

#if 1
rt_device_t uart_dev;
bool phy_link=false;

static void dump_thread_entry(void* parameter)
{
	const void *data_ptr;
    rt_size_t data_size;
    const void *last_data_ptr;
	int dev=(int)parameter;
	long sent_size=0,receive_size=0;
	char uart[6];
	rt_uint8_t uart_buf[512];
	if(dev==1)
	{
		rt_memset(uart,'\0',6);
		rt_sprintf(uart,"uart%d",dev-1);
		uart_dev = rt_device_find(uart);
		if (uart_dev == RT_NULL)
		{
			rt_kprintf("app_uart: can not find device: uart%d\n", dev-1);
			return ;
		}
		if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)
		{
			rt_device_set_rx_indicate(uart_dev, uart_rx_ind);
		}
		rt_sem_init(&(rx_sem), uart, 0, 0);
	}
	while(1)
	{
		if(dev==1)
		{
			if(rt_sem_take(&(rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;
			//rt_kprintf("uart got\n");
			int len,i;
			char *ptr;
			ptr=uart_buf;
			len=rt_device_read(uart_dev, 0, ptr, 512);
			if(phy_link&&(len>0))
			{				
				if((is_right(g_conf.config[dev/2],CONFIG_IPV6)&&g_ip6[dev/2].connected)||(!is_right(g_conf.config[dev/2],CONFIG_IPV6)&&g_ip4[dev/2].connected))
				{					
					rt_data_queue_push(&g_data_queue[dev-1], ptr, len, RT_WAITING_FOREVER);
					//if(!is_right(g_conf.config[dev/2],CONFIG_TCP))
					//rt_thread_delay(10);
				}
			}
		}
		else
		{
			#if 1
			rt_data_queue_pop(&g_data_queue[dev], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
			if(data_size!=0&&last_data_ptr)
			{			
				rt_data_queue_push(&g_data_queue[dev-1], last_data_ptr, data_size, 0); 
				sent_size+=data_size;
				receive_size+=data_size;
				char *ptr=(char *)rt_malloc((data_size+1)*sizeof(char));
				rt_memcpy(ptr,last_data_ptr,data_size);
				ptr[data_size]='\0';
				//rt_kprintf("socket %d sent %d receive %d=>%s\n",dev/2,sent_size,receive_size,ptr);
				rt_free(ptr);
			}
			#else
			rt_data_queue_push(&g_data_queue[dev-1], buf, strlen(buf),RT_WAITING_FOREVER); 
			rt_thread_delay(RT_TICK_PER_SECOND/2);
			#endif
		}
	}
}
static void dump_thread_entry1(void* parameter)
{
	//rt_hw_led_init();
	const void *data_ptr;
    rt_size_t data_size;
    const void *last_data_ptr;
	int dev=(int)parameter;
	while(1)
	{
		rt_data_queue_pop(&g_data_queue[dev], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if(data_size!=0&&last_data_ptr)
		{			
			rt_device_write(uart_dev, 0, last_data_ptr, data_size);
		}
	}
}
#endif
#endif
/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
    /* Initialization RT-Thread Components */
	int i;
#ifdef RT_USING_LWIP
		rt_hw_tiva_eth_init();
#endif
    rt_components_init();
#ifdef RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	
	
	//app_uart_init();
	//uart_init();
	//ring_buffer_init();
	//ping_test("192.168.1.7",5,32);
	//usbtest();
	g_data_queue=(struct rt_data_queue *)rt_malloc(sizeof(struct rt_data_queue)*8);
	for(i=0;i<8;i++)//0,1 for socket0,2,3 for socket1,4,5 for socket2,6,7 for socket3
	{
		if((i%2)==0)
			rt_data_queue_init(&g_data_queue[i], 32, 1, RT_NULL);
		else
			rt_data_queue_init(&g_data_queue[i], 32, 1, RT_NULL);
	}
	common_init(DEV_USB);
	//test_select_connect();
	//test_select_accept();
	netio_init();
	//socket_init();
	//ping_test6("fe80::5867:8730:e9e6:d5c5%11",5,32);
	//ping_test6("fe80::483:d903:e2ee:d05e%12",5,32);
	//ping_test("192.168.2.32",5,32);
	//ping_test("192.168.2.32",5,32);
}

int rt_application_init(void)
{
    rt_thread_t tid;
    rt_thread_t led_thread;
	int i;
	char buf[5];
    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);
    /* Create led thread */
    led_thread = rt_thread_create("led",
			    led_thread_entry, RT_NULL,
			    2048, 20, 20);
    if(led_thread != RT_NULL)
		  rt_thread_startup(led_thread);
	#if 0
	for(i=0;i<4;i++)
	{
		rt_sprintf(buf,"led%d",i);
		led_thread = rt_thread_create(buf,
					dump_thread_entry, (void *)(i*2+1),
					1024, 20, 20);
		if(led_thread != RT_NULL)
			  rt_thread_startup(led_thread);
	}
			led_thread = rt_thread_create("uart0_read",
						dump_thread_entry1, (void *)(1),
						1024, 20, 20);
			if(led_thread != RT_NULL)
				  rt_thread_startup(led_thread);
	#endif
    return 0;
}
