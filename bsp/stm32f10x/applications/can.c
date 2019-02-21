#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stm32f10x.h>
#include <string.h>
#include "prop.h"
#include "can.h"
#include "wtn6.h"
#include "master.h"
#include "subpoint.h"
#define MASTER	1
rt_uint8_t wire_code  = 0;
extern struct rt_event g_info_event;
extern rt_uint8_t 	g_main_state;
extern rt_uint8_t g_num;
CanRxMsg 							RxMessage;
CAN_FilterInitTypeDef  				CAN_FilterInitStructure;
CanTxMsg 							TxMessage;
#ifdef MASTER
uint16_t 							local_addr = 0x01;
#else
uint16_t 							local_addr = 0x02;
#endif
#define RCC_APB2Periph_GPIO_CAN1   	RCC_APB2Periph_GPIOA
#define CANx			   			CAN1
#define GPIO_CAN		   			GPIOA
#define GPIO_Remapping_CAN	   		GPIO_Remap1_CAN1
#define GPIO_Pin_CAN_RX 	   		GPIO_Pin_11
#define GPIO_Pin_CAN_TX 	   		GPIO_Pin_12
extern rt_uint8_t g_alarmType;
extern rt_uint8_t 	cur_status;
extern rt_uint8_t 	s1;
extern rt_uint8_t g_fq_index;
extern rt_uint8_t  	g_operationType;
extern rt_uint8_t  	g_voiceType;
extern rt_uint16_t command_type;
extern rt_uint8_t 	g_mute;
extern rt_uint8_t g_ac;
extern rt_uint8_t g_wire_type;
extern rt_uint32_t g_wire_addr;
extern rt_uint8_t g_all_fq_num;
extern rt_uint8_t g_all_fq_num_bak;
#if 0
int poll_can()
{
	int i=0;
	while(CAN_MessagePending(CANx, CAN_FIFO0) == 0) {
		i++;
		delay_ms(1);
		if (i==1000)
			break;
	}
	if (i==1000)
		rt_kprintf("no can data\r\n");
	else
		rt_kprintf("have can data %d\r\n", i);
	return (i==1000) ? 0:1;
}
#endif
int can_send(unsigned short id, unsigned char *payload, 
		unsigned char payload_len)
{
	int i;
	uint8_t status,TransmitMailbox;
	TxMessage.StdId = id;
	memcpy(TxMessage.Data, payload, 8);
	//CAN_ClearFlag(CAN1,0xffffffff);
#if 0
	rt_kprintf("CAN send ID %x:\r\n", id);
	for (i=0;i<payload_len;i++)
		rt_kprintf("%02x ", payload[i]);
	rt_kprintf("\r\n");
#endif
	TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
	if (TransmitMailbox == CAN_TxStatus_NoMailBox) {
		rt_kprintf("can send error %x\r\n",CAN_GetLastErrorCode(CANx));
		return 0;
	}

	i = 0;
	while(((status = CAN_TransmitStatus(CANx, TransmitMailbox)) != CANTXOK) 
			&& (i != 0xFFFF))
	{
		i++;
	}
	//rt_kprintf("i is %x\r\n", i);
	if (i == 0xFFFF)
	{
		rt_kprintf("can send2 error %x\r\n",CAN_GetLastErrorCode(CANx));
		return 0;
	}
	
	return 1;
}
int can_read(unsigned char *buf, unsigned char *buf_len)
{
	*buf_len = 0;
	//uint8_t num = CAN_MessagePending(CAN1, CAN_FIFO0);
	//if (num == 0)
	//	return 0;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
#if 0
	//rt_krt_kprintf("pending message %d\r\n", num);
	rt_kprintf("DLC %x, ExtId %x, FMI %x, IDE %x, RTR %x, StdId %x\r\n",
			RxMessage.DLC,(unsigned int)RxMessage.ExtId,RxMessage.FMI,
			RxMessage.IDE,RxMessage.RTR,(unsigned int)RxMessage.StdId);
#endif
	if ((RxMessage.StdId == local_addr)&&(RxMessage.IDE == CAN_ID_STD) 
			&& (RxMessage.DLC == 8))
	{
		memcpy(buf, RxMessage.Data, 8);
		*buf_len = 8;
#if 0
		rt_kprintf("we got message %02x %02x %02x %02x %02x %02x %02x %02x\r\n", 
				RxMessage.Data[0],RxMessage.Data[1],RxMessage.Data[2],
				RxMessage.Data[3],RxMessage.Data[4],RxMessage.Data[5],
				RxMessage.Data[6],RxMessage.Data[7]);
#endif
		return RxMessage.StdId;
	}
	return 0;
}
void set_id(unsigned short id)
{
	local_addr = id & 0x3ff;
	CAN_FilterInitStructure.CAN_FilterIdHigh = local_addr << 5;
	CAN_FilterInit(&CAN_FilterInitStructure);
}
void can_init()
{
	uint8_t i = 0;
	GPIO_InitTypeDef	GPIO_InitStructure;
	CAN_InitTypeDef		CAN_InitStructure;

	NVIC_InitTypeDef  	NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIO_CAN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_CAN, &GPIO_InitStructure);

