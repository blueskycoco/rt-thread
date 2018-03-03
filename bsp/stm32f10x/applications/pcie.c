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
#include "master.h"
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
	rt_kprintf("pcie1 sm %x\r\n", g_type1);
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
	rt_kprintf("pcie0 sm %x\r\n", g_type0);
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
					ec20_proc((void *)last_data_ptr, data_size);
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
/*ADAE0003000102ADAE0400010203ADAE050001020304ADAE*/
rt_uint8_t handle_server(rt_uint8_t *data, rt_size_t len)
{
	int i=0;
	int cnt=0;
	rt_uint16_t packet_len[5];
	rt_uint8_t index[5];
	rt_uint16_t crc[5];
	for (i=0; i<len; i++)
	{
		//rt_kprintf("data %x\r\n",data[i]);
		if (data[i] == 0xAD && data[i+1] == 0xAC)
		{
			rt_kprintf("we got ADAC %d\r\n",i);
			if (i+2 > len)
			{
				rt_kprintf("return 1 %d %d\r\n", i+2,len);
				return 0;
			}
			packet_len[cnt] = (data[i+2]<<8)|data[i+3];
			index[cnt]=i+4;
			if (i+packet_len[cnt]-2 >len)
			{
				rt_kprintf("return 2 %d %d\r\n", i+packet_len[cnt]-2,len);
				return 0;
			}
			crc[cnt]=(data[i+packet_len[cnt]-2]<<8)|data[i+packet_len[cnt]-1];
			rt_kprintf("Found 0xAD 0xAC at %d, len %d, crc %x\r\n", index[cnt],packet_len[cnt],crc[cnt]);
			if (crc[cnt] == CRC_check(data+i+2,packet_len[cnt]-4))
			{
				rt_kprintf("packet %d, CRC is match\r\n",index[cnt]);
				handle_packet(data+index[cnt]);
			} else {
				rt_kprintf("packet %d, CRC not match %x %x\r\n", index[cnt],crc[cnt],CRC_check(data+i+2,packet_len[cnt]-4));
			}
			cnt++;
		}
	}
	
}
void server_proc(void* parameter)
{	
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	rt_uint8_t *tmp_buf = RT_NULL;
	while (1) {
		rt_err_t r = rt_data_queue_pop(&g_data_queue[3], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		rt_kprintf("<<%d ",data_size);
		tmp_buf = (rt_uint8_t *)rt_malloc((data_size/2)*sizeof(rt_uint8_t));
		for (int i=0; i<data_size;i=i+2)
		{
			if (((char *)last_data_ptr)[i] >= 'a' && ((char *)last_data_ptr)[i] <= 'z')
			{
				tmp_buf[i/2]=((char *)last_data_ptr)[i]-'a'+10;
			}
			else if (((char *)last_data_ptr)[i] >= 'A' && ((char *)last_data_ptr)[i] <= 'Z')
			{
				tmp_buf[i/2]=((char *)last_data_ptr)[i]-'A'+10;
			}
			else if (((char *)last_data_ptr)[i] >= '0' && ((char *)last_data_ptr)[i] <= '9')
			{
				tmp_buf[i/2]=((char *)last_data_ptr)[i]-'0';
			}
			
			if (((char *)last_data_ptr)[i+1] >= 'a' && ((char *)last_data_ptr)[i+1] <= 'z')
			{
				tmp_buf[i/2]=(tmp_buf[i/2] << 4)|(((char *)last_data_ptr)[i+1]-'a'+10);
			}
			else if (((char *)last_data_ptr)[i+1] >= 'A' && ((char *)last_data_ptr)[i+1] <= 'Z')
			{
				tmp_buf[i/2]=(tmp_buf[i/2] << 4)|(((char *)last_data_ptr)[i+1]-'A'+10);
			}
			else if (((char *)last_data_ptr)[i+1] >= '0' && ((char *)last_data_ptr)[i+1] <= '9')
			{
				tmp_buf[i/2]=(tmp_buf[i/2] << 4)|(((char *)last_data_ptr)[i+1]-'0');
			}
			rt_kprintf("%x ", tmp_buf[i/2]);
		}
		rt_kprintf(">>\r\n");
		handle_server((rt_uint8_t *)tmp_buf,data_size/2);
		rt_free(tmp_buf);
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
	char login[45] = {0};
	gprs_wait_event(RT_WAITING_FOREVER);
	while(1)	{
		rt_thread_delay(500);
		rt_mutex_take(&(g_pcie[g_index]->lock),RT_WAITING_FOREVER);
		login[0]=0xad;
		login[1]=0xac;
		login[2]=0x00;//len
		login[3]=0x12;
		login[4]=0x00;
		login[5]=0x00;//login
		login[6]=0x01;		
		login[7]=0x00;//v
		login[8]=0x00;
		memcpy(login+9,mp.roProperty.sn,6);
		login[15]=g_pcie[g_index]->csq;
		login[16]=0x4d;
		login[17]=0x4e;
		login[18]=0x4f;
		login[19]=0x50;
		login[20]=0x51;
		login[21]=0x52;
		login[22]=0x20|0x06;
		memcpy(login+23,g_pcie[g_index]->qccid,10);
		memcpy(login+33,g_pcie[g_index]->imei,8);
		login[3]=43;
		rt_uint16_t crc = CRC_check(login+2,39);
		rt_kprintf("crc is %x\r\n",crc);
		login[41]=(crc>>8)&0xff;
		login[42]=crc&0xff;
		for(i=0;i<43;i++)
			rt_kprintf("login[%02d] = 0x%02x\r\n",i,login[i]);
		
		rt_data_queue_push(&g_data_queue[2], login, 43, RT_WAITING_FOREVER);
		gprs_wait_event(RT_WAITING_FOREVER);	
		rt_mutex_release(&(g_pcie[g_index]->lock));
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
	rt_thread_startup(rt_thread_create("gprs_send",send_process, 0,1024, 20, 10));
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

