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

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

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
static rt_uint8_t lcd_stack[ 1024 ];
static struct rt_thread lcd_thread;
struct rt_semaphore co2_rx_sem,wifi_rx_sem;
rt_device_t dev_co2,dev_wifi;
int data_co2=0;
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
        count++;
        rt_hw_led_on(0);
		rt_hw_led_on(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        //rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(0);
		rt_hw_led_off(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}
static rt_err_t co2_rx_ind(rt_device_t dev, rt_size_t size)
{
	/* release semaphore to let finsh thread rx data */	
	//DBG("dev %d common_rx_ind %d\r\n",which_common_dev(common_dev,dev),size);    
	rt_sem_release(&(co2_rx_sem));    
	return RT_EOK;
}
void common_w_usb(void* parameter)
{	
	
	char *ptr=rt_malloc(128);			
	while(1)	
	{		
		if (rt_sem_take(&(co2_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		int len=rt_device_read(dev_co2, 0, ptr, 128);
		if(len>0)	
		{	
			int i;		
			rt_kprintf("Get from CO2:\n");
			for(i=0;i<len;i++)		
			{		
				rt_kprintf("%x ",ptr[i]);
			}	
			data_co2=ptr[2]*256+ptr[3];
			rt_kprintf(" %d\n",data_co2);
			//rt_free(ptr);
		}		
	}	
}
static rt_err_t wifi_rx_ind(rt_device_t dev, rt_size_t size)
{
	/* release semaphore to let finsh thread rx data */	
	//DBG("dev %d common_rx_ind %d\r\n",which_common_dev(common_dev,dev),size);    
	rt_sem_release(&(wifi_rx_sem));    
	return RT_EOK;
}
void wifi_rcv(void* parameter)
{	
	int len=0,i=0,state=0;
	unsigned char ch;	
	char *ptr=rt_malloc(256);	

	while(1)	
	{		
		if (rt_sem_take(&(wifi_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;	
		#if 0
		while(1)
		{
			if(rt_device_read(dev_wifi, 0, &ch, 1)==1)
			{
				ptr[i]=ch;
				rt_kprintf("%c",ptr[i]);
				if(ptr[i]=='+')
					state=1;
				else if(ptr[i]=='o' && state==1)
					state=2;
				else if(ptr[i]=='k' && state==2)
				{
					state=0;
					break;
				}
				i++;
			}
		}
		len=i;
		#else
		len=rt_device_read(dev_wifi, 0, ptr+i, 128);
		i+=len;
		#endif
		if(strstr(ptr,"+ok")!=RT_NULL)	
		{	
			int j;		
			rt_kprintf("Get from Wifi:\n");
			for(j=0;j<i;j++)		
			{		
				rt_kprintf("%c",ptr[j]);
			}	
			rt_kprintf("\n");
			memset(ptr,0,256);
			i=0;
			//rt_free(ptr);	
		}	
	}	
}

static void lcd_thread_entry(void* parameter)
{
	int val1=0,val2=1,val3=2,val4=3,val5=4,val6=5;
	uint8_t bat1=20,bat2=0;
	unsigned char read_co2[10]={0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	unsigned char switch_at[4]={'+','+','+'};
	unsigned char done='a';
	unsigned char httpd_send[64]={0};//"AT+HTTPDT\r\n";
	u8 str[100];	/*将数字信息填充到str里*/
	sprintf(str,"%d%d%d%d%d.%d",val1,val2,val3,val4,val5,val6);
	/*初始化ssd1306*/    
	ssd1306_init();	/*绘制缓冲区，包含电池信息和数字信息*/
	draw(bat1,bat2,str);	/*打开显示*/
	display();
	dev_co2=rt_device_find("uart4");
	if (rt_device_open(dev_co2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{
		struct serial_configure config;			
		config.baud_rate=9600;
		config.bit_order = BIT_ORDER_LSB;			
		config.data_bits = DATA_BITS_8;			
		config.parity	 = PARITY_NONE;			
		config.stop_bits = STOP_BITS_1;				
		config.invert	 = NRZ_NORMAL;				
		config.bufsz	 = RT_SERIAL_RB_BUFSZ;			
		rt_device_control(dev_co2,RT_DEVICE_CTRL_CONFIG,&config);	
		rt_sem_init(&(co2_rx_sem), "co2_rx", 0, 0);
		rt_device_set_rx_indicate(dev_co2, co2_rx_ind);
		rt_thread_startup(rt_thread_create("thread_co2",common_w_usb, 0,512, 20, 10));
		rt_device_write(dev_co2, 0, (void *)read_co2, 9);
	}	
	dev_wifi=rt_device_find("uart2");
	if (rt_device_open(dev_wifi, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{		
		unsigned char httpd_url[64]={0};//"AT+HTTPURL=http://101.200.182.92,8080\r\n";
		unsigned char httpd_mode[64]={0};//"AT+HTTPTP=GET\r\n";	
		unsigned char httpd_local[64]={0};//"AT+HTTPPH=/mango/checkDataYes\r\n";
		strcpy(httpd_url,"AT+HTTPURL=http://101.200.182.92,8080\r\n");
		strcpy(httpd_mode,"AT+HTTPTP=GET\r\n");
		strcpy(httpd_local,"AT+HTTPPH=/mango/checkDataYes\r\n");
		strcpy(httpd_send,"AT+HTTPDT\r\n");
		rt_sem_init(&(wifi_rx_sem), "wifi_rx", 0, 0);
		rt_device_set_rx_indicate(dev_wifi, wifi_rx_ind);
		rt_thread_startup(rt_thread_create("thread_wifi",wifi_rcv, 0,512, 20, 10));
		rt_device_write(dev_wifi, 0, (void *)switch_at, 3);
		rt_thread_delay(10);
		rt_device_write(dev_wifi, 0, (void *)&done, 1);
		rt_thread_delay(30);
		rt_device_write(dev_wifi, 0, (void *)httpd_url, strlen(httpd_url));
		rt_thread_delay(30);
		rt_device_write(dev_wifi, 0, (void *)httpd_mode, strlen(httpd_mode));
		rt_thread_delay(30);
		rt_device_write(dev_wifi, 0, (void *)httpd_local, strlen(httpd_local));
		rt_thread_delay(30);
	}
	while(1){
		val1++;
		val2++;
		val3++;
		val4++;
		val5++;
		val6++;
		bat1++;
		bat2++;
		//rt_kprintf("cur str is %s\n",str);
		rt_thread_delay(500);
		rt_device_write(dev_co2, 0, (void *)read_co2, 9);
		sprintf(str,"%07d",data_co2);
		rt_device_write(dev_wifi, 0, (void *)httpd_send, strlen(httpd_send));
		draw(bat1,bat2,str);
		display();
		if(val1==9)
			val1=0;
		if(val2==9)
			val2=0;
		if(val3==9)
			val3=0;
		if(val4==9)
			val4=0;
		if(val5==9)
			val5=0;
		if(val6==9)
			val6=0;
		if(bat1==100)
			bat1=0;
		if(bat2==100)
			bat2=0;
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

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */

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

        /* re-init device driver */
        rt_device_init_all();

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
/* init led thread */
    result = rt_thread_init(&lcd_thread,
                            "oled",
                            lcd_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&lcd_stack[0],
                            sizeof(lcd_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
       rt_thread_startup(&lcd_thread);
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
