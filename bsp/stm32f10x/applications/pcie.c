#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "ec20.h"
#include "nb_iot.h"
#include "ip_module.h"
#include "pcie.h"
#include "bsp_misc.h"
typedef struct _pcie_param {
	rt_device_t dev;
	struct rt_event event;
	struct rt_mutex lock;
	struct rt_semaphore sem;
}pcie_param,*ppcie_param;
ppcie_param g_pcie[2];
rt_uint8_t pcie_init(rt_uint8_t type);
rt_uint8_t pcie_switch(rt_uint8_t type);
static rt_err_t pcie0_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(g_pcie[0]->sem));
	return RT_EOK;
}
static rt_err_t pcie1_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(g_pcie[1]->sem));
	return RT_EOK;
}
static void pcie1_rcv(void* parameter)
{	
	uint32_t len = 0, total_len = 0;
	uint8_t *buf = rt_malloc(1600);
	while(1)	
	{			
		rt_sem_take(&&(g_pcie[1]->sem), RT_WAITING_FOREVER);
		while (1) {
			len = rt_device_read(g_pcie[1]->dev, 0, &(buf[total_len]) , 1600-total_len);

			if (len>0)
			{
				total_len += len;
				buf[total_len] = '\0';
			}
			else
				break;			
		}
		//rt_kprintf("==>%s", buf);
		if (total_len >= 4 && buf[total_len-2] == '\r' && buf[total_len-1] == '\n' || strchr(buf,'>')!=RT_NULL) {
			uint8_t *rcv = (uint8_t *)rt_malloc(total_len+1);
			rt_memcpy(rcv, buf, total_len);
			rcv[total_len] = '\0';
			rt_data_queue_push(&g_data_queue[1], rcv, total_len, RT_WAITING_FOREVER);
			total_len = 0;
		}
	}
}
static void pcie0_rcv(void* parameter)
{	
	uint32_t len = 0, total_len = 0;
	uint8_t *buf = rt_malloc(1600);
	while(1)	
	{			
		rt_sem_take(&&(g_pcie[0]->sem), RT_WAITING_FOREVER);
		while (1) {
			len = rt_device_read(g_pcie[0]->dev, 0, &(buf[total_len]) , 1600-total_len);

			if (len>0)
			{
				total_len += len;
				buf[total_len] = '\0';
			}
			else
				break;			
		}
		//rt_kprintf("==>%s", buf);
		if (total_len >= 4 && buf[total_len-2] == '\r' && buf[total_len-1] == '\n' || strchr(buf,'>')!=RT_NULL) {
			uint8_t *rcv = (uint8_t *)rt_malloc(total_len+1);
			rt_memcpy(rcv, buf, total_len);
			rcv[total_len] = '\0';
			rt_data_queue_push(&g_data_queue[0], rcv, total_len, RT_WAITING_FOREVER);
			total_len = 0;
		}
	}
}
void pcie1_sm(void* parameter)
{
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	while (1) 
	{
		rt_err_t r = rt_data_queue_pop(&g_data_queue[1], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			/*call different module handle function*/
			switch (*(rt_uint8_t *)parameter) 
			{
				case PCIE_2_IP:
					ip_module_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_M26:
					m26_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_EC20:
					ec20_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_NBIOT:
					nb_iot_proc(last_data_ptr, data_size);
					break;
				default:
					rt_kprintf("pcie1 unknown sm\r\n");
					break;				
			}
			if (last_data_ptr != RT_NULL) {
				rt_free((void *)last_data_ptr);
				last_data_ptr = RT_NULL;
			}
		}
	}
}
void pcie0_sm(void* parameter)
{
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	while (1) 
	{
		rt_err_t r = rt_data_queue_pop(&g_data_queue[0], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			/*call different module handle function*/
			switch (*(rt_uint8_t *)parameter) 
			{
				case PCIE_1_IP:
					ip_module_proc(last_data_ptr, data_size);
					break;
				case PCIE_1_M26:
					m26_proc(last_data_ptr, data_size);
					break;
				case PCIE_1_EC20:
					ec20_proc(last_data_ptr, data_size);
					break;
				case PCIE_1_NBIOT:
					nb_iot_proc(last_data_ptr, data_size);
					break;
				default:
					rt_kprintf("pcie0 unknown sm\r\n");
					break;				
			}
			if (last_data_ptr != RT_NULL) {
				rt_free((void *)last_data_ptr);
				last_data_ptr = RT_NULL;
			}
		}
	}
}
void server_proc(void* parameter)
{	
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	while (1) {
		rt_err_t r = rt_data_queue_pop(&g_data_queue[3], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		rt_kprintf("<<%d ",data_size);
		for (int i=0; i<data_size;i++)
			rt_kprintf("%c", ((char *)last_data_ptr)[i]);
		rt_kprintf(">>\r\n");
		rt_free((void *)last_data_ptr);
	}
}

rt_uint8_t pcie_init(rt_uint8_t type0, rt_uint8_t type1)
{
	rt_uint8_t index;
	g_type0 = type0;
	g_type1 = type1;
	g_pcie = (ppcie_param *)rt_malloc(sizeof(ppcie_param) * 2);
	if (type0) {
		g_pcie[0] = (ppcie_param)rt_malloc(sizeof(pcie_param));
		g_pcie[0]->dev = rt_device_find("uart3"); //PCIE1	
		rt_device_open(g_pcie[0]->dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
		rt_event_init(&(g_pcie[0]->event), 	"pcie0_event", 	RT_IPC_FLAG_FIFO );
		rt_mutex_init(&(g_pcie[0]->lock), 	"pcie0_lock", 	RT_IPC_FLAG_FIFO);
		rt_sem_init(&(g_pcie[0]->sem), 		"pcie0_sem", 	0, 0);
	}

	if (type1) {
		g_pcie[1] = (ppcie_param)rt_malloc(sizeof(pcie_param));
		g_pcie[1]->dev = rt_device_find("uart2"); //PCIE2
		rt_device_open(g_pcie[1]->dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);	
		rt_event_init(&(g_pcie[1]->event), 	"pcie1_event", 	RT_IPC_FLAG_FIFO );
		rt_mutex_init(&(g_pcie[1]->lock), 	"pcie1_lock", 	RT_IPC_FLAG_FIFO);
		rt_sem_init(&(g_pcie[1]->sem), 		"pcie1_sem", 	0, 0);
	}
	g_data_queue = (struct rt_data_queue *)rt_malloc(sizeof(struct rt_data_queue)*4);
	/*  g_data_queue 0 pcie0 rxd
	  *  g_data_queue 1 pcie1 rxd
	  *  g_data_queue 2 to server
	  *  g_data_queue 3 from server
	*/
	for (index = 0; index < 4; index++)
		rt_data_queue_init(&g_data_queue[index],64,4,RT_NULL);

	if (type0) {
		rt_device_set_rx_indicate(g_pcie[0]->dev, pcie0_rx_ind);
		rt_thread_startup(rt_thread_create("pcie0_rcv",pcie0_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("pcie0_sm", pcie0_sm,  (void *)&type0,2048, 20, 10));
	}
	if (type1) {
		rt_device_set_rx_indicate(g_pcie[1]->dev, pcie1_rx_ind);
		rt_thread_startup(rt_thread_create("pcie1_rcv",pcie1_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("pcie1_sm", pcie1_sm,  (void *)&type1,2048, 20, 10));
	}
	rt_thread_startup(rt_thread_create("server",server_proc, 0,2048, 20, 10));
	return 1;
}

rt_uint8_t pcie_switch(rt_uint8_t type)
{
	switch (type) 
	{
		case PCIE_1_IP:
			ip_module_start(0);
			break;
		case PCIE_1_M26:
			m26_start(0);
			break;
		case PCIE_1_EC20:
			ec20_start(0);
			break;
		case PCIE_1_NBIOT:
			nb_iot_start(0);
			break;
		case PCIE_2_IP:
			ip_module_start(1);
			break;
		case PCIE_2_M26:
			m26_start(1);
			break;
		case PCIE_2_EC20:
			ec20_start(1);
			break;
		case PCIE_2_NBIOT:
			nb_iot_start(1);
			break;
		default:
			rt_kprintf("uninsert module on pcie\r\n");
			break;
	}	
}
