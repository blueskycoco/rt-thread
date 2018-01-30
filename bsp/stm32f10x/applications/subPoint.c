#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "subPoint.h"
#define MSG_HEAD0		0x6c
#define MSG_HEAD1		0xaa
rt_uint8_t cur_status = 0;//protect off 1 for on
rt_uint8_t stm32_id[] = {0xa1,0x18,0x01,0x02,0x12,0x34};
rt_uint32_t g_addr[60] = {0x00};
rt_uint8_t base_addr = 2;
rt_uint8_t addr_cnt = 0;
unsigned short CRC1(unsigned char *Data,unsigned char Data_length)
{
	unsigned int mid=0;
	unsigned char times=0,Data_index=0;
	unsigned short CRC_data=0xFFFF;
	while(Data_length)
	{
		CRC_data=Data[Data_index]^CRC_data;
		for(times=0;times<8;times++)
		{
			mid=CRC_data;
			CRC_data=CRC_data>>1;
			if(mid & 0x0001)
			{
				CRC_data=CRC_data^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC_data;
}
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
			cur_status = 1;
			return "Remoter Protect ON";
		case 0x0004:
			cur_status = 0;
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
			return "Remoter";
		case 0x02:
			return "Infrar";
		default:
			return "UnKnown";
	}
	return "UnKnown";
}
char get_addr(rt_uint32_t subId)
{
	int i=0;
	for (i=0; i<addr_cnt; i++)
	{
		if (subId == g_addr[i])
			break;
	}

	if (i == addr_cnt)
		return base_addr+addr_cnt+1;
	return base_addr+i;
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
	unsigned short crc = CRC1(data, len+3);
	/*check crc*/
	if (crc != (data[len+3] << 8 | data[len+4]))
	{
		rt_kprintf("crc failed %x %x\r\n", crc, (data[len+3] << 8 | data[len+4]));
		return ;
	}
	/*check subdevice id = local device id*/
	rt_kprintf("<== \r\nSTM32 ID :\t%x%x%x%x%x%x\r\n", data[5],data[6],data[7],data[8],data[9],data[10]);
	rt_kprintf("Sub ID :\t%x%x%x%x\r\n", data[11],data[12],data[13],data[14]);
	rt_kprintf("CMD Type :\t%s\r\n",cmd_type(data[15]<<8|data[16]));
	if (0x0000 != (data[15]<<8|data[16]) && 0x0014 != (data[15]<<8|data[16]))
	{
		rt_kprintf("CMD SubType :\t%s\r\n",cmd_sub_type(data[17]));
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(data[18]));
	}
	else
	{
		rt_kprintf("Dev Type :\t%s\r\n",cmd_dev_type(data[17]));
		rt_kprintf("Dev Model :\t%x\r\n", data[18]);
	}
	rt_kprintf("Battery :\t%d\r\n",data[19]<<8|data[20]);
	rt_kprintf("Protect Zone :\t%x\r\n", data[1]);
	
	if (data[18] != 0x01) {
		resp[0] = data[1];
		resp[1] = data[0];
		rt_memcpy(resp+2, data+2,13);
		rt_memcpy(resp+5, stm32_id, 6);
		resp[15]=0x00;resp[16]=data[16]+0x01;resp[17]=data[17];
		if (0 == (data[15]<<8|data[16]))
		{
			resp[18] = get_addr(data[11]<<24|data[12]<<16|data[13]<<8|data[14]);
		} else if (0x0010 == (data[15]<<8|data[16])) {
			resp[18] = cur_status;
		} else if (0x0014 == (data[15]<<8|data[16])) {
			resp[18] = 0x01;
			g_addr[addr_cnt] = data[11]<<24|data[12]<<16|data[13]<<8|data[14];			
			addr_cnt++;
		} else if (0x0006 == (data[15]<<8|data[16])) {
			resp[18] = data[18];
		} else if (0x000c == (data[15]<<8|data[16])) {
			resp[18] = data[18];
		}
		resp[4]=16;
		unsigned short crc = CRC1(resp,19);
		resp[19]=(crc>>8) & 0xff;
		resp[20]=(crc) & 0xff;
		cc1101_send_write(resp,21);
	}
}
