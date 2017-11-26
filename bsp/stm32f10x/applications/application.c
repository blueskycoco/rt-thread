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
#include <stdlib.h>
#include <string.h>

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
#include "cJSON.h"
#include "led.h"
static rt_uint8_t lcd_stack[ 1024 ];
static struct rt_thread lcd_thread;
struct rt_semaphore co2_rx_sem,wifi_rx_sem,server_sem,ch2o_rx_sem;
rt_device_t dev_co2,dev_wifi,dev_ch2o;
int data_co2=0,data_ch2o;
char *http_parse_result(const char*lpbuf);
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
	int len1=0,m=0;
	char *ptr=rt_malloc(128);			
	while(1)	
	{		
		if (rt_sem_take(&(co2_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		int len=rt_device_read(dev_co2, 0, ptr+m, 128);		
		if(len>0)	
		{	
			int i;		
			len1=len1+len;
			if(len1==9)
			{
				rt_kprintf("Get from CO2:\n");
				for(i=0;i<len1;i++)		
				{		
					rt_kprintf("%x ",ptr[i]);
				}	
				data_co2=ptr[2]*256+ptr[3];
				rt_kprintf(" %d\n",data_co2);
				len1=0;
				m=0;
				//rt_free(ptr);
			}
			else
				m=m+len;
		}		
	}	
}
static rt_err_t ch2o_rx_ind(rt_device_t dev, rt_size_t size)
{
	/* release semaphore to let finsh thread rx data */	
	//DBG("dev %d common_rx_ind %d\r\n",which_common_dev(common_dev,dev),size);    
	rt_sem_release(&(ch2o_rx_sem));    
	return RT_EOK;
}
void ch2o_rcv(void* parameter)
{	
	int len1=0,m=0;
	char *ptr=rt_malloc(32);			
	while(1)	
	{		
		if (rt_sem_take(&(ch2o_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		int len=rt_device_read(dev_ch2o, 0, ptr+m, 128);		
		if(len>0)	
		{	
			int i;		
			len1=len1+len;
			if(len1==9)
			{
				rt_kprintf("Get from CH2O:\n");
				for(i=0;i<len1;i++)		
				{		
					rt_kprintf("%x ",ptr[i]);
				}	
				data_ch2o=ptr[4]*256+ptr[5];
				rt_kprintf(" %d\n",data_ch2o);
				len1=0;
				m=0;
				//rt_free(ptr);
			}
			else
				m=m+len;
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
char *doit_data(char *text,const char *item_str)
{	
	char *out=RT_NULL;
	cJSON *item_json;	
	item_json=cJSON_Parse(text);	
	if (!item_json)
	{
		rt_kprintf("Error before: [%s]\n",cJSON_GetErrorPtr());
	}
	else	
	{	
		if (item_json)
		{	 		
			cJSON *data;	
			data=cJSON_GetObjectItem(item_json,item_str);
			if(data)		
			{			
				int nLen = strlen(data->valuestring);
				//rt_kprintf("%s ,%d %s\n",item_str,nLen,data->valuestring);			
				out=(char *)malloc(nLen+1);		
				memset(out,'\0',nLen+1);	
				memcpy(out,data->valuestring,nLen);	
			}		
			else		
				rt_kprintf("can not find %s\n",item_str);	
		} 
		else	
			rt_kprintf("get %s failed\n",item_str); 
			cJSON_Delete(item_json);	
	}	
	return out;
}
void wifi_rcv(void* parameter)
{	
	int len=0,i=0,state=0;
	unsigned char ch;	
	char *ptr=(char *)malloc(256);
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
		rt_sem_release(&(server_sem));    
		continue;
		if((len==1 && (ptr[0]=='+'||ptr[0]=='A'))||strstr(ptr,"+ERR")!=RT_NULL)
			continue;
		if(len>0)
		{
			i=i+len;
		#endif
		if(strstr(ptr,"ok")!=RT_NULL)
		{
			memset(ptr,'\0',256);
			i=0;
		}
		else if(/*strstr(ptr,"HTTP/1.1")!=RT_NULL && */strchr(ptr,'}')!=RT_NULL)	
		{	
			int j,m=0;	
			while(1)
			{
				if(ptr[m]=='H'&&ptr[m+1]=='T'&&ptr[m+2]=='T'&&ptr[m+3]=='P'&&ptr[m+4]=='/'&&ptr[m+5]=='1'&&ptr[m+6]=='.'&&ptr[m+7]=='1')
					break;
				m++;
			}
			if(m==i)
			{
				memset(ptr,'\0',256);
				i=0;
				continue;
			}
			rt_kprintf("Get from Server:\n");
			//for(j=m;j<i;j++)		
			//{		
			//	rt_kprintf("%c",ptr[j]);
			//}	
			//rt_kprintf("\n");
			char *result=http_parse_result(ptr+m);
			if(result!=RT_NULL)
			{
				char *id=doit_data(result,"30");
				char *start=doit_data(result,"101");
				char *stop=doit_data(result,"102");
				//rt_kprintf("result is %s\n",result);
				if(id!=RT_NULL)
				{
					rt_kprintf("ID %s\n",id);
					rt_free(id);
				}
				if(start!=RT_NULL)
				{
					rt_kprintf("start time %s\n",start);
					rt_free(start);
				}
				if(stop!=RT_NULL)
				{
					rt_kprintf("stop time %s\n",stop);
					rt_free(stop);
				}
				rt_free(result);
			}
			rt_sem_release(&(server_sem));    
			memset(ptr,'\0',256);
			i=0;
			//rt_free(ptr);				
		}	
		}
	}	
}
char *http_parse_result(const char*lpbuf)  
{
	char *ptmp = RT_NULL;      
	char *response = RT_NULL;   
	ptmp = (char*)strstr(lpbuf,"HTTP/1.1");  
	if(!ptmp)
	{
		rt_kprintf("http/1.1 not find\n");  
		return RT_NULL;
	}
	if(atoi(ptmp + 9)!=200)
	{
		rt_kprintf("result:\n%s\n",lpbuf);   
		return RT_NULL; 
	}
	ptmp = (char*)strstr(lpbuf,"\r\n\r\n"); 
	if(!ptmp)
	{
		rt_kprintf("ptmp is NULL\n");
		return RT_NULL;  
	}
	response = (char *)malloc(strlen(ptmp)+1);  
	if(!response)
	{
		rt_kprintf("malloc failed %d\n",strlen(ptmp)+1);   
		return RT_NULL;  
	}
	strcpy(response,ptmp+4); 
	return response;
}  

char *add_item(char *old,char *id,char *text)
{
	cJSON *root;
	char *out;
	int i=0,j=0;
	if(old!=RT_NULL)
		root=cJSON_Parse(old);
	else
		root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, id, cJSON_CreateString(text));
	out=cJSON_Print(root);	
	cJSON_Delete(root);
	if(old)
		free(old);
	
	return out;
}
char *add_obj(char *old,char *id,char *pad)
{
	cJSON *root,*fmt;
	char *out;
	root=cJSON_Parse(old);
	fmt=cJSON_Parse(pad);
	cJSON_AddItemToObject(root, id, fmt);
	out=cJSON_Print(root);
	cJSON_Delete(root);
	cJSON_Delete(fmt);
	free(pad);
	return out;
}
static void lcd_thread_entry(void* parameter)
{
	int val1=0,val2=1,val3=2,val4=3,val5=4,val6=5;
	uint8_t bat1=20,bat2=0;
	char flag=0;
	char *post_message=RT_NULL;
	unsigned char read_co2[10]={0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	unsigned char switch_at='+';
	unsigned char done='a';
	unsigned char httpd_send[64]={0};//"AT+HTTPDT\r\n";
	unsigned char httpd_local[64]={0};//"AT+HTTPPH=/mango/checkDataYes\r\n";
	u8 str[100],str1[100];	/*将数字信息填充到str里*/
	rt_sprintf(str,"%d%d%d%d%d.%d",val1,val2,val3,val4,val5,val6);
	/*初始化ssd1306*/    
	ssd1306_init();	/*绘制缓冲区，包含电池信息和数字信息*/
	draw("000","000");	/*打开显示*/
	display();
	rt_thread_delay(300);
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
	dev_ch2o=rt_device_find("uart1");
	if (rt_device_open(dev_ch2o, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{
		struct serial_configure config;			
		config.baud_rate=9600;
		config.bit_order = BIT_ORDER_LSB;			
		config.data_bits = DATA_BITS_8;			
		config.parity	 = PARITY_NONE;			
		config.stop_bits = STOP_BITS_1;				
		config.invert	 = NRZ_NORMAL;				
		config.bufsz	 = RT_SERIAL_RB_BUFSZ;			
		rt_device_control(dev_ch2o,RT_DEVICE_CTRL_CONFIG,&config);	
		rt_sem_init(&(ch2o_rx_sem), "ch2o_rx", 0, 0);
		rt_device_set_rx_indicate(dev_ch2o, ch2o_rx_ind);
		rt_thread_startup(rt_thread_create("thread_ch2o",ch2o_rcv, 0,512, 20, 10));
	}
	dev_wifi=rt_device_find("uart2");
	if (rt_device_open(dev_wifi, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{		
		unsigned char httpd_url[64]={0};//"AT+HTTPURL=http://101.200.182.92,8080\r\n";
		unsigned char httpd_mode[64]={0};//"AT+HTTPTP=GET\r\n";	
		strcpy(httpd_url,"AT+HTTPURL=http://101.200.182.92,8080\n");
		strcpy(httpd_mode,"AT+HTTPTP=GET\n");
		strcpy(httpd_local,"AT+HTTPPH=/mango/test?a=");
		strcpy(httpd_send,"AT+HTTPDT\n");
		rt_sem_init(&(wifi_rx_sem), "wifi_rx", 0, 0);
		rt_sem_init(&(server_sem), "server_rx", 0, 0);
		rt_device_set_rx_indicate(dev_wifi, wifi_rx_ind);
		rt_thread_startup(rt_thread_create("thread_wifi",wifi_rcv, 0,512, 20, 10));
		rt_thread_delay(300);
		rt_device_write(dev_wifi, 0, (void *)&switch_at, 1);
		rt_thread_delay(10);
		rt_device_write(dev_wifi, 0, (void *)&switch_at, 1);
		rt_thread_delay(10);
		rt_device_write(dev_wifi, 0, (void *)&switch_at, 1);
		rt_thread_delay(10);
		rt_device_write(dev_wifi, 0, (void *)&done, 1);
		rt_thread_delay(1);
		rt_device_write(dev_wifi, 0, (void *)httpd_url, strlen(httpd_url));
		rt_thread_delay(30);
		rt_device_write(dev_wifi, 0, (void *)httpd_url, strlen(httpd_url));
		rt_thread_delay(30);
		rt_device_write(dev_wifi, 0, (void *)httpd_mode, strlen(httpd_mode));
		rt_thread_delay(30);
		//rt_device_write(dev_wifi, 0, (void *)httpd_local, strlen(httpd_local));
		//rt_thread_delay(80);
	}
	while(1){
		//rt_kprintf("cur str is %s\n",str);
		rt_device_write(dev_co2, 0, (void *)read_co2, 9);
		rt_sprintf(str,"%03d",data_co2);
		rt_sprintf(str1,"%03d",data_ch2o);
		clear();
		draw(str,str1);
		display();		
		post_message=RT_NULL;
		//get device uid,ip,port,cap data,cap time send to server
		//post_message=add_item(NULL,ID_DGRAM_TYPE,TYPE_DGRAM_DATA);
		//post_message=add_item(post_message,ID_DEVICE_UID,"230FFEE9981283737D");
		//post_message=add_item(post_message,ID_DEVICE_IP_ADDR,"192.168.1.63");
		//post_message=add_item(post_message,ID_DEVICE_PORT,"6547");
		if(flag)
			post_message=add_item(post_message,ID_CAP_CO2,str);
		else
		post_message=add_item(post_message,ID_CAP_HCHO,str1);
		flag=!flag;
		//post_message=add_item(post_message,id1,data1);
		//post_message=add_item(post_message,ID_DEVICE_CAP_TIME,"2015-08-06 00:00");
		int i,j=0;
		for(i=0;i<strlen(post_message);i++)
		{
			if(post_message[i]==',')
				j++;
		}
		char *out1=malloc(strlen(post_message)+3*j+1);
		memset(out1,'\0',strlen(post_message)+3*j+1);
		//out1[0]='"';
		j=0;
		for(i=0;i<strlen(post_message);i++)
		{
			if(post_message[i]!='\r'&&post_message[i]!='\n'&&post_message[i]!=9)
			//{
			//	out1[j++]='[';out1[j++]='0';out1[j++]='D';out1[j++]=']';
			//}
			//else if(post_message[i]=='\n')
			//{
			//	out1[j++]='[';out1[j++]='0';out1[j++]='A';out1[j++]=']';
			//} 
			//else
			{
				if(post_message[i]==',')
				{
					out1[j++]='[';out1[j++]='2';out1[j++]='C';out1[j++]=']';
				}
				else
				out1[j++]=post_message[i];
			}
		}
		//out1[j]='"';
		free(post_message);
		char *send=(char *)malloc(strlen(out1)+strlen(httpd_local)+1+1);
		memset(send,'\0',strlen(out1)+strlen(httpd_local)+1+1);
		strcpy(send,httpd_local);
		strcat(send,out1);
		strcat(send,"\n");
		rt_kprintf("send %s",send);
		rt_device_write(dev_wifi, 0, (void *)send, strlen(send));
		rt_thread_delay(10);
		rt_device_write(dev_wifi, 0, (void *)httpd_send, strlen(httpd_send));
		rt_free(send);
		rt_free(out1);
		//rt_sem_take(&(server_sem), RT_WAITING_FOREVER);
		rt_thread_delay(600);
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
