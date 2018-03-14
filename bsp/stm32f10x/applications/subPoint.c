#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "cc1101.h"
#include "subPoint.h"
#include "master.h"
#include "bsp_misc.h"
#include "lcd.h"
#define MSG_HEAD0		0x6c
#define MSG_HEAD1		0xaa
rt_uint8_t cur_status = 0;//protect off 1 for on
rt_uint8_t stm32_id[6] = {0xa1,0x18,0x01,0x02,0x12,0x34};
rt_uint8_t stm32_zero[6] = {0};
rt_uint32_t sub_id = {0}; 
rt_uint16_t command_type = 0;
rt_uint8_t sub_cmd_type = 0;
rt_uint16_t dev_model = 0;
rt_uint32_t dev_time = 0;
rt_uint8_t dev_type = 0;
rt_uint16_t battery = 0;
rt_uint8_t in_fq = 0;
rt_uint32_t g_addr[60] = {0x00};
rt_uint8_t g_index_sub = 0;
rt_uint8_t base_addr = 2;
rt_uint8_t addr_cnt = 0;
rt_uint8_t g_main_state = 0; /*0 normal , 1 code, 2 factory reset*/
extern rt_uint32_t g_coding_cnt;
extern struct rt_event g_info_event;
extern rt_uint8_t g_num;
rt_uint8_t s1=0;
char *cmd_type(rt_uint16_t type)
{
	switch (type) {
		case 0x0000:
			return "Register";
		case 0x0014:
			return "Confirm";
		case 0x0006:
			return "Alarm";
		case 0x000c:
			return "Low Power";
		case 0x0010:
			return "Cur Status";
		case 0x000e:
			return "Remoter Mute";
		case 0x0002:
			//cur_status = 1;
			return "Remoter Protect ON";
		case 0x0004:
			//cur_status = 0;
			return "Remoter Protect OFF";
//		case 0x0006:
//			return "Remoter Alarm";			
		default:
			return "UnKnown";
	}
	return "UnKnown";
}
char *cmd_sub_type(rt_uint8_t type)
{
	switch (type) {
		case 0x01:
			return "Normal";
		case 0x02:
			return "S1";
		default:
			return "UnKnown";
	}
	return "UnKnown";
}
char *cmd_dev_type(rt_uint8_t type)
{
	switch (type) {
		case 0x01:
			return "Wireless Remoter";
		case 0x42:
			return "Wireless Infrar";
		case 0x43:
			return "Wireless Door";
		case 0x22:
			return "Wire Infrar";
		case 0x23:
			return "Wire Door";
		default:
			return "UnKnown";
	}
	return "UnKnown";
}
char get_addr(rt_uint32_t subId, struct FangQu *list, int len)
{
	int i;
	for (i=0;i<len;i++)
	{
		if (subId == list[i].slave_sn)
			return list[i].index;
	}

	if (i==len)
	{
		i=0;
		while(list[i].index != 0)
			i++;
	}
	if (len == 50)
		return i+2;
	else
		return i+51;
}
void save_fq(struct FangQu *list, int len)
{
	int i;	
	g_coding_cnt =0;
	for (i=0;i<len;i++)
	{
		rt_kprintf("salve sn %d, sub id %d\r\n",
			list[i].slave_sn,sub_id);
		if (list[i].slave_sn== sub_id)
		{
			if (len!=50)
				in_fq = i+51;//SetErrorCode(i+51);
			else
				in_fq = i+2;//SetErrorCode(i+2);
			g_num = in_fq;
			rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);
			/*play audio here*/
			return;
		}
	}
	for (i=0;i<len;i++)
	{
		if (list[i].index == 0)
		{
			if (len == 50)
				list[i].index = i+2;
			else
				list[i].index = i+51;
			list[i].slave_sn = sub_id;
			list[i].slave_type = dev_type;
			list[i].slave_model = dev_model;
			list[i].slave_batch = dev_time&0x00ffffff;
			list[i].type =2;
			list[i].voiceType =1;
			if (sub_id == 0x0a) {
				list[i].alarmType = 0x00;
				list[i].operationType= 0x00;
			} else if (sub_id == 0x0b) {
				list[i].alarmType = 0x00;
				list[i].operationType= 0x00;
			} else if (sub_id == 0x0c) {
				list[i].alarmType = 0x02;
				list[i].operationType= 0x02;
			} else if (sub_id == 0x0d) {
				list[i].alarmType = 0x02;
				list[i].operationType= 0x02;
			} else if (sub_id == 0x0e) {
				list[i].alarmType = 0x00;
				list[i].operationType= 0x01;
			}
			in_fq=list[i].index;
			rt_kprintf("save fq to %d , index %d, sn %08x\r\n",
				i,list[i].index,list[i].slave_sn);
			//save_param(1);
			rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
			//if (len!=50)
			//	SetErrorCode(i+51);
			//else
			//	SetErrorCode(i+2);
			g_num = in_fq;
			rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);
			/*play audio here*/
			return;
		}
	}
	/*play wrong audio here*/
}
void cmd_dump(rt_uint8_t *data)
{
	/*check subdevice id = local device id*/
	memcpy(stm32_id, data+5, 6);
	sub_id= data[11]<<24|data[12]<<16|data[13]<<8|data[14];
	command_type = data[15]<<8|data[16];
	rt_kprintf("<== \r\nSTM32 ID :\t%02x%02x%02x%02x%02x%02x\r\n", data[5],data[6],data[7],data[8],data[9],data[10]);
	rt_kprintf("Sub ID :\t%02x%02x%02x%02x\r\n", data[11],data[12],data[13],data[14]);
	rt_kprintf("CMD Type :\t%s\r\n",cmd_type(command_type));
	if (0x0002 != command_type && 0x0004 != command_type &&
		0x000e != command_type) /*from remoter or coding request ,dump dev_type, model, time*/
	{
		if (0x0000 == command_type)
		{
			dev_type = data[17];
			battery = data[18]<<8|data[19];
		}
		else if(0x0014 == command_type)
		{
			dev_type = data[19];
			dev_model = (data[20]<<8)|data[21];
			dev_time = (data[22]<<16)|(data[23]<<8)|data[24];
			battery = data[25]<<8|data[26];
			rt_kprintf("Dev Model :\t%04x\r\n", dev_model);
			rt_kprintf("Dev build time :%06x\r\n", dev_time);
		}
		else
		{
			dev_type = data[18];
			sub_cmd_type = data[17];			
			battery = data[19]<<8|data[20];
			rt_kprintf("CMD SubType :\t%s\r\n",cmd_sub_type(data[17]));
		}
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(dev_type));
	}
	else
	{
		dev_type = data[17];
		dev_model = (data[18]<<8)|data[19];
		dev_time = (data[20]<<16)|(data[21]<<8)|data[22];
		battery = data[23]<<8|data[24];
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(dev_type));
		rt_kprintf("Dev Model :\t%04x\r\n", dev_model);
		rt_kprintf("Dev build time :%06x\r\n", dev_time);
	}
	in_fq = data[1];
	rt_kprintf("Battery :\t%d\r\n",battery);
	rt_kprintf("Protect Zone :\t%02x\r\n", data[1]);

	rt_kprintf("STM32 Param sn: %02x%02x%02x%02x%02x%02x\r\n", 
		mp.roProperty.sn[0],
		mp.roProperty.sn[1],
		mp.roProperty.sn[2],
		mp.roProperty.sn[3],
		mp.roProperty.sn[4],
		mp.roProperty.sn[5]
		);
	rt_kprintf("g_main_state %d\r\n",g_main_state);
}

