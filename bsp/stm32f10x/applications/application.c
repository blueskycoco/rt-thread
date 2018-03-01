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

#include "spi_flash_w25qxx.h"
#include "rt_stm32f10x_spi.h"
#include "button.h"
#include "lcd.h"
#include "led.h"
#include "gprs.h"
#include "cc1101.h"
#include "string.h"
#include "subPoint.h"
#include "master.h"
#include "pcie.h"
#define NO_ERROR				0x00000000
#define ERROR_SPI_HW			0x00000001
#define ERROR_SPI_MOUNT			0x00000002
#define ERROR_CC1101_HW			0x00000004
#define ERROR_PCIE1_HW			0x00000008
#define ERROR_PCIE2_HW			0x00000010
#define ERROR_PCIE_NULL			0x00000020
#define ERROR_FILE_RW			0x00000040
#define ERROR_FILESYSTEM_FORMAT 0x00000080
#define ERROR_LOAD_PARAM		0x00000100
rt_uint32_t err_code = NO_ERROR;
rt_uint8_t  pcie_status = 0x00; /*0x01 pcie1, 0x02 pcie2 0x03 pcie1 & pcie2*/
extern int readwrite();
ALIGN(RT_ALIGN_SIZE)
	static rt_uint8_t led_stack[ 512 ];
	static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
	unsigned int count=0;

	rt_hw_led_init();
	//button_init();
	//battery_init();
	//GPIO_Lcd_Init();
	while (1)
	{
		/* led1 on */
#ifndef RT_USING_FINSH
		// rt_kprintf("led on, count : %d, battery %d\r\n",count,get_bat());
#endif
		rt_hw_led_off(0);
		//buzzer_ctl(1);
		rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */
		//SetStateIco(count%7,0);
		count++;

		/* led1 off */
#ifndef RT_USING_FINSH
		//rt_kprintf("led off\r\n");
#endif
		rt_hw_led_on(0);
		//buzzer_ctl(0);
		rt_thread_delay( RT_TICK_PER_SECOND );
		//rt_kprintf("SetFirstTo0 %d\r\n",count%10);
		//SetErrorCode(count%99);//0 - 10
		//rt_kprintf("SetSignalIco %d\r\n",count%10);
		//SetSignalIco(count%6);//
		//rt_kprintf("SetBatteryWifiIco %d\r\n",count%10);
		//SetBatteryIco(count%5);
		//SetWifiIco(count%5);
		//rt_kprintf("SetSimTypeIco %d\r\n",count%10);
		//SetSimTypeIco(count%5);
		//rt_kprintf("SetStateIco %d\r\n",count%10);
		//SetStateIco(count%7,1);		
	}
}

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
	GPIO_Lcd_Init();
	button_init();
	battery_init();

	/* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* mount sd card fat partition 1 as root directory */
	rt_hw_spi_init();
	if (RT_EOK != w25qxx_init("sd0","spi11"))
	{
		rt_kprintf("w25 init failed\r\n");
		err_code |= ERROR_SPI_HW;
		SetErrorCode(err_code);
	}
	else
	{
		rt_kprintf("w25 init ok\r\n");
		//if (0 == dfs_mkfs("elm","sd0"))
		//	rt_kprintf("mkfs sd0 ok\r\n");
		//else
		//	rt_kprintf("mkfs sd0 failed\r\n");

		if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
		{
			rt_kprintf("File System initialized!\n");
			if (!readwrite())			
			{
				err_code |= ERROR_FILE_RW;
				SetErrorCode(err_code);
				if (0 == dfs_mkfs("elm","sd0"))
					rt_kprintf("mkfs sd0 ok\r\n");
				else
				{
					rt_kprintf("mkfs sd0 failed\r\n");
					err_code |= ERROR_FILESYSTEM_FORMAT;
					SetErrorCode(err_code);		
				}
			}
		}
		else
		{
			if (0 == dfs_mkfs("elm","sd0"))
				rt_kprintf("mkfs sd0 ok\r\n");
			else
			{
				rt_kprintf("mkfs sd0 failed\r\n");
				err_code |= ERROR_FILESYSTEM_FORMAT;
				SetErrorCode(err_code);
			}
			if (dfs_mount("sd0", "/", "elm", 0, 0) ==0)
			{
				rt_kprintf("File System initialzation failed!\n");
				err_code |= ERROR_SPI_MOUNT;
				SetErrorCode(err_code);
			}
			else
			{
				rt_kprintf("File System initialized 2!\n");
			}

		}
	}
	rt_kprintf("spi flash init done , err 0x%08x\r\n",err_code);
