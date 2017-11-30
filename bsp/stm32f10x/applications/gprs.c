#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include "cc1101.h"
#include "string.h"
struct rt_event gprs_event;
#define GPRS_EVENT_0 (1<<0)
rt_device_t dev_gprs;
uint8_t g_rcv[2048] = {0};
uint32_t g_len = 0;
struct rt_semaphore gprs_rx_sem;
const uint8_t qifcimi[] = "AT+CIMI\n";
static rt_err_t gprs_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(gprs_rx_sem));    
	return RT_EOK;
}
void gprs_rcv(void* parameter)
{	
	while(1)	
	{		
		if (rt_sem_take(&(gprs_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		rt_memset(g_rcv,0,2048);
		g_len=rt_device_read(dev_gprs, 0, g_rcv, 2048);		
		if(g_len>0)	
		{
			rt_event_send(&gprs_event,GPRS_EVENT_0);
		}		
	}	
}
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &gprs_event, GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}

rt_err_t gprs_cmd(uint8_t *cmd, uint32_t cmd_len, uint8_t **rcv, uint32_t *rcv_len, uint32_t timeout)
{
	rt_err_t ret;
	
	ret = rt_device_write(dev_gprs, 0, (void *)cmd, cmd_len);
	
	if (ret == RT_EOK)
		ret = gprs_wait_event(timeout);
	
	if (ret == RT_EOK && g_len > 0 && rcv != RT_NULL) {
		*rcv = (uint8_t *)rt_malloc(g_len * sizeof(uint8_t));
		memcpy(*rcv, g_rcv, g_len);
		*rcv_len = g_len;
	}

	return ret;
}
int gprs_init(void)
{
	/*handle m26*/
	uint8_t *response = NULL;
	uint32_t len = 0;
	dev_gprs=rt_device_find("uart3");
	if (rt_device_open(dev_gprs, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{
		struct serial_configure config;			
		config.baud_rate = 9600;
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
		if (RT_EOK == gprs_cmd(gifcimi, rt_strlen(qifcimi), &reponse, &len, 200))
		{
			rt_kprintf("len %d => %s", len, response);
			rt_free(response);
		}
	}
	return 0;
}
INIT_APP_EXPORT(gprs_init);