int check_sub_id(rt_uint32_t sub_mid,struct FangQu *list, int len)
{
	int i;
	for (i=0; i<len; i++)
	{
		if (sub_mid == list[i].slave_sn)
		{
			g_index_sub = i;
			return 1;
		}
	}
	return 0;
}
void handleSub(rt_uint8_t *data)
{
	rt_uint8_t resp[32] = {0};
	rt_uint32_t len;
	if (data == RT_NULL ||  len == 0)
	{
		rt_kprintf("invalid param\r\n");
		return;
	}
	
	if (data[2] != MSG_HEAD0 || data[3] != MSG_HEAD1)
	{
		rt_kprintf("invalid header\r\n");
		return ;
	}
	
	len = data[4];
	unsigned short crc = CRC_check(data, len+3);
	/*check crc*/
	if (crc != (data[len+3] << 8 | data[len+4]))
	{
		rt_kprintf("crc failed %x %x\r\n", crc, (data[len+3] << 8 | data[len+4]));
		return ;
	}
	cmd_dump(data);
	/*check stm32 id == mp.sn || stm32 id == 0*/
	if (memcmp(stm32_id, mp.roProperty.sn, 6) != 0 
		&& memcmp(stm32_id, stm32_zero, 6) != 0)
	{
		rt_kprintf("command not for me\r\n"); 
		return ;
	}
	/*check sub id in fangqu list && non-coding mode*/
	if (!check_sub_id(sub_id, fangqu_wireless,WIRELESS_MAX)
		&& g_main_state != 1) {
		rt_kprintf("sub id not in list , and not codeing mode\r\n");
		return ;
	}
	if (dev_type != 0x01) {
		resp[0] = data[1];
		resp[1] = data[0];
		rt_memcpy(resp+2, data+2,13);
		rt_memcpy(resp+5, mp.roProperty.sn, 6);
		resp[15]=0x00;resp[16]=data[16]+0x01;resp[17]=data[17];
		if (0 == command_type)
		{	/*require cc1101 addr*/
			resp[18] = get_addr(data[11]<<24|data[12]<<16|data[13]<<8|data[14],fangqu_wireless,WIRELESS_MAX);
		} else if (0x0010 == command_type) {
			/*get cur status*/
			resp[18] = cur_status;
		} else if (0x0014 == command_type) {
			/*configrm coding*/
			resp[18] = 0x01;
			//g_addr[addr_cnt] = data[11]<<24|data[12]<<16|data[13]<<8|data[14];			
			//addr_cnt++;
			if (g_main_state ==1)
				save_fq(fangqu_wireless,WIRELESS_MAX);
		} else if (0x0006 == command_type) {
			/*send alarm to server*/
			resp[18] = cur_status;
			if (sub_cmd_type == 2 || cur_status || fangqu_wireless[g_index_sub].operationType==2) {
				g_num = in_fq;
				if (sub_cmd_type == 2)
					s1=1;
				rt_event_send(&(g_info_event), INFO_EVENT_ALARM);
				rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);					
			}
		} else if (0x000c == command_type) {
			/*send low power alarm to server*/
			resp[18] = cur_status;
			rt_event_send(&(g_info_event), INFO_EVENT_ALARM);
			g_num = in_fq;
			rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);
		}
		resp[4]=16;
		unsigned short crc = CRC_check(resp,19);
		resp[19]=(crc>>8) & 0xff;
		resp[20]=(crc) & 0xff;
		cc1101_send_write(resp,21);
	}else {
		if (command_type == 0x0002 && !cur_status)
		{
			cur_status = 1;
			rt_event_send(&(g_info_event), INFO_EVENT_PROTECT_ON);
		}
		else if(command_type == 0x0004 && cur_status)
		{
			cur_status = 0;
			rt_event_send(&(g_info_event), INFO_EVENT_PROTECT_OFF);
		}
		g_num = in_fq;
		rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);
		if (g_main_state ==1)
		save_fq(fangqu_wireless,WIRELESS_MAX);
	}
}
