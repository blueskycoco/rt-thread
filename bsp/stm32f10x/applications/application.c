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
unsigned short CRC1(unsigned char *Data,unsigned char Data_length)
{
	unsigned int mid=0;
	unsigned char times=0,Data_index=0;
	unsigned short CRC_data=0xFFFF;
	while(Data_length)
	{
		CRC_data=Data[Data_index]^CRC_data;
		for(times=0;times<8;times++)
		{
			mid=CRC_data;
			CRC_data=CRC_data>>1;
			if(mid & 0x0001)
			{
				CRC_data=CRC_data^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC_data;
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
#endif /* #ifdef RT_USINGRTGUI */
	unsigned int count=0,count1=256;
	rt_uint8_t buf[256]={0};
	rt_uint8_t buf1[256]={0};
	rt_uint8_t cmd_addr[] = {0x01,0x00 ,0x6c ,0xaa ,0x12 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x02 ,0xd1};
	rt_uint8_t cmd_confirm[] = {0x01,0x17,0x6c,0xaa,0x12,0xa1,0x18,0x01,0x02,0x12,0x34,0x00,0x00,0x00,0x01,0x00,0x14,0x01,0x17};
	rt_uint8_t cmd_status[] = {0x01,0x17,0x6c,0xaa,0x12,0xa1,0x18,0x01,0x02,0x12,0x34,0x00,0x00,0x00,0x01,0x00,0x10,0x00,0x02};
	rt_uint8_t resp_addr[32] = {0};
	rt_uint8_t stm32_id[] = {0xa1,0x18,0x01,0x02,0x12,0x34};
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
		int len = cc1101_receive_read(buf1,128);
		if (len > 0)
		{
			//rt_kprintf("read %d  bytes, %s\r\n",len , buf1 );
			//cc1101_send_write(buf1,len);
			//rt_hw_led_off(0);.
			if (rt_memcmp(buf1, cmd_addr, sizeof(cmd_addr)) ==0) {
				resp_addr[0] = cmd_addr[1];
				resp_addr[1] = cmd_addr[0];
				rt_memcpy(resp_addr+2, cmd_addr+2,13);
				rt_memcpy(resp_addr+5, stm32_id, 6);
				resp_addr[15]=0x00;resp_addr[16]=0x01;resp_addr[17]=0x00;
				resp_addr[18]=0x17;
				resp_addr[4]=16;
				unsigned short crc = CRC1(resp_addr,19);
				resp_addr[19]=(crc>>8) & 0xff;
				resp_addr[20]=(crc) & 0xff;
				cc1101_send_write(resp_addr,21);
			} else if (rt_memcmp(buf1, cmd_confirm, sizeof(cmd_confirm)) ==0) {
				resp_addr[0] = cmd_confirm[1];
				resp_addr[1] = cmd_confirm[0];
				rt_memcpy(resp_addr+2, cmd_confirm+2,13);
				//rt_memcpy(resp_addr+5, stm32_id, 6);
				resp_addr[15]=0x00;resp_addr[16]=0x15;resp_addr[17]=0x00;
				resp_addr[18]=0x01;
				resp_addr[4]=16;
				unsigned short crc = CRC1(resp_addr,19);
				resp_addr[19]=(crc>>8) & 0xff;
				resp_addr[20]=(crc) & 0xff;
				cc1101_send_write(resp_addr,21);
			} else if (rt_memcmp(buf1, cmd_status, sizeof(cmd_status)) ==0) {
				resp_addr[0] = cmd_status[1];
				resp_addr[1] = cmd_status[0];
				rt_memcpy(resp_addr+2, cmd_status+2,13);
				resp_addr[15]=0x00;resp_addr[16]=0x11;resp_addr[17]=0x00;
				resp_addr[18]=0x01;
				resp_addr[4]=16;
				unsigned short crc = CRC1(resp_addr,19);
				resp_addr[19]=(crc>>8) & 0xff;
				resp_addr[20]=(crc) & 0xff;
				cc1101_send_write(resp_addr,21);
			}
			count++;
		}
		#endif
		rt_thread_delay(RT_TICK_PER_SECOND);
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