//	GPIO_PinRemapConfig(GPIO_Remapping_CAN , ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);


	CAN_DeInit(CANx);
	CAN_StructInit(&CAN_InitStructure);

	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = 64; //72.5kbps 4 for 1Mbps
	CAN_Init(CANx, &CAN_InitStructure);

	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
#ifdef MASTER
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0020;
#else
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0040;
#endif
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xffff;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xffff;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	TxMessage.StdId = 0x001;
	TxMessage.ExtId = 0x00;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = CAN_ID_STD;
	TxMessage.DLC = 8;


	RxMessage.StdId = 0x00;
	RxMessage.ExtId = 0x00;
	RxMessage.IDE = CAN_ID_STD;
	RxMessage.DLC = 0;
	RxMessage.FMI = 0;
	for (i = 0;i < 8;i++)
	{
		RxMessage.Data[i] = 0x00;
	}
	uint8_t num = CAN_MessagePending(CANx,CAN_FIFO0); 
	if (num > 0)
	while (num--) {
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	}
	CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
}
char *can_cmd_type(rt_uint16_t cmdtype)
{
	switch (cmdtype) {
		case 0x0000:
			return "require can addr";
		case 0x0002:
			return "upload wire information";
		case 0x0006:
			return "alarm";
		case 0x0010:
			return "ask cur status";
		default:
			return "unknown";
	}
	return "unknown";
}
void handle_wire_alarm(rt_uint8_t addr)
{
	g_num = addr+WIRELESS_MAX;
	g_alarmType = fangqu_wire[addr].alarmType;
	rt_kprintf("proc wire alarm %d %d %d %d\r\n",addr,fangqu_wire[addr].operationType,
		cur_status,fangqu_wire[addr].isBypass);
	g_mute=0;
	s1=0;
	if (fangqu_wire[addr].operationType==2 /*24 hour*/
		) {
		g_fq_index = fangqu_wire[addr].index;
		g_operationType = fangqu_wire[addr].operationType;
		g_voiceType = fangqu_wire[addr].voiceType;
		rt_event_send(&(g_info_event), INFO_EVENT_ALARM);
		rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);				
		rt_kprintf("wire emergency alarm %d\r\n",g_operationType);
	} else {
		/*normal alarm*/
		if (cur_status && !fangqu_wire[addr].isBypass) {					
			g_fq_index = fangqu_wire[addr].index;
			g_operationType = fangqu_wire[addr].operationType;
			g_voiceType = fangqu_wire[addr].voiceType;
			rt_event_send(&(g_info_event), INFO_EVENT_ALARM);
			rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);										
			rt_kprintf("wire normal alarm %d\r\n",g_operationType);
		}
	}
}

