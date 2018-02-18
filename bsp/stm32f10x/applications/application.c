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
#include "spi_flash_w25qxx.h"
#include "rt_stm32f10x_spi.h"

#include "led.h"
#include "cc1101.h"
#include "string.h"
#include "subPoint.h"
ALIGN(RT_ALIGN_SIZE)
	static rt_uint8_t led_stack[ 512 ];
	static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
	unsigned int count=0;

	rt_hw_led_init();
	while (1)
	{
		/* led1 on */
#ifndef RT_USING_FINSH
	    //rt_kprintf("led on, count : %d\r\n",count);
#endif
		rt_hw_led_off(0);
		rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */
		count++;

		/* led1 off */
#ifndef RT_USING_FINSH
		//rt_kprintf("led off\r\n");
#endif
		rt_hw_led_on(0);
		rt_thread_delay( RT_TICK_PER_SECOND );
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
#define cc1101_hex_printf1(buf, count) \
{\
    int i;\
    int flag=0; \
	for(i = 0; i < count; i++)\
	{\
		if (buf[i] < 32 || buf[i] > 126) \
			flag =1;\
	}\
    for(i = 0; i < count; i++)\
    {\
    	if (!flag) \
        	rt_kprintf("%c", buf[i]);\
        else \
			rt_kprintf("%02x ", buf[i]);\
    }\
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
	rt_hw_spi_init();
	w25qxx_init("sd0","spi11");

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
#endif /* #ifdef RT_USINGRTGUI */
	unsigned int count=0,count1=256;
	rt_uint8_t buf[256]={0};
	rt_uint8_t buf1[256]={0};	
	//rt_thread_delay(1000);
	radio_init();

	while (1) {
		rt_memset(buf,0,256);
		rt_sprintf(buf,"led on , count : %d",count);
		#if 0
		cc1101_send_write(buf,strlen(buf));
		count++;
		#else
		//cc1101_send_write(buf,strlen(buf));
		//rt_hw_led_off(0);
		wait_cc1101_sem();
		int len = cc1101_receive_read(buf1,128);
		if (len > 0)
		{
			rt_kprintf("\r\ncc1101 recv data %d:",len);
			cc1101_hex_printf1(buf1,len);
			rt_kprintf("\r\n");
			handleSub(buf1);			
			count++;
		}
		#endif
		//rt_thread_delay(RT_TICK_PER_SECOND);
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
			2048, 8, 25);
#else
	init_thread = rt_thread_create("init",
			rt_init_thread_entry, RT_NULL,
			2048, 80, 25);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
