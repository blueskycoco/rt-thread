#include "con_socket.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "con_common.h"
#include "board.h"
#include "con_uart.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
rt_bool_t ind[4]={RT_TRUE,RT_TRUE,RT_TRUE,RT_TRUE};
extern struct rt_semaphore rx_sem[4];
extern rt_device_t uart_dev[4];
extern struct rt_mutex config_mutex;
extern rt_bool_t phy_link;
enum STATE_OP{
	GET_F5,
	GET_8A_8B,
	GET_LEN,
	GET_DATA,
	GET_26,
	GET_FA,
	GET_CHECSUM
};

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
rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
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

