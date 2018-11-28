#include "wtn6.h"
#include <rtthread.h>
//#include "delay.h"
extern void speaker_ctl(int flag);
uint8_t state_play = 0;
uint8_t wtn6_mute = 0;
/*
*�ⲿ�ӿڣ���ʼ��IO�ӿ�
#define BUSY   PBout(0)			//оƬ״̬���
#define CLK    PBin(1)			//ʱ��
#define DATA   PBin(2)			//����
*/
void Wtn6_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	GPIO_InitStructure.GPIO_Pin = wtn_BUSY;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(wtn_BUSY_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = wtn_CLK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(wtn_CLK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = wtn_DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(wtn_DATA_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);
	GPIO_ResetBits(wtn_DATA_PORT,wtn_DATA);

	Wtn6_Set_Volumne(LEVEL08);
}
/*
*�ⲿ�ӿڣ����ò�������
*ȡֵ��Χ 0-16 , 16�������
*/
void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne)
{
	rt_kprintf("Wtn6_Set_Volumne %d\r\n", volumne);
	if (volumne == 0xe0)
		wtn6_mute = 1;
	else
		wtn6_mute = 0;
	Send_Command(volumne);
}
void Stop_Played(void)
{
  while(Is_Playing())
  	rt_thread_delay(10);
  speaker_ctl(0);
}
/*
*�ⲿ�ӿڣ�����һ������
*voice ������ַ
*playType ����һ�λ���ѭ������
*/
void Wtn6_Play(u8 voice,Wtn6_PlayTypeDef PlayType, u8 flag)
{
	if (wtn6_mute)
		return ;
	speaker_ctl(1);
	Send_Command(voice);
	if(PlayType==LOOP)
	{
		Set_Loop();
	} else {
		if (flag)
			Stop_Played();
	}
	state_play=1;
}

/*
*�ⲿ�ӿڣ����Ŷ������
* voice ������ַ����
* muteTimes ÿ������֮��ļ��ʱ�� 10ms Ϊ��λ,�Ƽ�muteTimes=3
*-------------------------------------------------------------*
* 											����������Ϸ�ʽ											*
*-------------------------------------------------------------*
* VOICE_JIANPAN  |  VOICE_BUFANG  | ���̲���									*
*-------------------------------------------------------------*
* VOICE_SHOUJI   |  VOICE_BUFANG  | �ֻ�����									*
*-------------------------------------------------------------*
* VOICE_YAOKONG  |  VOICE_BUFANG  | ң�ز���									*
*-------------------------------------------------------------*
* VOICE_ZHONGXIN |  VOICE_BUFANG  | ���Ĳ���									*
*-------------------------------------------------------------*
*-------------------------------------------------------------*
* VOICE_JIANPAN  |  VOICE_CHEFANG | ���̳���									*
*-------------------------------------------------------------*
* VOICE_SHOUJI   |  VOICE_CHEFANG | �ֻ�����									*
*-------------------------------------------------------------*
* VOICE_YAOKONG  |  VOICE_CHEFANG | ң�س���									*
*-------------------------------------------------------------*
* VOICE_ZHONGXIN |  VOICE_CHEFANG | ���ĳ���									*
*-------------------------------------------------------------*
*/
void Wtn6_JoinPlay(u8 voices[],u8 size,u8 muteTimes)
{
	int i;
	if (wtn6_mute)
		return ;
	if(size<=0) return;
	speaker_ctl(1);
	for(i=0;i<size;i++)
	{
		Send_Command(CMD_JOIN);
		Send_Command(voices[i]);
		if(muteTimes>0)
		{
			Send_Command(CMD_MUTE);
			Send_Command(muteTimes);
		}
	}
	rt_thread_delay(200);
	Stop_Played();
	state_play=1;
}

/*
*�ڲ��ӿڣ���оƬ����ѭ������
*/
void Set_Loop(void)
{
	Send_Command(CMD_LOOP);
}
/*
*�ڲ��ӿڣ���ѯоƬ�Ƿ����ڲ�����
*/
u8 Is_Playing(void)
{
	//rt_kprintf("is playing %d\r\n", GPIO_ReadInputDataBit(wtn_BUSY_PORT,wtn_BUSY));
	return !GPIO_ReadInputDataBit(wtn_BUSY_PORT,wtn_BUSY);
}
/*
*�ڲ��ӿڣ�ֹͣ���ڲ��ŵ�����
*/
void Stop_Playing(void)
{
	if(state_play==1)
	{
		rt_kprintf("stop playing\r\n");
		Send_Command(CMD_Stop);
		speaker_ctl(0);
	}
	state_play=0;
}
static void delay_us(unsigned long ms)
{
    unsigned long len;
    for (;ms > 0; ms --)
        for (len = 0; len < 8; len++ );
}

/*
*�ڲ��ӿڣ���оƬ������������
*command:������ַ�Լ��������
*/
static void Send_Command(u8 command)
{
	u8 index;
	GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);
	delay_us(400);
	for(index=0;index<8;index++)
	{
		GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);//����ʱ��
		if(command&0x01)
		{
			GPIO_SetBits(wtn_DATA_PORT,wtn_DATA);//��ֵ
		}
		else
		{
			GPIO_ResetBits(wtn_DATA_PORT,wtn_DATA);//��ֵ
		}
		delay_us(100);			//40us-320us֮�䣬�Ƽ���ʱ300us
		GPIO_SetBits(wtn_CLK_PORT,wtn_CLK); //����ʱ��,�������ύ����
		delay_us(100);			//40us-320us֮�䣬�Ƽ���ʱ300us
		command=command>>1;
	}
	GPIO_SetBits(wtn_CLK_PORT,wtn_CLK);
	GPIO_SetBits(wtn_DATA_PORT,wtn_DATA);
}