void save_fq_wire(int addr, rt_uint8_t type, rt_uint32_t fact_time)
{
	if (fangqu_wire[addr].slave_batch == 0) {
	    fangqu_wire[addr].slave_type = type;
		fangqu_wire[addr].slave_batch = fact_time;
		fangqu_wire[addr].voiceType =TYPE_VOICE_Y;
		fangqu_wire[addr].operationType= TYPE_DELAY;
		fangqu_wire[addr].alarmType= TYPE_ALARM_00;
		fangqu_wire[addr].slave_delay = TYPE_SLAVE_MODE_DELAY;
		fangqu_wire[addr].status= TYPE_PROTECT_OFF;
		fangqu_wire[addr].isStay= TYPE_STAY_N;
		fangqu_wire[addr].isBypass= TYPE_BYPASS_N;		
		fangqu_wire[addr].normal_info = 1;
		g_num=fangqu_wire[addr].index;
		rt_kprintf("save fq to wire %d , index %d, sn %08x\r\n",
			addr, fangqu_wire[addr].index,fangqu_wire[addr].slave_sn);			
			wire_code = 1;
		rt_kprintf("duima ok\r\n");
		g_wire_type = type;
		g_wire_addr = addr;
		g_all_fq_num++;
		rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
		rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);
	}
	add_fqp_t(fangqu_wire[g_wire_addr].index,g_wire_type);
}
void set_sub_wire_led(rt_uint8_t addr, rt_uint8_t time)
{
	rt_uint8_t cmd[8];
	cmd[0] = 0x00;
	cmd[1] = 0x12;
	cmd[2] = 0x01;
	cmd[3] = time;
	cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
	cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
	cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
	cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
	rt_kprintf("going to light %d 10 secs\r\n", addr);
	if (!can_send(addr+WIRELESS_MAX,cmd,8))
		can_init();
}
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    //rt_kprintf("can1 rx0 %d\r\n", CAN_GetITStatus(CAN1,CAN_IT_FMP0));
    if (CAN_GetITStatus(CAN1,CAN_IT_FMP0) != RESET) {
#ifdef MASTER
       // key |= KEY_CAN;
    unsigned char resp[8] = {0};
    unsigned char cmd[8] = {0};
    unsigned char len = 32;
    unsigned short stdid = 0;
    uint16_t addr;
    uint32_t id;
            if ((stdid = can_read(resp, &len)) != 0) {	
				rt_kprintf("<$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");	
                if (resp[0] == 0x00 && resp[1] == 0x00)
                {   //assign can addr
	                id = resp[4];
	                id = (id<<8) + resp[5];
	                id = (id<<8) + resp[6];
	                id = (id<<8) + resp[7];
         			addr = get_addr(id,fangqu_wire,WIRE_MAX);
					if (addr < 80) {
						fangqu_wire[addr - WIRELESS_MAX].index = addr;
						fangqu_wire[addr - WIRELESS_MAX].slave_sn = id;
						fangqu_wire[addr - WIRELESS_MAX].slave_model = (resp[2]<<8)|resp[3];
	                	rt_kprintf("id\t\t%08x\r\n",fangqu_wire[addr - WIRELESS_MAX].slave_sn);
						rt_kprintf("model\t\t%04x\r\n",fangqu_wire[addr - WIRELESS_MAX].slave_model);
						rt_kprintf("assign addr\t%0x\r\n",fangqu_wire[addr - WIRELESS_MAX].index);
	                    cmd[0] = 0x00;cmd[1]=0x01;
	                    cmd[2] = (addr >> 8) & 0xff;
	                    cmd[3] = addr&0xff;
	                    memcpy(cmd+4, resp+4, 4);
	                    if (!can_send(2,cmd,8))
							can_init();
					} else
						rt_kprintf("exceed MAX wire \r\n");
                } else if (resp[0] == 0x00 && resp[1] == 0x02) {
                    //info
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
					//if (g_main_state ==1)
						save_fq_wire(addr, resp[3], (resp[4]<<24)|(resp[5]<<16)|(resp[6]<<8)|resp[7]);
                    rt_kprintf("id\t\t%08x\r\naddr\t\t%x\r\nfact_time\t%x\r\ntype\t\t%x\r\n",
                    	fangqu_wire[addr].slave_sn,resp[2],fangqu_wire[addr].slave_batch,resp[3]);
                    cmd[0] = 0x00;cmd[1]=0x03;
                    cmd[2] = 0;
					if (g_ac)
						cmd[3] = 1;
					else
						cmd[3] = fangqu_wire[addr].status-1;
                    
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
                    if (!can_send(resp[2],cmd,8))
						can_init();
                }  else if (resp[0] == 0x00 && resp[1] == 0x06) {
                    //alarm
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
                    cmd[0] = 0x00;cmd[1]=0x07;
                    cmd[2] = resp[3];
					if (g_ac)
						cmd[3] = 1;
					else
						cmd[3] = fangqu_wire[addr].status-1;
                    
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
					command_type = (resp[0]<<8|resp[1]);
				rt_kprintf("CMD\t\t%s\r\n", can_cmd_type(resp[0]<<8|resp[1]));		
					rt_kprintf("id\t\t%08x\r\n", fangqu_wire[addr].slave_sn);
					rt_kprintf("addr\t\t%x\r\n", resp[2]);
					rt_kprintf("status\t\t%s\r\n", (cmd[3]==0)?"che fang":"bu fang");
					rt_kprintf("alarm\t\t%s\r\n", cmd_sub_type(resp[3]));
                    if (!can_send(resp[2],cmd,8))
						can_init();
					handle_wire_alarm(addr);
				record_fqp_ts(fangqu_wire[addr].index);
                } else if (resp[0] == 0x00 && resp[1] == 0x10) {
                    //curr status
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
                    cmd[0] = 0x00;cmd[1]=0x11;
                    cmd[2] = 0x00;
					if (g_ac)
						cmd[3] = 1;
					else
						cmd[3] = fangqu_wire[addr].status-1;
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
					rt_kprintf("id\t\t%08x\r\n", fangqu_wire[addr].slave_sn);
					rt_kprintf("addr\t\t%x\r\n", resp[2]);
					rt_kprintf("status\t\t%s\r\n", (cmd[3]==0)?"che fang":"bu fang");
					record_fqp_ts(fangqu_wire[addr].index);
                    if (!can_send(resp[2],cmd,8))
						can_init();
                }
				rt_kprintf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$>\r\n");
            }
#else
            handle_can_resp();
#endif
       //CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}

