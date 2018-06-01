#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
//#include "bc26.h"
#include "ec20.h"
//#include "nb_iot.h"
//#include "ip_module.h"
#include "pcie.h"
#include "master.h"
#include "bsp_misc.h"
#include "prop.h"
rt_uint8_t g_type0 = 0;
rt_uint8_t g_type1 = 0;
int g_index = 0;
rt_uint8_t flow_cnt = 0;
extern struct rt_event g_info_event;
extern rt_uint16_t g_bat;
extern rt_uint8_t g_ac;
extern rt_uint8_t 	cur_status;
rt_uint8_t g_net_state = NET_STATE_UNKNOWN;
extern rt_uint8_t heart_time;
rt_uint8_t g_exit_reason=0;
rt_uint16_t g_alarm_reason;
rt_uint8_t g_alarm_fq;
rt_uint16_t g_main_event_code;
rt_uint8_t g_operate_platform=0x0;
rt_uint8_t g_operater[6]={0x00};
rt_uint16_t g_sub_event_code;
rt_uint8_t g_fq_len;
rt_uint8_t g_fq_event[10];
rt_uint8_t g_addr_type;
rt_uint8_t g_heart_cnt=0;
extern rt_uint8_t heart_time;
extern rt_uint8_t r_signal;
extern rt_uint16_t battery;
extern rt_uint8_t g_module_type;
extern rt_uint8_t need_read;
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
	if (buf == RT_NULL)
		rt_kprintf("pcie 1 malloc failed\r\n");
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
			if (rcv == RT_NULL) {
				rt_kprintf("no memory\r\n");
				while((rcv = (uint8_t *)rt_malloc(total_len+1)) == RT_NULL)
					rt_thread_delay(10);
			}
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
	if (buf == RT_NULL)
		rt_kprintf("pcie 0 malloc failed\r\n");
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
			
			if (rcv == RT_NULL) {
				rt_kprintf("no memory\r\n");
				while((rcv = (uint8_t *)rt_malloc(total_len+1)) == RT_NULL)
					rt_thread_delay(10);
			}
			rt_memcpy(rcv, buf, total_len);
			rcv[total_len] = '\0';
			rt_data_queue_push(&g_data_queue[0], rcv, total_len, RT_WAITING_FOREVER);
			total_len = 0;
		}
	}
}
#ifdef M26_INIT
extern rt_device_t g_dev_m26;
#endif
void pcie1_sm(void* parameter)
{
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	rt_kprintf("pcie1 sm %x\r\n", g_type1);
	#ifdef M26_INIT
	rt_thread_delay(1000);
	gprs_at_cmd(g_dev_m26,"AT+IPR=115200;&W\r\n");
	rt_thread_delay(200);
	gprs_at_cmd(g_dev_m26,"AT&W\r\n");
	rt_thread_delay(200);
	#endif
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
					ec20_proc(last_data_ptr, data_size);
					break;
				case PCIE_2_NBIOT:
					//bc26_proc(last_data_ptr, data_size);
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
					//bc26_proc(last_data_ptr, data_size);
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
	rt_uint16_t packet_len[20];
	rt_uint8_t index[20];
	rt_uint16_t crc[20];
	for (i=0; i<len; i++)
	{
		//rt_kprintf("data %x\r\n",data[i]);
		if (data[i] == 0xAD && data[i+1] == 0xAC)
		{
			//rt_kprintf("we got ADAC %d\r\n",i);
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
			if (packet_len[cnt] <= 4) {
				rt_kprintf("len is not correct\r\n");
				return 0;
			}
			crc[cnt]=(data[i+packet_len[cnt]-2]<<8)|data[i+packet_len[cnt]-1];
			rt_kprintf("Found 0xAD 0xAC at %d, len %d, crc %x\r\n", index[cnt],packet_len[cnt],crc[cnt]);
			if (crc[cnt] == CRC_check(data+i+2,packet_len[cnt]-4))
			{
				//rt_kprintf("packet %d, CRC is match\r\n",index[cnt]);
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
		#if 0
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
		#else
		for (int i=0;i<data_size;i++)
			rt_kprintf("%02x ", ((char *)last_data_ptr)[i]);
		rt_kprintf(">>\r\n");
		handle_server((rt_uint8_t *)last_data_ptr,data_size);
		#endif
		rt_free((void *)last_data_ptr);
	}
}
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &(g_pcie[g_index]->event), GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}
int build_cmd(rt_uint8_t *cmd,rt_uint16_t type)
{	
	int i=0,ofs;
	static rt_uint8_t heart_type = 0;
	rt_uint8_t zero_array[10]={0};
	rt_time_t cur_time;		
	time(&cur_time);
	flow_cnt++;
	cmd[0]=0xad;
	cmd[1]=0xac;
	cmd[2]=0x00;//len
	cmd[3]=0x12;
	cmd[4]=flow_cnt;
	cmd[7]=(PROTL_V>>8) & 0xff;//v
	cmd[8]=PROTL_V&0xff;
	memcpy(cmd+9,mp.roProperty.sn,6);
	rt_kprintf("\r\n******************************************************\r\n");
	if (type == CMD_LOGIN) {
		rt_kprintf("req\t\tCMD LOGIN Packet\r\n");
		cmd[5]=(CMD_LOGIN >> 8) & 0xff;//login
		cmd[6]=CMD_LOGIN&0xff;		
		cmd[15]=g_pcie[g_index]->csq;
		cmd[16]='W';
		cmd[17]='E';
		cmd[18]='A';
		cmd[19]='D';
		cmd[20]='F';
		cmd[21]='E'; 	
		memcpy(cmd+22,g_pcie[g_index]->imei,8);
		cmd[30] = 0;
		ofs = 31;
		//rt_kprintf("login %d %d %d\r\n", g_index,g_type1,g_type0);
		if ((g_index == 1 && g_type1 == PCIE_2_M26) ||
			(g_index == 0 && g_type0 == PCIE_1_M26))
		{	
			cmd[30] |= 0x20; 
			rt_kprintf("interface\tm26\r\n");
		}
		if ((g_index == 1 && g_type1 == PCIE_2_EC20) ||
			(g_index == 0 && g_type0 == PCIE_1_EC20))
		{
			cmd[30] |= 0x10;
			rt_kprintf("interface\tec20\r\n");
		}
		if (g_pcie[g_index]->lac_ci !=0 )
		{
			cmd[30] |= 0x08;
			cmd[ofs++] = (g_pcie[g_index]->lac_ci>>24)&0xff;
			cmd[ofs++] = (g_pcie[g_index]->lac_ci>>16)&0xff;
			cmd[ofs++] = (g_pcie[g_index]->lac_ci>>8)&0xff;
			cmd[ofs++] = (g_pcie[g_index]->lac_ci>>0)&0xff;
			rt_kprintf("lac_ci\t\t%08x\r\n",g_pcie[g_index]->lac_ci);
		}
		if (rt_memcmp(g_pcie[g_index]->qccid,zero_array,10) !=0 &&
			rt_memcmp(g_pcie[g_index]->qccid,mp.qccid,10) !=0 )
		{
			rt_memcpy(mp.qccid, g_pcie[g_index]->qccid, 10);
			rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
			cmd[30] |= 0x04;
			memcpy(cmd+ofs,g_pcie[g_index]->qccid,10);
			ofs+=10;
			rt_kprintf("qccid\t\t%010x\r\n",g_pcie[g_index]->qccid);
		}
		if (cur_status)
			cmd[ofs++] = 2;
		else
			cmd[ofs++] = 1;
		need_read = 1;
		rt_kprintf("csq\t\t%d\r\n",	cmd[15]);
		rt_kprintf("imei\t\t%02x%02x%02x%02x%02x%02x%02x%02x\r\n",cmd[22],cmd[23],cmd[24],cmd[25],cmd[26],cmd[27],cmd[28],cmd[29]);
	} else if(type == CMD_HEART) {			
		rt_kprintf("req\t\tCMD HEART Packet\r\n");
		cmd[5]=(CMD_HEART >> 8) & 0xff;//heart
		cmd[6]=CMD_HEART&0xff;		
		if (cur_status)
			cmd[15] = 0x20;
		else
			cmd[15] = 0x10;
		if (g_ac)
			cmd[15] |= 0x01; 
		cmd[16]=g_pcie[g_index]->csq;
		cmd[17] = g_bat % 256;
		if (heart_type == 0) {
			cmd[18] = 0x01;
			cmd[19] = mp.firmVersion >> 8;
			cmd[20] = mp.firmVersion & 0xff;
			heart_type = 1;
			rt_kprintf("addr_type\tAPP %d\r\n",mp.firmVersion);
		} else if (heart_type == 1) {
			cmd[18] = 0x02;
			cmd[19] = mp.socketAddressVersion >> 8;
			cmd[20] = mp.socketAddressVersion & 0xff;
			heart_type = 2;
			rt_kprintf("addr_type\tSocketIP %d\r\n",mp.socketAddressVersion);
		} else if (heart_type == 2) {
			cmd[18] = 0x03;
			cmd[19] = mp.updateAddressVersion >> 8;
			cmd[20] = mp.updateAddressVersion & 0xff;
			heart_type = 0;
			rt_kprintf("addr_type\tUpgradeIP %d\r\n",mp.updateAddressVersion);
		}
		ofs = 21;
		need_read = 1;
		rt_kprintf("status\t\t%s\r\n", (cur_status==0)?"chefang":"bufang");
		rt_kprintf("power\t\t%s\r\n", (g_ac==1)?"ext power":"battery");
		rt_kprintf("csq\t\t%d\r\n", cmd[16]);
		rt_kprintf("battery\t\t%d\r\n", cmd[17]);
	}else if(type == CMD_EXIT) {
		rt_kprintf("\r\n<CMD EXIT Packet>\r\n");
		cmd[5]=(CMD_EXIT >> 8) & 0xff;//exit
		cmd[6]=CMD_EXIT&0xff;		
		cmd[15] = g_exit_reason;
		ofs= 16;
	} else if(type == CMD_ALARM) {		
		rt_kprintf("req\t\tCMD ALARM Packet\r\n");
		cmd[5]=(CMD_ALARM >> 8) & 0xff;//alarm
		cmd[6]=CMD_ALARM&0xff;		
		cmd[15] = (g_alarm_reason >> 8) & 0xff;
		cmd[16] = (g_alarm_reason) & 0xff;
		cmd[17] = (cur_time >> 24) & 0xff;
		cmd[18] = (cur_time >> 16) & 0xff;
		cmd[19] = (cur_time >>  8) & 0xff;
		cmd[20] = (cur_time >>  0) & 0xff;
		cmd[21] = g_alarm_fq;
		cmd[22] = (battery>>8) & 0xff;
		cmd[23] = (battery) & 0xff;
		cmd[24] = con_rssi(r_signal);
		ofs= 25;
		need_read = 1;
		rt_kprintf("alarm\t\t%02x%02x\r\n", cmd[15],cmd[16]);
		rt_kprintf("time\t\t%s\r\n",ctime(&cur_time));
		rt_kprintf("alarm fq\t\t%d\r\n", g_alarm_fq);
		rt_kprintf("battery\t\t%d\r\n",battery);
		rt_kprintf("signal\t\t%d\r\n",cmd[24]);
	} else if (type == CMD_MAIN_EVENT) {
		rt_kprintf("req\t\tCMD MAIN EVENT Packet\r\n");
		cmd[5]=(CMD_MAIN_EVENT >> 8) & 0xff;//main event
		cmd[6]=CMD_MAIN_EVENT&0xff;
		cmd[15] = (g_main_event_code >> 8) & 0xff;
		cmd[16] = (g_main_event_code) & 0xff;
		cmd[17] = (cur_time >> 24) & 0xff;
		cmd[18] = (cur_time >> 16) & 0xff;
		cmd[19] = (cur_time >>  8) & 0xff;
		cmd[20] = (cur_time >>  0) & 0xff;
		cmd[21] = g_operate_platform;
		memcpy(cmd+22,g_operater,6);
		ofs = 28;
		need_read = 1;
		rt_kprintf("event code\t%04x\r\n",g_main_event_code);
		rt_kprintf("time\t\t%s\r\n",ctime(&cur_time));
		rt_kprintf("platform\t\t%x\r\n",g_operate_platform);
		rt_kprintf("operator\t\t%02x%02x%02x%02x%02x%02x\r\n",
			cmd[22],cmd[23],cmd[24],cmd[25],cmd[26],cmd[27]);
	} else if (type == CMD_SUB_EVENT) {
		rt_kprintf("\r\n<CMD SUB EVENT Packet>\r\n");
		cmd[5]=(CMD_SUB_EVENT >> 8) & 0xff;//main event
		cmd[6]=CMD_SUB_EVENT&0xff;
		cmd[15] = (g_sub_event_code >> 8) & 0xff;
		cmd[16] = (g_sub_event_code) & 0xff;
		cmd[17] = (cur_time >> 24) & 0xff;
		cmd[18] = (cur_time >> 16) & 0xff;
		cmd[19] = (cur_time >>  8) & 0xff;
		cmd[20] = (cur_time >>  0) & 0xff;		
		cmd[21] = g_fq_len;
		ofs = 22;
		if (cmd[21] == 1)
		{
			cmd[ofs++] = g_fq_event[0];
		} else {
			memcpy(cmd+ofs,g_fq_event,10);
			//for (i=0;i<8;i++)
			//rt_kprintf("cmd %02x , event %02x\r\n",cmd[i+22],g_fq_event[i]);
			ofs+=10;
		}
		cmd[ofs++] = g_operate_platform;
		memcpy(cmd+ofs,g_operater,6);
		ofs += 6;
		if ((g_sub_event_code == 0x2001||
			g_sub_event_code == 0x2002) &&
			(g_operate_platform == 0xff ||
			g_operate_platform == 0xfe))
			{				
				cmd[ofs++] = (battery>>8) & 0xff;
				cmd[ofs++] = (battery) & 0xff;
				cmd[ofs++] = con_rssi(r_signal);
			}
		need_read = 1;
	} else if (type == CMD_ASK_ADDR) {
		rt_kprintf("req\t\tCMD ASK ADDR Packet\r\n");
		cmd[5] = (CMD_ASK_ADDR >> 8) & 0xff;//ask addr
		cmd[6] = CMD_ASK_ADDR&0xff;
		cmd[15]= g_addr_type;
		ofs = 16;
		need_read = 1;
		rt_kprintf("addr type\t\t%d\r\n",g_addr_type);
	} else if (type == CMD_ASK_SUB_ACK) {
		rt_kprintf("req\t\tCMD ASK SUB ADDR Packet\r\n");
		cmd[5] = (CMD_ASK_SUB_ACK >> 8) & 0xff;//ask addr
		cmd[6] = CMD_ASK_SUB_ACK&0xff;
		cmd[15]= fqp.delya_out;
		cmd[16]= fqp.delay_in;
		cmd[17]=0;		
		ofs=18;		
		/*store fq list*/
		for(i=0;i<WIRE_MAX;i++)
		{
			if(fangqu_wire[i].index != 0) {
				cmd[ofs++] = fangqu_wire[i].index;
				cmd[ofs] = 0x40;
				if (fangqu_wire[i].operationType==TYPE_24)
					cmd[ofs]|=0x20;
				else if (fangqu_wire[i].operationType==TYPE_DELAY)
					cmd[ofs]|=0x10;
				cmd[ofs++]|=fangqu_wire[i].alarmType;
				cmd[ofs]=0;
				if (fangqu_wire[i].voiceType)
					cmd[ofs]|=0x80;
				if (fangqu_wire[i].status)
					cmd[ofs]|=0x20;
				if (fangqu_wire[i].slave_delay)
					cmd[ofs]|=0x40;			
				if (fangqu_wire[i].isBypass)
					cmd[ofs]|=0x10;
				ofs++;
				cmd[ofs++] = fangqu_wire[i].slave_type;
				(cmd[17])++;
			}
		}
		for(i=0;i<WIRELESS_MAX;i++)
		{
			if(fangqu_wireless[i].index != 0) {
				cmd[ofs++] = fangqu_wireless[i].index;
				cmd[ofs] = 0x80;
				if (fangqu_wireless[i].operationType==TYPE_24)
					cmd[ofs]|=0x20;
				else if (fangqu_wireless[i].operationType==TYPE_DELAY)
					cmd[ofs]|=0x10;
				cmd[ofs++]|=fangqu_wireless[i].alarmType;
				cmd[ofs]=0;
				if (fangqu_wireless[i].voiceType)
					cmd[ofs]|=0x80;
				if (fangqu_wireless[i].status)
					cmd[ofs]|=0x20;
				if (fangqu_wireless[i].slave_delay)
					cmd[ofs]|=0x40;			
				if (fangqu_wireless[i].isBypass)
					cmd[ofs]|=0x10;
				ofs++;
				cmd[ofs++] = fangqu_wireless[i].slave_type;
				(cmd[17])++;
			}
		}		
		cmd[ofs++] = g_operate_platform;
		memcpy(cmd+ofs,g_operater,6);
		ofs += 6;
	} else if (type == CMD_ASK_MAIN_ACK) {	
		rt_kprintf("req\t\tCMD ASK MAIN ADDR Packet\r\n");
		cmd[5] = (CMD_ASK_MAIN_ACK >> 8) & 0xff;//ask addr
		cmd[6] = CMD_ASK_MAIN_ACK&0xff;
		cmd[15]= (fqp.alarm_voice_time<<4)|(fqp.audio_vol&0x0f);
		cmd[16]= fqp.is_lamp<<4;
		cmd[17]=(fqp.PGM0<<4)|(fqp.PGM1&0x0f);
		cmd[18]=0x00;
		if (fqp.is_check_AC)
		cmd[18]|= 0x80;
		if (fqp.is_check_DC)
			cmd[18]|=0x40;
		if (fqp.is_alarm_voice)
			cmd[18]|=0x20;
		cmd[19]=(fqp.auto_bufang>>24)&0xff;
		cmd[20]=(fqp.auto_bufang>>16)&0xff;
		cmd[21]=(fqp.auto_bufang>>8)&0xff;
		cmd[22]=(fqp.auto_bufang>>0)&0xff;
		cmd[23]=(fqp.auto_chefang>>24)&0xff;
		cmd[24]=(fqp.auto_chefang>>16)&0xff;
		cmd[25]=(fqp.auto_chefang>>8)&0xff;
		cmd[26]=(fqp.auto_chefang>>0)&0xff;
		ofs=27;		
		cmd[ofs++] = g_operate_platform;
		memcpy(cmd+ofs,g_operater,6);
		ofs += 6;
		rt_kprintf("voice time\t\t%d\r\n",fqp.alarm_voice_time);
		rt_kprintf("vol\t\t%d\r\n",fqp.audio_vol);
		rt_kprintf("is_lamp\t\t%d\r\n",fqp.is_lamp);
		rt_kprintf("pgm0,1\t\t%d\r\n",cmd[17]);
		rt_kprintf("check ac\t\t%d\r\n",fqp.is_check_AC);
		rt_kprintf("check dc\t\t%d\r\n",fqp.is_check_DC);
		rt_kprintf("voice switch\t\t%d\r\n",fqp.is_alarm_voice);
		rt_kprintf("auto_bufang\t%04x\r\n",fqp.auto_bufang);
		rt_kprintf("auto_chefang\t%04x\r\n",fqp.auto_chefang);
		rt_kprintf("platform\t\t%x\r\n",g_operate_platform);		
		rt_kprintf("operator\t\t%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
			g_operater[0],g_operater[1],g_operater[2],g_operater[3],
			g_operater[4],g_operater[5]);
	}
	//rt_kprintf("ofs is %d\r\n", ofs);
	cmd[3]=ofs+2;
	rt_uint16_t crc = CRC_check(cmd+2,ofs-2);
	//rt_kprintf("crc is %x\r\n",crc);
	cmd[ofs++]=(crc>>8)&0xff;
	cmd[ofs++]=crc&0xff;
	for(i=0;i<ofs;i++)
		rt_kprintf("%02x ",cmd[i]);
	
	if (flow_cnt == 255)
		flow_cnt=0;
	net_flow();
	rt_kprintf("\r\n******************************************************\r\n");
	return ofs;
}
void send_process(void* parameter)
{
	char *cmd = {0};
	int send_len = 0;
	
	while(1)	{
		rt_kprintf("wait for event\r\n");
		gprs_wait_event(RT_WAITING_FOREVER);
		rt_kprintf("wait lock\r\n");
		rt_mutex_take(&(g_pcie[g_index]->lock),RT_WAITING_FOREVER);		
		cmd = (char *)rt_malloc(50);
		if (cmd == RT_NULL)
			rt_kprintf("build cmd failed\r\n");
		rt_kprintf("begin send cmd, %d\r\n",g_net_state);
		if (g_net_state == NET_STATE_INIT) {
			SetStateIco(7,1);
			rt_kprintf("send login\r\n");
			send_len = build_cmd(cmd,CMD_LOGIN);
			g_net_state = NET_STATE_LOGIN;
			heart_time = 0;
		} else if (g_net_state == NET_STATE_LOGED && heart_time == 60) {
			send_len = build_cmd(cmd,CMD_HEART);
			rt_kprintf("heart cnt %d\r\n",g_heart_cnt);
			g_heart_cnt++;
			if (g_heart_cnt > 6) {
				g_heart_cnt=0;
				g_net_state = NET_STATE_UNKNOWN;
				pcie_switch(g_module_type);
			}
		} else {
			heart_time = 0;
			rt_free(cmd);
			rt_mutex_release(&(g_pcie[g_index]->lock));
			rt_kprintf("unknown state ,ignore\r\n");
			continue;
		}
		heart_time = 0;
		rt_data_queue_push(&g_data_queue[2], cmd, send_len, RT_WAITING_FOREVER);
		//gprs_wait_event(RT_WAITING_FOREVER);
		rt_mutex_release(&(g_pcie[g_index]->lock));
	}	
}
void upload_server(rt_uint16_t cmdType)
{
	char *cmd = {0};
	//if (g_net_state != NET_STATE_LOGED)
	//	return;
	rt_mutex_take(&(g_pcie[g_index]->lock),RT_WAITING_FOREVER);
	cmd = (char *)rt_malloc(400);
	if (cmd != RT_NULL) {
	int send_len = build_cmd(cmd,cmdType);
	rt_kprintf("send cmd %d to server\r\n",cmdType);
	rt_data_queue_push(&g_data_queue[2], cmd, send_len, RT_TICK_PER_SECOND);
//	gprs_wait_event(RT_WAITING_FOREVER);	
	rt_mutex_release(&(g_pcie[g_index]->lock));
	rt_kprintf("send buf done\r\n");
	} else {
		rt_kprintf("not enough mem\r\n");
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
		rt_memset(g_pcie[0],0,sizeof(pcie_param));
		g_pcie[0]->dev = rt_device_find("uart3"); //PCIE1
		rt_device_open(g_pcie[0]->dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
		rt_event_init(&(g_pcie[0]->event), 	"pcie0_event", 	RT_IPC_FLAG_FIFO );
		rt_mutex_init(&(g_pcie[0]->lock), 	"pcie0_lock", 	RT_IPC_FLAG_FIFO);
		rt_sem_init(&(g_pcie[0]->sem), 		"pcie0_sem", 	0, 0);
	}

	if (type1) {
		g_pcie[1] = (ppcie_param)rt_malloc(sizeof(pcie_param));
		rt_memset(g_pcie[1],0,sizeof(pcie_param));
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
		rt_thread_startup(rt_thread_create("1pcie0",pcie0_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("2pcie0", pcie0_sm,  0,2048, 20, 10));
	}
	if (type1) {
		rt_device_set_rx_indicate(g_pcie[1]->dev, pcie1_rx_ind);
		rt_thread_startup(rt_thread_create("3pcie1",pcie1_rcv, 0,1524, 15, 10));
		rt_thread_startup(rt_thread_create("4pcie1", pcie1_sm,  0,2048, 20, 10));
	}
	rt_thread_startup(rt_thread_create("5serv",server_proc, 0,2048, 15, 10));
	rt_thread_startup(rt_thread_create("6gprs",send_process, 0,2048, 20, 10));
	return 1;
}
void switch_pcie_power(rt_uint8_t type)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	if (type == 0) {
		GPIO_WriteBit(GPIOD,GPIO_Pin_6,Bit_RESET);
		GPIO_WriteBit(GPIOD,GPIO_Pin_5,Bit_SET);
	} else {
		GPIO_WriteBit(GPIOD,GPIO_Pin_6,Bit_SET);
		GPIO_WriteBit(GPIOD,GPIO_Pin_5,Bit_RESET);
	}
}
rt_uint8_t pcie_switch(rt_uint8_t type)
{
	g_module_type = type;
	switch (type) 
	{
		case PCIE_1_IP:
			//ip_module_start(0);
			break;
		case PCIE_1_M26:
			switch_pcie_power(0);
			m26_start(0);
			break;
		case PCIE_1_EC20:
			switch_pcie_power(0);
			ec20_start(0);
			break;
		case PCIE_1_NBIOT:
			//nb_iot_start(0);
			break;
		case PCIE_2_IP:
			//ip_module_start(1);
			break;
		case PCIE_2_M26:
			switch_pcie_power(1);
			m26_start(1);
			break;
		case PCIE_2_EC20:
			switch_pcie_power(1);
			ec20_start(1);
			break;
		case PCIE_2_NBIOT:
			//nb_iot_start(1);
			break;
		default:
			rt_kprintf("uninsert module on pcie\r\n");
			break;
	}	
}
rt_uint8_t check_type(rt_uint8_t pcie_index)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t pin0,pin1,pin2;
	uint32_t port;
	rt_uint8_t result=0;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	if (pcie_index == 0) {
		port = GPIOA;
		pin0 = GPIO_Pin_8;pin1 = GPIO_Pin_9;pin2 = GPIO_Pin_10;
	} else {
		port = GPIOE;
		pin0 = GPIO_Pin_10;pin1 = GPIO_Pin_11;pin2 = GPIO_Pin_12;
	}

	if (!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin0) &&
		!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin1) &&
		GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin2))
		result = PCIE_1_EC20;
	else if(!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin0) &&
		GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin1) &&
		!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin2))
		result = PCIE_1_M26;
	else if(!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin0) &&
		!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin1) &&
		!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin2))
		result = PCIE_1_IP;
	else if(!GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin0) &&
		GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin1) &&
		GPIO_ReadInputDataBit(((GPIO_TypeDef *)port),pin2))
		result = PCIE_1_NBIOT;

	if (pcie_index == 1)
		result = (result << 4);
	rt_kprintf("PCIE index %d, type %x\r\n", pcie_index,result);
	return result;
}
rt_uint8_t check_pcie(rt_uint8_t num) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if (num == 0) {
		/*pcie1_cd pc6*/		
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == SET)
			return 0;
	} else {
		/*pcie2_cd pe15*/		
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_15) == SET)
			return 0;
	}
	return check_type(num);
}

