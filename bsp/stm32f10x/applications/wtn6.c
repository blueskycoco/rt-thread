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
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = BUSY;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(BUSY_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CLK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CLK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DATA_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(CLK_PORT,CLK);
	GPIO_ResetBits(DATA_PORT,DATA);

	Wtn6_Set_Volumne(LEVEL16);
}
/*
*�ⲿ�ӿڣ���ʼ��IO�ӿ�
*/
void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne)
{
	Send_Command(volumne);
}
/*
*�ⲿ�ӿڣ���ʼ��IO�ӿ�
*/
void Wtn6_Play(Wtn6_VoiceTypeDef voice,Wtn6_PlayTypeDef PlayType)
{
	Stop_Playing();
	//delay_ms(5);
	rt_thread_delay(1);
	Send_Command(voice);
	if(PlayType==LOOP)
	{
		//delay_ms(5); 
		rt_thread_delay(1);
		Set_Loop();
	}
}

/*
*�ڲ��ӿڣ���оƬ����ѭ������
*/
static void Set_Loop(void)
{
	Send_Command(CMD_LOOP);
}
/*
*�ڲ��ӿڣ���ѯоƬ�Ƿ����ڲ�����
*/
static u8 Is_Playing(void)
{
	return GPIO_ReadInputDataBit(BUSY_PORT,BUSY);
	//return BUSY;
}
/*
*�ڲ��ӿڣ�ֹͣ���ڲ��ŵ�����
*/
static void Stop_Playing(void)
{
	if(Is_Playing())
	{
		Send_Command(CMD_Stop);
	}
}
void delay_us(int us)
{
	volatile long cnt = 0;
	while (cnt < us * 10000)
		cnt++;
}
/*
*�ڲ��ӿڣ���оƬ������������
*command:������ַ�Լ��������
*/
static void Send_Command(u8 command)
{
	u8 index=0;
	GPIO_SetBits(CLK_PORT,CLK);
	//delay_ms(5);
	rt_thread_delay(1);
	GPIO_ResetBits(CLK_PORT,CLK);
	//delay_ms(5);
	rt_thread_delay(1);
	for(index=0;index<8;index++)
	{
		GPIO_ResetBits(CLK_PORT,CLK);//����ʱ��
		if(command&0x01)
		{
			GPIO_SetBits(DATA_PORT,DATA);//��ֵ
		}
		else
		{
			GPIO_ResetBits(DATA_PORT,DATA);//��ֵ
		}
		delay_us(300);			//��ʱ300us
		//rt_thread_delay(1);
		GPIO_SetBits(CLK_PORT,CLK); //����ʱ��,�������ύ����
		delay_us(300);			//��ʱ300us
		//rt_thread_delay(1);
		command=command>>1;
	}
	GPIO_ResetBits(CLK_PORT,CLK);
	GPIO_ResetBits(DATA_PORT,DATA);
}
