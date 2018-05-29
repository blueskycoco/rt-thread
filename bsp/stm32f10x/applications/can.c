#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stm32f10x.h>
#include <string.h>
#include "prop.h"
#include "can.h"
#define MASTER	1
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
int can_send(unsigned short id, unsigned char *payload, 
		unsigned char payload_len)
{
	int i;
	uint8_t status,TransmitMailbox;
	TxMessage.StdId = id;
	memcpy(TxMessage.Data, payload, 8);
	//CAN_ClearFlag(CAN1,0xffffffff);
#if 1
	rt_kprintf("CAN send ID %x:\r\n", id);
	for (i=0;i<payload_len;i++)
		rt_kprintf("%02x ", payload[i]);
	rt_kprintf("\r\n");
#endif
	TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
	rt_kprintf("can send error %x\r\n",CAN_GetLastErrorCode(CANx));
	if (TransmitMailbox == CAN_TxStatus_NoMailBox) {
		return 0;
	}

	i = 0;
	while(((status = CAN_TransmitStatus(CANx, TransmitMailbox)) != CANTXOK) 
			&& (i != 0xFFFF))
	{
		i++;
	}
	rt_kprintf("i is %x\r\n", i);
	if (i == 0xFFFF)
		return 0;
	
	return 1;
}
int can_read(unsigned char *buf, unsigned char *buf_len)
{
	*buf_len = 0;
	//uint8_t num = CAN_MessagePending(CAN1, CAN_FIFO0);
	//if (num == 0)
	//	return 0;
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
#if 1
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
#if 1
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
	CAN_InitStructure.CAN_Prescaler = 4;
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

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    rt_kprintf("can1 rx0 %d\r\n", CAN_GetITStatus(CAN1,CAN_IT_FMP0));
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
				rt_kprintf("get stdid %d message\r\n", stdid);
                if (resp[0] == 0x00 && resp[1] == 0x00)
                {   //assign can addr
	                id = resp[4];
	                id = (id<<8) + resp[5];
	                id = (id<<8) + resp[6];
	                id = (id<<8) + resp[7];
         			addr = get_addr(id,fangqu_wire,WIRE_MAX);
					fangqu_wire[addr - WIRELESS_MAX].index = addr;
					fangqu_wire[addr - WIRELESS_MAX].slave_sn = id;
					fangqu_wire[addr - WIRELESS_MAX].slave_model = (resp[2]<<8)|resp[3];
                	rt_kprintf("addr %d, id %08x mode %02x\r\n",addr - WIRELESS_MAX,
						fangqu_wire[addr - WIRELESS_MAX].slave_sn,
						fangqu_wire[addr - WIRELESS_MAX].slave_model);
                    cmd[0] = 0x00;cmd[1]=0x01;
                    cmd[2] = (addr >> 8) & 0xff;
                    cmd[3] = addr&0xff;
                    memcpy(cmd+4, resp+4, 4);
                    can_send(2,cmd,8);
                } else if (resp[0] == 0x00 && resp[1] == 0x02) {
                    //info
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
                    fangqu_wire[addr].slave_type = resp[3];
					fangqu_wire[addr].slave_batch = 
						((resp[4]<<24)|(resp[5]<<16)|(resp[6]<<8)|(resp[7]<<0))&0x00ffffff;						
					fangqu_wire[addr].voiceType =TYPE_VOICE_Y;
					fangqu_wire[addr].operationType= TYPE_DELAY;
					fangqu_wire[addr].alarmType= TYPE_ALARM_00;
					fangqu_wire[addr].slave_delay = TYPE_SLAVE_MODE_DELAY;
					fangqu_wire[addr].status= TYPE_PROTECT_OFF;
					fangqu_wire[addr].isStay= TYPE_STAY_N;
					fangqu_wire[addr].isBypass= TYPE_BYPASS_N;
                    rt_kprintf("dev_type %x, fact_time %x\r\n",
                    	resp[3],fangqu_wire[addr].slave_batch);
                    cmd[0] = 0x00;cmd[1]=0x03;
                    cmd[2] = 0;cmd[3] = fangqu_wire[addr].status-1;
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
                    can_send(resp[2],cmd,8);
                }  else if (resp[0] == 0x00 && resp[1] == 0x06) {
                    //alarm
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
                    cmd[0] = 0x00;cmd[1]=0x07;
                    cmd[2] = 0x00;cmd[3] = fangqu_wire[addr].status-1;
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
                    can_send(resp[2],cmd,8);
                } else if (resp[0] == 0x00 && resp[1] == 0x10) {
                    //curr status
                    if (resp[2] < WIRELESS_MAX)
						return;
                    addr = resp[2] - WIRELESS_MAX;
                    cmd[0] = 0x00;cmd[1]=0x11;
                    cmd[2] = 0x00;cmd[3] = fangqu_wire[addr].status-1;
					cmd[4] = (fangqu_wire[addr].slave_sn>>24) & 0xff;
					cmd[5] = (fangqu_wire[addr].slave_sn>>16) & 0xff;
					cmd[6] = (fangqu_wire[addr].slave_sn>>8) & 0xff;
					cmd[7] = (fangqu_wire[addr].slave_sn>>0) & 0xff;
                    can_send(resp[2],cmd,8);
                }
            }
#else
            handle_can_resp();
#endif
       // CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}

