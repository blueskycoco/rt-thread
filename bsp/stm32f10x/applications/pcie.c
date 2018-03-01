#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "ec20.h"
//#include "nb_iot.h"
//#include "ip_module.h"
#include "pcie.h"
#include "bsp_misc.h"
rt_uint8_t g_type0 = 0;
rt_uint8_t g_type1 = 0;
int g_index = 0;
//rt_uint8_t pcie_init(rt_uint8_t type);
//rt_uint8_t pcie_switch(rt_uint8_t type);
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
		rt_sem_take(&(g_pcie[1]->sem), RT_WAITING_FOREVER);
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
		rt_sem_take(&(g_pcie[0]->sem), RT_WAITING_FOREVER);
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
	rt_kprintf("pcie1 sm %d\r\n", g_type1);
	while (1) 
	{
		rt_err_t r = rt_data_queue_pop(&g_data_queue[1], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			/*call different module handle function*/
			switch (g_type1) 
			{
				case PCIE_2_IP:
					//ip_module_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_M26:
					m26_proc((void *)last_data_ptr, data_size);
					break;
				case PCIE_2_EC20:
					//ec20_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_NBIOT:
					//nb_iot_proc(last_data_ptr, data_size);
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
	rt_kprintf("pcie0 sm %d\r\n", g_type0);
	while (1) 
	{
		rt_err_t r = rt_data_queue_pop(&g_data_queue[0], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			/*call different module handle function*/
			switch (g_type0) 
			{
				case PCIE_1_IP:
					//ip_module_proc(last_data_ptr, data_size);
					break;
				case PCIE_1_M26:
					m26_proc((void *)last_data_ptr, data_size);
					break;
				case PCIE_1_EC20:
					ec20_proc(last_data_ptr, data_size);
					break;
				case PCIE_1_NBIOT:
					//nb_iot_proc(last_data_ptr, data_size);
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
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &(g_pcie[g_index]->event), GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}
void send_process(void* parameter)
{
	int i=0;
	while(1)	{		
		char id[10] = {0};	
		char data[10] = {0};
		rt_thread_delay(500);
		rt_sprintf(id, "id>%d", i);
		rt_sprintf(data, "data>%d", i);
		char *json = add_item(RT_NULL,id, data);
		json = add_item(json,"id1", "data1");
		json = add_item(json,"id2", "data2");
		json = add_item(json,"id3", "data3");
		if (json != RT_NULL) {		
			int len = rt_strlen(json) + 2;
			//uint8_t *buf = (uint8_t *)rt_malloc(len + 4);
			//buf[0] = DATA_BEGIN0;buf[1] = DATA_BEGIN1;
			//buf[2] = (len >> 8) & 0xff;
			//buf[3] = len & 0xff;
			//rt_memcpy(buf+4, json, rt_strlen(json));
			//rt_uint16_t crc = CRC_check(buf, len+2);
			//buf[len+2] = (crc >> 8) & 0xff;
			//buf[len+3] = crc & 0xff;
			//rt_kprintf("push ptr %p\r\n",buf);
			rt_kprintf("push ptr %p\r\n",json);
			//rt_data_queue_push(&g_data_queue[1], buf, len+4, RT_WAITING_FOREVER);
			rt_data_queue_push(&g_data_queue[2], json, rt_strlen(json), RT_WAITING_FOREVER);
			gprs_wait_event(RT_WAITING_FOREVER);
			rt_free(json);
			//rt_free(buf);
			i++;
		} else 
			rt_kprintf("send process malloc failed\r\n");
	}
}

rt_uint8_t pcie_init(rt_uint8_t type0, rt_uint8_t type1)
{
	rt_uint8_t index;
	g_type0 = type0;
	g_type1 = type1;
	//g_pcie = (ppcie_param *)rt_malloc(sizeof(ppcie_param) * 2);
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
		rt_thread_startup(rt_thread_create("pcie0_sm", pcie0_sm,  0,2048, 20, 10));
	}
	if (type1) {
		rt_device_set_rx_indicate(g_pcie[1]->dev, pcie1_rx_ind);
		rt_thread_startup(rt_thread_create("pcie1_rcv",pcie1_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("pcie1_sm", pcie1_sm,  0,2048, 20, 10));
	}
	rt_thread_startup(rt_thread_create("server",server_proc, 0,2048, 20, 10));
	//rt_thread_startup(rt_thread_create("gprs_send",send_process, 0,1024, 20, 10));
	return 1;
}

rt_uint8_t pcie_switch(rt_uint8_t type)
{
	switch (type) 
	{
		case PCIE_1_IP:
			//ip_module_start(0);
			break;
		case PCIE_1_M26:
			m26_start(0);
			break;
		case PCIE_1_EC20:
			ec20_start(0);
			break;
		case PCIE_1_NBIOT:
			//nb_iot_start(0);
			break;
		case PCIE_2_IP:
			//ip_module_start(1);
			break;
		case PCIE_2_M26:
			m26_start(1);
			break;
		case PCIE_2_EC20:
			//ec20_start(1);
			break;
		case PCIE_2_NBIOT:
			//nb_iot_start(1);
			break;
		default:
			rt_kprintf("uninsert module on pcie\r\n");
			break;
	}	
}
rt_uint8_t check_pcie(rt_uint8_t num) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if (num == 0) {
		/*pcie1_cd pd11*/		
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		if (GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11) == SET)
			return 0;
		return PCIE_1_EC20;
		/*TODO: more check on IO*/
	} else {
		/*pcie2_cd pe15*/		
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_15) == SET)
			return 0;
		return PCIE_2_M26;
		/*TODO: more check on IO*/
	}
}

