#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "cc1101.h"
#include "subPoint.h"
#include "master.h"
#include "bsp_misc.h"
#define MSG_HEAD0		0x6c
#define MSG_HEAD1		0xaa
rt_uint8_t cur_status = 0;//protect off 1 for on
rt_uint8_t stm32_id[6] = {0xa1,0x18,0x01,0x02,0x12,0x34};
rt_uint8_t stm32_zero[6] = {0};
rt_uint32_t sub_id = {0}; 
rt_uint16_t command_type = 0;
rt_uint8_t sub_cmd_type = 0;
rt_uint8_t model = 0;
rt_uint8_t dev_type = 0;
rt_uint16_t battery = 0;
rt_uint8_t in_fq = 0;
rt_uint32_t g_addr[60] = {0x00};
rt_uint8_t base_addr = 2;
rt_uint8_t addr_cnt = 0;
rt_uint8_t g_main_state = 0; /*0 normal , 1 code, 2 factory reset*/

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
			return "Wire";
		case 0x02:
			return "Wireless";
		default:
			return "UnKnown";
	}
	return "UnKnown";
}
char get_addr(rt_uint32_t subId, struct FangQu *list)
{
	int i;
	for (i=0;i<140;i++)
	{
		if (subId == list[i].slave_sn)
			return list[i].index;
	}

	if (i==140)
	{
		i=0;
		while(list[i].index != 0)
			i++;
	}
	return i+2;
}
void save_fq(rt_uint32_t subId, struct FangQu *list)
{
	int i;
	for (i=0;i<140;i++)
	{
		if (list[i].index == 0)
		{
			list[i].index = i+2;
			list[i].slave_sn = subId;
			rt_kprintf("save fq to %d , index %d, sn %x\r\n",
				i,list[i].index,list[i].slave_sn);
		}
	}
}
void cmd_dump(rt_uint8_t *data)
{
	/*check subdevice id = local device id*/
	memcpy(stm32_id, data+5, 6);
	sub_id= data[11]<<24|data[12]<<16|data[13]<<8|data[14];
	command_type = data[15]<<8|data[16];
	rt_kprintf("<== \r\nSTM32 ID :\t%x%x%x%x%x%x\r\n", data[5],data[6],data[7],data[8],data[9],data[10]);
	rt_kprintf("Sub ID :\t%x%x%x%x\r\n", data[11],data[12],data[13],data[14]);
	rt_kprintf("CMD Type :\t%s\r\n",cmd_type(command_type));
	if (0x0000 != (data[15]<<8|data[16]) && 0x0014 != (data[15]<<8|data[16]))
	{
		sub_cmd_type = data[17];
		dev_type = data[18];
		rt_kprintf("CMD SubType :\t%s\r\n",cmd_sub_type(data[17]));
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(data[18]));
	}
	else
	{
		dev_type = data[17];
		model = data[18];
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(data[17]));
		rt_kprintf("Dev Model :\t%x\r\n", data[18]);
	}
	battery = data[19]<<8|data[20];
	in_fq = data[1];
	rt_kprintf("Battery :\t%d\r\n",data[19]<<8|data[20]);
	rt_kprintf("Protect Zone :\t%x\r\n", data[1]);
}
int check_sub_id(rt_uint32_t sub_mid,struct FangQu *list)
{
	int i;
	for (i=0; i<140; i++)
	{
		if (sub_mid == list[i].slave_sn)
			return 1;
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
	if (!check_sub_id(sub_id, fqp.fangquList)
		&& g_main_state != 1) {
		rt_kprintf("sub id not in list , and not codeing mode\r\n");
		return ;
	}
	if (data[18] != 0x01) {
		resp[0] = data[1];
		resp[1] = data[0];
		rt_memcpy(resp+2, data+2,13);
		rt_memcpy(resp+5, stm32_id, 6);
		resp[15]=0x00;resp[16]=data[16]+0x01;resp[17]=data[17];
		if (0 == (data[15]<<8|data[16]))
		{	/*require cc1101 addr*/
			resp[18] = get_addr(data[11]<<24|data[12]<<16|data[13]<<8|data[14],fqp.fangquList);
		} else if (0x0010 == (data[15]<<8|data[16])) {
			/*get cur status*/
			resp[18] = cur_status;
		} else if (0x0014 == (data[15]<<8|data[16])) {
			/*configrm coding*/
			resp[18] = 0x01;
			//g_addr[addr_cnt] = data[11]<<24|data[12]<<16|data[13]<<8|data[14];			
			//addr_cnt++;
			save_fq(sub_id,fqp.fangquList);
		} else if (0x0006 == (data[15]<<8|data[16])) {
			/*send alarm to server*/
			resp[18] = cur_status;
		} else if (0x000c == (data[15]<<8|data[16])) {
			/*send low power alarm to server*/
			resp[18] = cur_status;
		}
		resp[4]=16;
		unsigned short crc = CRC_check(resp,19);
		resp[19]=(crc>>8) & 0xff;
		resp[20]=(crc) & 0xff;
		cc1101_send_write(resp,21);
	}else {
		if (command_type == 0x0002)
			cur_status = 1;
		else if(command_type == 0x0004)
			cur_status = 0;
	}
}
