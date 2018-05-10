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
#include "wtn6.h"
#include "prop.h"
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
extern struct rt_event g_info_event;
extern rt_uint8_t g_main_state;
extern rt_uint32_t g_coding_cnt;
extern rt_uint8_t cur_status;
extern rt_uint8_t s1;
extern rt_uint8_t g_alarm_voice;
extern rt_uint8_t g_delay_in;
extern rt_uint8_t g_delay_out;
extern rt_uint8_t g_index_sub;
extern rt_uint8_t g_mute;
extern rt_uint8_t g_alarmType;
extern rt_uint16_t pgm0_cnt;
extern rt_uint16_t pgm1_cnt;
extern rt_uint8_t g_net_state;
extern int g_index;
rt_uint16_t g_bat = 0;
rt_uint8_t g_ac=1;
rt_uint8_t g_flag=0;
rt_uint8_t heart_time = 0;
extern rt_uint8_t alarm_led;
extern struct rt_semaphore alarm_sem;
extern rt_time_t cur_alarm_time;
struct rt_mutex g_stm32_lock;
extern rt_uint8_t g_alarm_fq;
extern rt_uint16_t g_alarm_reason;
rt_uint16_t should_upload_bat = 0;
extern int readwrite();
ALIGN(RT_ALIGN_SIZE)
	static rt_uint8_t led_stack[ 512 ];
	static struct rt_thread led_thread;
