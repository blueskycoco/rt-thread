/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "led.h"
#include "cc1101.h"
#include "string.h"
rt_device_t dev_gprs;
struct rt_semaphore gprs_rx_sem;
const uint8_t qifcimi[] = "AT+CIMI\n";
ALIGN(RT_ALIGN_SIZE)
	static rt_uint8_t led_stack[ 2048 ];
	static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
	unsigned int count=0;
	rt_uint8_t buf[256]={0};

	rt_hw_led_init();
	cc1101_init();
	while (1)
	{
		/* led1 on */
#ifndef RT_USING_FINSH
		//        rt_kprintf("led on, count : %d\r\n",count);
#endif
		rt_sprintf(buf,"led on , count : %d",count);
		cc1101_send_packet(buf,strlen(buf));
		rt_device_write(dev_gprs, 0, (void *)qifcimi, rt_strlen(qifcimi));
		count++;
		rt_hw_led_on(0);
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

		/* led1 off */
#ifndef RT_USING_FINSH
		//        rt_kprintf("led off\r\n");
#endif
		rt_hw_led_off(0);
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
	}
}

#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
	rt_kprintf("cali setup entered\n");
	return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
	rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
			data->min_x,
			data->max_x,
			data->min_y,
			data->max_y);
}
#endif /* RT_USING_RTGUI */
static rt_err_t gprs_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(gprs_rx_sem));    
	return RT_EOK;
}
void gprs_rcv(void* parameter)
{	
	int len1=0,m=0;
	char *ptr=rt_malloc(128);			
	while(1)	
	{		
		if (rt_sem_take(&(gprs_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		rt_memset(ptr,0,128);
		int len=rt_device_read(dev_gprs, 0, ptr, 128);		
		if(len>0)	
		{
			rt_kprintf("%s", ptr);
		}		
	}	
}
void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
	/* initialization RT-Thread Components */
	rt_components_init();
#endif

	/* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
	{
		rt_kprintf("File System initialized!\n");
	}
	else
		rt_kprintf("File System initialzation failed!\n");
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
	{
		extern void rt_hw_lcd_init();
		extern void rtgui_touch_hw_init(void);

		rt_device_t lcd;

		/* init lcd */
		rt_hw_lcd_init();

		/* init touch panel */
		rtgui_touch_hw_init();

		/* find lcd device */
		lcd = rt_device_find("lcd");

		/* set lcd device as rtgui graphic driver */
		rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
		/* init rtgui system server */
		rtgui_system_server_init();
#endif

		calibration_set_restore(cali_setup);
		calibration_set_after(cali_store);
		calibration_init();
	}
#endif /* #ifdef RT_USING_RTGUI */

	/*handle m26*/
	dev_gprs=rt_device_find("uart3");
	if (rt_device_open(dev_gprs, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{
		struct serial_configure config;			
		config.baud_rate=9600;
		config.bit_order = BIT_ORDER_LSB;			
		config.data_bits = DATA_BITS_8;			
		config.parity	 = PARITY_NONE;			
		config.stop_bits = STOP_BITS_1;				
		config.invert	 = NRZ_NORMAL;				
		config.bufsz	 = RT_SERIAL_RB_BUFSZ;			
		rt_device_control(dev_gprs,RT_DEVICE_CTRL_CONFIG,&config);	
		rt_sem_init(&(gprs_rx_sem), "ch2o_rx", 0, 0);
		rt_device_set_rx_indicate(dev_gprs, gprs_rx_ind);
		rt_thread_startup(rt_thread_create("thread_gprs",gprs_rcv, 0,512, 20, 10));
	}
}

int rt_application_init(void)
{
	rt_thread_t init_thread;

	rt_err_t result;

	/* init led thread */
	result = rt_thread_init(&led_thread,
			"led",
			led_thread_entry,
			RT_NULL,
			(rt_uint8_t*)&led_stack[0],
			sizeof(led_stack),
			20,
			5);
	if (result == RT_EOK)
	{
		rt_thread_startup(&led_thread);
	}

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("init",
			rt_init_thread_entry, RT_NULL,
			2048, 8, 20);
#else
	init_thread = rt_thread_create("init",
			rt_init_thread_entry, RT_NULL,
			2048, 80, 20);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
