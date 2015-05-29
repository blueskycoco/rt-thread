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
 * 2013-11-15     bright       add init thread and components initial
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include <board.h>
#include <rtthread.h>
#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#include "led.h"
#include "s1.h"

int one_page_max=0;//one page size
int one_userzone_max=0;//one user zone size
int userzone_num=0;//userzone num
unsigned char chip_info[24];
unsigned char chip_at88sc0104c[10]={0x3b,0xb2,0x11,0x00,0x10,0x80,0x00,0x01,0x10,0x10};
unsigned char chip_at88sc0204c[10]={0x3b,0xb2,0x11,0x00,0x10,0x80,0x00,0x02,0x20,0x20};
unsigned char chip_at88sc0404c[10]={0x3b,0xb2,0x11,0x00,0x10,0x80,0x00,0x04,0x40,0x40};
unsigned char chip_at88sc0808c[10]={0x3b,0xb2,0x11,0x00,0x10,0x80,0x00,0x08,0x80,0x60};
unsigned char chip_at88sc1616c[10]={0x3b,0xb2,0x11,0x00,0x10,0x80,0x00,0x16,0x16,0x80};
unsigned char chip_at88sc3216c[10]={0x3b,0xb3,0x11,0x00,0x00,0x00,0x00,0x32,0x32,0x10};
unsigned char chip_at88sc6416c[10]={0x3b,0xb3,0x11,0x00,0x00,0x00,0x00,0x64,0x64,0x40};
unsigned char chip_at88sc12816c[10]={0x3b,0xb3,0x11,0x00,0x00,0x00,0x01,0x28,0x28,0x60};
unsigned char chip_at88sc25616c[10]={0x3b,0xb3,0x11,0x00,0x00,0x00,0x02,0x56,0x58,0x60};
BOOL Virtual_Alloc()
{
	//memset(chip_info,0xff,24);
	if(get_config(chip_info))
	{
		if(memcmp(chip_info,chip_at88sc0104c,10)==0)
		{
			one_page_max=16;
			one_userzone_max=32;
			userzone_num=4;
			rt_kprintf("\r\nat88sc0104c found\r\n");
		}
		else if(memcmp(chip_info,chip_at88sc0204c,10)==0)
		{
			one_page_max=16;	
			one_userzone_max=64;
			userzone_num=4;
			rt_kprintf("\r\nat88sc0204c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc0404c,10)==0)
		{
			one_page_max=16;	
			one_userzone_max=128;
			userzone_num=4;
			rt_kprintf("\r\nat88sc0404c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc0808c,10)==0)
		{
			one_page_max=16;	
			one_userzone_max=128;
			userzone_num=8;
			rt_kprintf("\r\nat88sc0808c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc1616c,10)==0)
		{
			one_page_max=16;	
			one_userzone_max=128;
			userzone_num=16;
			rt_kprintf("\r\nat88sc1616c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc3216c,10)==0)
		{
			one_page_max=64;	
			one_userzone_max=256;
			userzone_num=16;
			rt_kprintf("\r\nat88sc3216c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc6416c,10)==0)
		{
			one_page_max=64;	
			one_userzone_max=512;
			userzone_num=16;
			rt_kprintf("\r\nat88sc6416c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc12816c,10)==0)
		{
			one_page_max=128;	
			one_userzone_max=1024;
			userzone_num=16;
			rt_kprintf("\r\nat88sc12816c found\r\n");
		}		
		else if(memcmp(chip_info,chip_at88sc25616c,10)==0)
		{
			one_page_max=128;	
			one_userzone_max=2048;
			userzone_num=16;
			rt_kprintf("\r\nat88sc25616c found\r\n");
		}		
		else
		{
			rt_kprintf("\r\nUnKnown at88sc chip\r\n");
			return FALSE;
		}
		
	}
	else
	{
		rt_kprintf("\r\nat88sc bus error\r\n");
	
		return FALSE;
	}
	return TRUE;
}

#if !BURN

typedef struct {
	unsigned char *data;
	unsigned char g[8];
	unsigned char pw[3];
	unsigned int addr;
	unsigned int size;
}at88,*pat88;

BOOL WriteReg(at88* value)
{
	BOOL rc = FALSE;
	int index=0,local_size,local_addr;
	ge param;
	/*transfer addr to userzone 0 byte to 32768 byte*/
	if((value->addr+value->size)>one_userzone_max*userzone_num)
		return FALSE;
	int user_zone_begin=value->addr/one_userzone_max;
	int user_zone_end=(value->addr+value->size)/one_userzone_max;
	memcpy(param.g,value->g,8);
	memcpy(param.pw,value->pw,3);	
	param.use_g=0;
	param.use_pw=0;
	param.page_size=one_page_max;	
	local_size=value->size;
	local_addr=value->addr;
	for(param.zone_index=user_zone_begin;param.zone_index<user_zone_end;param.zone_index++)
	{		
		param.addr=local_addr%one_userzone_max;
		if((param.addr+local_size)<=one_userzone_max)
			param.len=local_size;
		else
			param.len=one_userzone_max-param.addr;
			
		//memcpy(param.user_zone,value->data+index,param.len);
		param.user_zone=value->data+index;
		userzone_proc(&param,0);
		local_size=local_size-param.len;		
		local_addr=local_addr+param.len;
		index=index+param.len;
	}
}

BOOL ReadReg(at88* value)
{
	int index=0,local_size,local_addr;
	ge param;
	/*transfer addr to userzone 0 byte to 32768 byte*/
	if((value->addr+value->size)>one_userzone_max*userzone_num)
		return FALSE;
	int user_zone_begin=value->addr/one_userzone_max;
	int user_zone_end=(value->addr+value->size)/one_userzone_max;
	memcpy(param.g,value->g,8);
	memcpy(param.pw,value->pw,3);	
	param.use_g=0;
	param.use_pw=0;
	param.page_size=one_page_max;
	local_size=value->size;
	local_addr=value->addr;
	for(param.zone_index=user_zone_begin;param.zone_index<user_zone_end;param.zone_index++)
	{		
		param.addr=local_addr%one_userzone_max;
		if((param.addr+local_size)<=one_userzone_max)
			param.len=local_size;
		else
			param.len=one_userzone_max-param.addr;
			
		//memcpy(param.user_zone,value->data+index,param.len);
		param.user_zone=value->data+index;
		/*rt_kprintf("user_zone %x\r\ng %x %x %x %x %x %x %x %x\r\npw %x %x %x\r\npage_size %x\r\nzone_index %x\r\n",
			param.user_zone,param.g[0],param.g[1],param.g[2],param.g[3],param.g[4],param.g[5],param.g[6],param.g[7],
			param.pw[0],param.pw[1],param.pw[2],param.page_size,param.zone_index);*/
		userzone_proc(&param,1);
		local_size=local_size-param.len;		
		local_addr=local_addr+param.len;
		index=index+param.len;
	}
return TRUE;
}


#else
void b(unsigned char fuse)
{
	pe p;
	int i;
	int fd,length;
	memset(&p,0xff,sizeof(pe));
	p.New_SecureCode[0]=0x32;
	p.New_SecureCode[1]=0xb2;
	p.New_SecureCode[2]=0x3f;
	p.New_SecureCode[3]=0xff;
	p.New_SecureCode[4]=0x4b;
	p.New_SecureCode[5]=0x1c;
	p.New_SecureCode[6]=0x8a;
	p.ar[0]=0x17;//normal auth,encrypted
	p.ar[1]=0x00;//(zone<<6)|(zone&0x3);//use g[zone],pw[zone]
	p.num_ar=userzone_num;
	for(i=0;i<7;i++)
	{
		p.ci[i]=i;
		if(i!=3)
			p.pw[i]=i;
	}
	
	for(i=0;i<8;i++)
	{
		p.g[i]=i;
	}
	for(i=0;i<7;i++)
	{
		p.id[i]=0xee;//assign id
	}
	p.fuse=fuse;
	
	burn(p);
}
#endif

static void rt_init_thread_entry(void* parameter)
{
	rt_thread_t system_thread;
	rt_uint8_t buf[128];
	int i;

	/* Initialization RT-Thread Components */
#ifdef RT_USING_COMPONENTS_INIT
	rt_components_init();
#endif

	/* Set finsh device */
#ifdef  RT_USING_FINSH
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */
	//rt_hw_led1_off();
	//cmx865a_init();


	//unsigned int count=1000;
	//rt_memset(buf,'\0',256);
	rt_hw_led_init();
//	rt_kprintf("led on, count : %d\r\n",count);
	//rt_sprintf(buf,"%s","- RT -    Thread Operating System");
		
	//	ST7585_Write_String(0,5,"- RT -    ");
	//ST7585_Write_String(0,4,"Thread Operating System");
	//	Draw_bat(3);
	Virtual_Alloc();
#if BURN
	b(0);
#else
	at88 at88;
	at88.data=buf;//(unsigned char *)malloc(32);
	//memset(at88.data,123,128);
	for(i=0;i<128;i++)
		at88.data[i]=i;
	for(i=0;i<3;i++)
	{
		at88.pw[i]=i;
	}
	for(i=0;i<8;i++)
	{
		at88.g[i]=i;
	}
	#if READ
	
	at88.addr=0;
	at88.size=128;
	ReadReg(&at88);
	
	AT88DBG("\nRead user zone data again:\n");
	for(i=0;i<at88.size;i++)
	{
		if(i%8==0 && i!=0)
			AT88DBG("\n");
		AT88DBG("%4X ",at88.data[i]); 	
	}

	#else
	at88.addr=0;
	at88.size=128;
	WriteReg(&at88);
	#endif
#endif
	while (1)
	{
		
		/* led1 on */
#ifdef RT_USING_FINSH
		rt_kprintf("led on , count : %d\r\n",count);
#endif
	
		
	//	rt_sprintf(buf,"led on , count : %d",count);
	//	ST7585_Write_String(0,5,buf);

		//test_cmx865a();
	//	count++;
		rt_hw_led1_off();
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

		/* led1 off */
#ifdef RT_USING_FINSH
		rt_kprintf("led off\r\n");
#endif
	//	rt_sprintf(buf,"led off, count : %d",count);
	//	ST7585_Write_String(0,5,buf);

		rt_hw_led1_on();
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
	}

}

int rt_application_init()
{
  rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
  init_thread = rt_thread_create("init",
	  rt_init_thread_entry, RT_NULL,
	  2048, 8, 20);
#else
  init_thread = rt_thread_create("init",
	  rt_init_thread_entry, RT_NULL,
	  512, 80, 20);
#endif
  if(init_thread != RT_NULL)
	rt_thread_startup(init_thread);

  return 0;
}


/*@}*/