static void led_thread_entry1(void* parameter)
{
	time_t now;	
	#if 1
	set_alarm(17,32,20);
	#endif
	rt_hw_led_init();
	while (1) {
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
		rt_hw_led_on(AUX_LED0);
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
		rt_hw_led_off(AUX_LED0);
		time(&now);
		rt_kprintf("led %s\r\n",ctime(&now));
		//alarm_time->tm_sec += 10;
		//alarm_time->tm_sec %= 60;
		//rtc_set_alarm(mktime(alarm_time));
	}
}
static void set_next_alarm(rt_uint8_t cur_hour,rt_uint8_t cur_mins)
{
	rt_uint8_t hour[5];
	rt_uint8_t mins[5];
	rt_uint8_t i,j,tmp_h,tmp_m,m;
	rt_kprintf("cur hour %d, cur mins %d\r\n", cur_hour,cur_mins);
	hour[0] = (fqp.auto_bufang>>24)&0xff;
	hour[1] = (fqp.auto_bufang>>8)&0xff;
	hour[2] = (fqp.auto_chefang>>24)&0xff;
	hour[3] = (fqp.auto_chefang>>8)&0xff;
	hour[4] = cur_hour;

	mins[0] = (fqp.auto_bufang>>16)&0xff;
	mins[1] = (fqp.auto_bufang)&0xff;
	mins[2] = (fqp.auto_chefang>>16)&0xff;
	mins[3] = (fqp.auto_chefang)&0xff;
	mins[4] = cur_mins;

	for (i=0; i<4; i++)
		for (j=0; j<4-i; j++)
		if (hour[j] > hour[j+1])
		{
			tmp_h = hour[j+1];
			tmp_m = mins[j+1];
			hour[j+1] = hour[j];
			mins[j+1] = mins[j];
			hour[j] = tmp_h;
			mins[j] = tmp_m;
		} else if (hour[j] == hour[j+1]) {
			if (mins[j] > mins[j+1]) {
				tmp_h = hour[j+1];
				tmp_m = mins[j+1];
				hour[j+1] = hour[j];
				mins[j+1] = mins[j];
				hour[j] = tmp_h;
				mins[j] = tmp_m;
			}
		}
/*	for (i=0; i<4; i++)
	{		
		if (hour[i] == hour[i+1]) {
			if (mins[i] > mins[i+1])
			{
				tmp_h = hour[i];
				tmp_m = mins[i];
				hour[i] = hour[i+1];
				mins[i] = mins[i+1];
				hour[i+1] = tmp_h;
				mins[i+1] = tmp_m;
			} 
		}
	}*/
	for (i=0; i<5; i++)
	{
		rt_kprintf("hour[%d] %d , mins %d\r\n",i, hour[i],mins[i]);
	}
	for (i=0; i<5; i++)
	{
		rt_kprintf("hour[%d] %d , mins %d\r\n",i, hour[i],mins[i]);
		if (cur_hour == hour[i] &&
			cur_mins == mins[i]) {			
			j=i;
			m=0;
			while(1) {
				j++;
				if (j==5)
					j=0;
				if (hour[j] != cur_hour && hour[j] != 0xff)
					break;
				else {
					if (mins[j] != cur_mins && mins[j] != 0xff)
						break;
				}
				if (m==5)
					break;
				m++;
			}			
			rt_kprintf("set next alarm to %d:%d m %d\r\n",hour[j],mins[j],m);
			set_alarm(hour[j],mins[j],0);
			break;
		}
	}
}
void set_alarm_now()
{
	struct tm *to;
	rt_time_t local_time;
	time(&local_time);
	to = localtime(&local_time);
	rt_kprintf("auto bu/chefang %04x %04x\r\n", fqp.auto_bufang,fqp.auto_chefang);
	set_next_alarm(to->tm_hour,to->tm_min);
}
static void alarm_thread(void *parameter)
{
	struct tm *to;
	rt_uint32_t hour,mins;
	set_alarm_now();
	while(1) {
		rt_sem_take(&(alarm_sem), RT_WAITING_FOREVER);
		to = localtime(&cur_alarm_time);
		rt_kprintf("cur alarm time %d hour %d, min %d, sec %d, auto_bufang %04x, auto_chefang %04x\r\n",
			cur_alarm_time,to->tm_hour,to->tm_min,to->tm_sec,fqp.auto_bufang,
			fqp.auto_chefang);
		if (((to->tm_hour == ((fqp.auto_bufang>>24)&0xff)) &&
			(to->tm_min == ((fqp.auto_bufang>>16)&0xff))) ||
			((to->tm_hour == ((fqp.auto_bufang>>8)&0xff)) &&
			(to->tm_min == ((fqp.auto_bufang)&0xff))))
		{
			rt_kprintf("begin to hand auto on %d %d\r\n",cur_status,g_delay_out);
			if (!cur_status&& g_delay_out==0)
				handle_protect_on();
		}
		else
		{
			rt_kprintf("begin to hand auto off %d %d\r\n",cur_status,g_delay_out);
			if (cur_status|| (!cur_status && g_delay_out!=0))
			{
				cur_status=0;
				handle_protect_off();			
			}
		}
		set_next_alarm(to->tm_hour,to->tm_min);
	}
}
static void led_thread_entry(void* parameter)
{
	unsigned int count=0;
	rt_uint8_t ac=0;
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
		//buzzer_ctl(1);
		/*heart cnt*/
		heart_time++;
		if (heart_time == 60) {
			if (g_net_state == NET_STATE_LOGED)
				rt_event_send(&(g_pcie[g_index]->event), 1);
			else if (g_net_state == NET_STATE_LOGIN) {
				g_net_state = NET_STATE_INIT;
				rt_event_send(&(g_pcie[g_index]->event), 1);
			}
		}
		/*ac dc*/
		ac=check_ac();
		if (ac && !g_ac)
		{
			g_ac = 1;
			Wtn6_Play(VOICE_JIAOLIUHF,ONCE);
			SetStateIco(4,1);
			if (should_upload_bat!=0) {
				g_alarm_reason=0x0020;
				g_alarm_fq = 0x00;			
				upload_server(CMD_ALARM);
			}
		} else if (!ac && g_ac){
			g_ac = 0;
			Wtn6_Play(VOICE_JIAOLIUDD,ONCE);
			SetStateIco(4,0);
			g_alarm_reason=0x0021;
			g_alarm_fq = 0x00;
			upload_server(CMD_ALARM);
			rt_hw_led_off(AUX_LED1);
			rt_hw_led_off(AUX_LED0);
		}
		/*2 mins exit coding mode*/
		if (g_main_state==1) {
			rt_hw_led_off(0);
			g_coding_cnt++;
			if (g_coding_cnt>120) {
				g_main_state = 0;
				/*play audio here*/
				Wtn6_Play(VOICE_TUICHUDM,ONCE);
				rt_event_send(&(g_info_event), INFO_EVENT_NORMAL);
			}
		}
		/*export led*/
		/*if ((cur_status && ((fqp.is_lamp & 0x0f) == 0x03||(fqp.is_lamp & 0x0f) == 0x04)) ||
			(!cur_status && ((fqp.is_lamp & 0x0f) == 0x01)) || s1 
			|| g_alarmType == 2 || alarm_led)
		{
			rt_kprintf("protect %d, lamp %d, alarmType %d, s1 %d\r\n",
				cur_status,fqp.is_lamp,g_alarmType,s1);
			if (alarm_led) {
				rt_hw_led_on(AUX_LED0);
			} else {	
			if ((fqp.is_lamp & 0x0f) == 0x03 && cur_status || !cur_status && ((fqp.is_lamp & 0x0f) == 0x01)) {
					if (count %2)
						rt_hw_led_on(AUX_LED0);
					else if (count %3)
						rt_hw_led_on(AUX_LED1);
				} else if (!cur_status && ((fqp.is_lamp & 0x0f) == 0x01)){
					rt_hw_led_on(AUX_LED0);				
				}
			}
		}*/
		if (fqp.is_lamp != 0 && g_ac) {
		if (alarm_led) {
			rt_hw_led_on(AUX_LED0);
		} else {
			if ((cur_status && ((fqp.is_lamp & 0x0f) == 0x03) ||
				(!cur_status && ((fqp.is_lamp & 0x0f) == 0x01)))) {
				if (count %2)
					rt_hw_led_on(AUX_LED0);
				else if (count %3)
					rt_hw_led_on(AUX_LED1);
			} else if (!cur_status && (fqp.is_lamp & 0x0f) == 0x04) {
				rt_hw_led_on(AUX_LED1);
			} else if (!cur_status && ((fqp.is_lamp & 0x0f) == 0x02 || (fqp.is_lamp & 0x0f) == 0x03)) {
				rt_hw_led_off(AUX_LED1);
			} else if (cur_status && (fqp.is_lamp & 0x0f) != 0x0) {
				rt_hw_led_on(AUX_LED0);
			}
		}
		} else {
			rt_hw_led_off(AUX_LED1);
		}
		/*bell ctl*/
		if (g_alarm_voice >0 )
		{
			rt_kprintf("alarm voice %d\r\n",g_alarm_voice);
			g_alarm_voice -=1;
			if (g_alarm_voice == 1) {		
			bell_ctl(0);		
			Stop_Playing();
			}
		}
		/*pgm ctl*/
		if (pgm0_cnt >0 || pgm1_cnt >0)
		{
			rt_kprintf("pgm3 %d,pgm4 %d\r\n",pgm0_cnt,pgm1_cnt);
			if (pgm0_cnt>0) {
				pgm0_cnt -=1;
				if (pgm0_cnt == 1) {		
					rt_hw_led_off(PGM3_LED);
				}
			}
			if (pgm1_cnt>0) {
				pgm1_cnt -=1;
				if (pgm1_cnt == 1) {		
					rt_hw_led_off(PGM4_LED);
				}
			}
		}
		/*delay alarm*/
		if (!g_mute) {
		if (!cur_status) {
			if (g_delay_in > 0)
			{
				g_delay_in = 0;
				//g_alarm_voice =0;				
			}
		} else {
		//	rt_kprintf("delay in %d\r\n", g_delay_in);
			if (g_delay_in > 10)
				g_delay_in -= 1;
			else if (g_delay_in >0 && g_delay_in <= 10){
				rt_kprintf("last count %d\r\n",g_delay_in);
				if (g_delay_in == 10) {
					Wtn6_Play(VOICE_COUNTDOWN,ONCE);
					rt_thread_delay(300);
				}
				g_delay_in -= 1;
			} else if (g_delay_in == 0 && g_flag == 0/* && g_alarm_voice >0*/) {
				//rt_kprintf("play end %d\r\n",g_alarm_voice);
				//if (g_alarm_voice == (fqp.alarm_voice_time - fqp.delay_in-1))
				//	Wtn6_Play(VOICE_ALARM1,LOOP);
				g_flag = 1;
				if (fangqu_wireless[g_index_sub].operationType == 1 && fqp.is_alarm_voice &&
					fangqu_wireless[g_index_sub].voiceType == 0) {
					rt_kprintf("open delay fq bell\r\n");
					bell_ctl(1);				
					Wtn6_Play(VOICE_ALARM1,LOOP);
					g_alarm_voice = fqp.alarm_voice_time;
				}
			}
		}
		} else {
			g_delay_in = 0;
			g_alarm_voice =0;				
			Stop_Playing();
		}
		/*delay protect on*/
		if (!g_mute) {
				if (g_delay_out > 10)
					g_delay_out -= 1;
				else if (g_delay_out >0 && g_delay_out <= 10){
					rt_kprintf("out last count %d\r\n",g_delay_out);
					if (g_delay_out == 10)
					{
						Wtn6_Play(VOICE_COUNTDOWN,ONCE);
					}
					if (g_delay_out == 1) {
						rt_thread_delay(100);
						rt_event_send(&(g_info_event), INFO_EVENT_PROTECT_ON);
						}
					g_delay_out -=1;
				}
		} else {
			if (g_delay_out || g_alarm_voice)
				rt_event_send(&(g_info_event), INFO_EVENT_PROTECT_ON);
			g_delay_out = 0;
			g_alarm_voice =0;				
			Stop_Playing();
		}
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */
		//SetStateIco(count%7,0);
		count++;

		/* led1 off */
#ifndef RT_USING_FINSH
		//rt_kprintf("led off\r\n");
#endif
		if (g_main_state ==1)
		rt_hw_led_on(0);
		//buzzer_ctl(0);
		if (alarm_led) {			
			if (fqp.is_lamp != 0 && g_ac) {
			for (int j=0;j<4;j++) {			
					rt_hw_led_on(AUX_LED0);
					rt_thread_delay( RT_TICK_PER_SECOND/8 );
					rt_hw_led_off(AUX_LED0);
					rt_thread_delay( RT_TICK_PER_SECOND/8 );			
			}
			}
		} else {
		
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
		}
		//rt_kprintf("Battery is %d\r\n",ADC_Get_aveg());		
		g_bat = ADC_Get_aveg();
		show_battery(g_bat);	
		if (g_bat < 1000 && (should_upload_bat > 3600 || should_upload_bat ==0)) {
			g_alarm_reason = 0x0022;
			g_alarm_fq = 0x00;
			upload_server(CMD_ALARM);
			should_upload_bat = 1;
		}
		should_upload_bat++;
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
	//rt_kprintf("==========================================\r\n\r\n");
	//rt_kprintf("\t\tUPGRADE Version\r\n");
	//rt_kprintf("\r\n==========================================\r\n\r\n");
	Wtn6_Init();
	GPIO_Lcd_Init();
	button_init();
	//battery_init();
	rt_event_init(&(g_info_event),	"info_event",	RT_IPC_FLAG_FIFO );
	rt_mutex_init(&(g_stm32_lock),	"stm32_lock",	RT_IPC_FLAG_FIFO);
	rt_thread_startup(rt_thread_create("7info",info_user, 0,4096, 20, 10));

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
		//dfs_mkfs("elm","sd0");

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
		dfs_mkfs("elm","sd0");
		load_param();	
		err_code |= ERROR_LOAD_PARAM;
		SetErrorCode(err_code);
	}
	if (err_code == 0) {
		buzzer_ctl(BUZZER_OK);
	} else {
		buzzer_ctl(BUZZER_ERROR);
	}
	Adc_Init();	
	set_date(2018,3,26);
	set_time(11,55,0);
	rt_time_t cur_time = time(RT_NULL);
	rt_kprintf("Curr Time: %08x %d %s\r\n",cur_time, sizeof(cur_time),ctime(&cur_time));
	rt_kprintf("Battery : %04x\r\n",ADC_Get_aveg());
	rt_hw_led_on(CODE_LED);
	rt_hw_led_on(ARM_LED);
	rt_hw_led_on(WIRE_LED);
	rt_hw_led_on(ALARM_LED);
	rt_hw_led_on(WIRELESS_LED);
	rt_hw_led_on(FAIL_LED);
	rt_hw_led_on(NET_LED);
	HtbLcdShow();/*
	SetErrorCode(1);
	SetSignalIco(1);
	SetBatteryIco(4);
	//SetWifiIco(4);
	SetSimTypeIco(1);
	SetStateIco(0,1);
	SetStateIco(1,1);
	SetStateIco(2,1);
	SetStateIco(3,1);
	SetStateIco(4,1);
	SetStateIco(5,1);
	SetStateIco(6,1);*/
	rt_thread_delay(200);	
	HtbLcdClear();	
	cur_status = fqp.status;
	if (fqp.status) {
		SetStateIco(0,1);
		SetStateIco(1,0);
		rt_hw_led_on(ARM_LED);	
		rt_hw_led_on(AUX_LED0);
	}else{
		SetStateIco(1,1);
		SetStateIco(0,0);
		rt_hw_led_off(ARM_LED);
		rt_hw_led_on(AUX_LED1);
	}
	if (check_ac())
		SetStateIco(4,1);	
	rt_hw_led_off(CODE_LED);	
	rt_hw_led_off(WIRE_LED);
	rt_hw_led_off(ALARM_LED);
	rt_hw_led_off(WIRELESS_LED);
	rt_hw_led_off(FAIL_LED);
	rt_hw_led_off(NET_LED);
	//rt_thread_delay( RT_TICK_PER_SECOND );			
	Wtn6_Play(VOICE_WELCOME,ONCE);
	rt_thread_delay( RT_TICK_PER_SECOND );	
	SetErrorCode(err_code);
	pcie_status |= check_pcie(0);
	rt_kprintf("PCIE 1 insert %x\r\n", pcie_status);
	pcie_status |= check_pcie(1);
	rt_kprintf("PCIE 2 insert %x\r\n", pcie_status);
	if (pcie_status == 0)
	{
		err_code |= ERROR_PCIE_NULL;
		SetErrorCode(err_code);
		SetStateIco(3,1);
	}
		rt_kprintf("pcie identify done , err 0x%08x\r\n",err_code);
	if ((pcie_status & PCIE_2_M26) && (pcie_status & PCIE_1_EC20))
	{
		SetSimTypeIco(3);
		//SetSimTypeIco(2);
		pcie_init(PCIE_1_EC20,PCIE_2_M26);
		pcie_switch(PCIE_1_EC20);
		//pcie_switch(PCIE_2_M26);
		SetStateIco(6,1);
		SetStateIco(5,0);
	} else if ((pcie_status & PCIE_1_M26) && (pcie_status & PCIE_2_EC20)) {
		SetSimTypeIco(3);
		//SetSimTypeIco(2);
		pcie_init(PCIE_1_M26,PCIE_2_EC20);
		pcie_switch(PCIE_2_EC20);
		//pcie_switch(PCIE_2_M26);
		SetStateIco(6,1);
		SetStateIco(5,0);
	}else if(pcie_status & PCIE_2_M26) {
		SetSimTypeIco(1);
		pcie_init(0,PCIE_2_M26);
		pcie_switch(PCIE_2_M26);
		SetStateIco(6,1);
		SetStateIco(5,0);
	} else if(pcie_status & PCIE_1_M26) {
		SetSimTypeIco(1);
		pcie_init(PCIE_1_M26,0);
		pcie_switch(PCIE_1_M26);
		SetStateIco(6,1);
		SetStateIco(5,0);
	} else if(pcie_status & PCIE_1_EC20) {
		SetSimTypeIco(3);
		pcie_init(PCIE_1_EC20,0);
		pcie_switch(PCIE_1_EC20);
		SetStateIco(6,1);
		SetStateIco(5,0);
	} else if(pcie_status & PCIE_2_EC20) {
		SetSimTypeIco(3);
		pcie_init(0,PCIE_2_EC20);
		pcie_switch(PCIE_2_EC20);
		SetStateIco(6,1);
		SetStateIco(5,0);
	} else {
		/*play audio here*/
		buzzer_ctl(1);
		SetSimTypeIco(0);
		SetStateIco(3,1);
		Wtn6_Play(VOICE_NOMOKUAI,ONCE);
		rt_thread_delay(550);
		Wtn6_Play(VOICE_NOMOKUAI,ONCE);
		rt_thread_delay(550);
		Wtn6_Play(VOICE_NOMOKUAI,ONCE);
		return ;
	}
	
	if (rt_thread_init(&led_thread,
			"9led",
			led_thread_entry,
			RT_NULL,
			(rt_uint8_t*)&led_stack[0],
			sizeof(led_stack),
			20,
			5) == RT_EOK)
	{
		rt_thread_startup(&led_thread);
	}	
	
	rt_thread_startup(rt_thread_create("alarm",alarm_thread, 0,512, 20, 10));
	while (1) {
		wait_cc1101_sem();
		int len = cc1101_receive_read(buf1,128);
		if (len > 0)
		{		
			//rt_mutex_take(&g_stm32_lock,RT_WAITING_FOREVER);
			rt_kprintf("\r\ncc1101 recv data %d:",len);
			cc1101_hex_printf1(buf1,len);
			rt_kprintf("\r\n");
			handleSub(buf1);				
			rt_mutex_release(&g_stm32_lock);
			count++;
		}
	}
}

int rt_application_init(void)
{
	rt_thread_t init_thread;

	rt_err_t result;

	/* init led thread */

#if (RT_THREAD_PRIORITY_MAX == 32)
	init_thread = rt_thread_create("8init",
			rt_init_thread_entry, RT_NULL,
			4096, 8, 25);
#else
	init_thread = rt_thread_create("8init",
			rt_init_thread_entry, RT_NULL,
			2048, 80, 25);
#endif

	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);
	

	return 0;
}

/*@}*/
