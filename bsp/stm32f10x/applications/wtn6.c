#include "wtn6.h"
//#include "delay.h"

/*
*�ⲿ�ӿڣ���ʼ��IO�ӿ�
#define BUSY   PBout(0)			//оƬ״̬���
#define CLK    PBin(1)			//ʱ��
#define DATA   PBin(2)			//����
*/
void Wtn6_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
//  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

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

	Wtn6_Set_Volumne(LEVEL16);
}
/*
*�ⲿ�ӿڣ����ò�������
*ȡֵ��Χ 0-16 , 16�������
*/
void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne)
{
	Send_Command(volumne);
}
/*
*�ⲿ�ӿڣ�����һ������
*voice ������ַ
*playType ����һ�λ���ѭ������
*/
void Wtn6_Play(u8 voice,Wtn6_PlayTypeDef PlayType)
{
	Send_Command(voice);
	if(PlayType==LOOP)
	{
		Set_Loop();
	}
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
	if(size<=0) return;
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
	return GPIO_ReadInputDataBit(wtn_BUSY_PORT,wtn_BUSY);
}
/*
*�ڲ��ӿڣ�ֹͣ���ڲ��ŵ�����
*/
void Stop_Playing(void)
{
	//if(Is_Playing())
	{
		Send_Command(CMD_Stop);
	}
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