#endif  /* RT_USING_DFS */

	unsigned int count=0,count1=256;
	rt_uint8_t buf[256]={0};
	rt_uint8_t buf1[256]={0};	
	rt_uint8_t pcie_status = 0;
	if (!radio_init())
	{
		err_code |= ERROR_CC1101_HW;
		SetErrorCode(err_code);
	}
	rt_kprintf("cc1101 init done , err 0x%08x\r\n",err_code);
	if (!load_param()) {
		rt_kprintf("load param failed\r\n");
		err_code |= ERROR_LOAD_PARAM;
		SetErrorCode(err_code);
	}
	buzzer_ctl(1);
	rt_hw_led_off(CODE_LED);
	rt_hw_led_off(ARM_LED);
	rt_hw_led_off(WIRE_LED);
	rt_hw_led_off(ALARM_LED);
	rt_hw_led_off(WIRELESS_LED);
	rt_hw_led_off(FAIL_LED);
	rt_hw_led_off(NET_LED);
	rt_thread_delay( RT_TICK_PER_SECOND );	
	buzzer_ctl(0);
	rt_hw_led_on(CODE_LED);
	rt_hw_led_on(ARM_LED);
	rt_hw_led_on(WIRE_LED);
	rt_hw_led_on(ALARM_LED);
	rt_hw_led_on(WIRELESS_LED);
	rt_hw_led_on(FAIL_LED);
	rt_hw_led_on(NET_LED);
	rt_thread_delay( RT_TICK_PER_SECOND );		
	buzzer_ctl(1);
	rt_hw_led_off(CODE_LED);
	rt_hw_led_off(ARM_LED);
	rt_hw_led_off(WIRE_LED);
	rt_hw_led_off(ALARM_LED);
	rt_hw_led_off(WIRELESS_LED);
	rt_hw_led_off(FAIL_LED);
	rt_hw_led_off(NET_LED);
	rt_thread_delay( RT_TICK_PER_SECOND );	
	buzzer_ctl(0);
	rt_hw_led_on(CODE_LED);
	rt_hw_led_on(ARM_LED);
	rt_hw_led_on(WIRE_LED);
	rt_hw_led_on(ALARM_LED);
	rt_hw_led_on(WIRELESS_LED);
	rt_hw_led_on(FAIL_LED);
	rt_hw_led_on(NET_LED);
	//rt_thread_delay( RT_TICK_PER_SECOND );	
	SetErrorCode(err_code);
	pcie_status |= check_pcie(0);
	rt_kprintf("PCIE 1 insert %x\r\n", pcie_status);
	pcie_status |= check_pcie(1);
	rt_kprintf("PCIE 2 insert %x\r\n", pcie_status);
	if (pcie_status == 0)
	{
		err_code |= ERROR_PCIE_NULL;
		SetErrorCode(err_code);
	}
		rt_kprintf("pcie identify done , err 0x%08x\r\n",err_code);
	if ((pcie_status & PCIE_2_M26) && (pcie_status & PCIE_1_EC20))
	{
		pcie_init(PCIE_1_EC20,PCIE_2_M26);
		pcie_switch(PCIE_1_EC20);
	} else if(pcie_status & PCIE_2_M26) {
		pcie_init(0,PCIE_2_M26);
		pcie_switch(PCIE_2_M26);
	} else if(pcie_status & PCIE_1_EC20) {
		pcie_init(PCIE_1_EC20,0);
		pcie_switch(PCIE_1_EC20);
	} else {
		buzzer_ctl(1);
	}


	
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
