#include "wtn6.h"
#include <rtthread.h>
//#include "delay.h"
#include "prop.h"
extern rt_uint8_t g_low_power;
extern void speaker_ctl(int flag);
//extern struct HwVersion	        hwv;
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

static uint8_t Wtn6_Check(u8 voice)
{
  rt_uint8_t result;
    switch(hwv.isdVersion)
    {
    case 0:
    case 1:
        if(voice > 0x1c || voice == 0x18)
        {
            result= 0xFF;
        }
        else
        {
            result= voice;
        }
        break;
    case 2:
        if(voice > 25)
        {
            result= 0xFF;
        }
        else
        {
            result= voice;
        }
        break;
    case 3:
        if(voice > 0x27 || voice == 0x06 || voice == 0x13 || voice == 0x1a || voice == 0x1c || voice == 0x24)
        {
            result= 0xFF;
        }
        else
        {
            result= voice;
        }
        break;
    }
    return result;
}
/*
*�ⲿ�ӿڣ�����һ������
*voice ������ַ
*playType ����һ�λ���ѭ������
*/
void Wtn6_Play(u8 voice,Wtn6_PlayTypeDef PlayType, u8 flag)
{
	rt_kprintf("Wtn6_Play %d %d %d %d %d\r\n",
			 wtn6_mute, hwv.isdVersion, voice, PlayType, flag);
	//if (g_low_power || wtn6_mute||(hwv.isdVersion<2&&(voice>0x1c||voice==0x18)))
	//	return ;
    if (g_low_power || wtn6_mute || Wtn6_Check(voice) == 0xFF)
        return ;
	rt_kprintf("going play\r\n");
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
void Wtn6_JoinPlay1(u8 voices[],u8 size,u8 muteTimes)
{
	int i;
	if (wtn6_mute||size<=0)
		return ;
  
  for(i=0;i<size;i++)
	{
    if(hwv.isdVersion<2&&(voices[i]>0x1c||voices[i]==0x18))
    {
      return;
    }
  }
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

void Wtn6_JoinPlay(u8 voices[], u8 size, u8 muteTimes)
{
    int i;
    if (wtn6_mute || size <= 0 || g_low_power)
        return ;
    u8 tempvoices[size];
    u8 len = 0;

    for(i = 0; i < size; i++)
    {
       /* if(hwv.isdVersion < 2)
        {
            if(voices[i] < 0x1d && voices[i] != 0x18)
            {
                tempvoices[len] = voices[i];
                len++;
            }
        }
        else
        {
            tempvoices[i] = voices[i];
            len++;
        }*/
        if(Wtn6_Check(voices[i]) != 0xFF)
        {
            tempvoices[len] = voices[i];
            len++;
        }
    }
    if(len == 0) return;
    if(len > 1)
    {
        speaker_ctl(1);
        for(i = 0; i < len; i++)
        {
            Send_Command(CMD_JOIN);
            Send_Command(tempvoices[i]);
            if(muteTimes > 0)
            {
                Send_Command(CMD_MUTE);
                Send_Command(muteTimes);
            }
        }
        rt_thread_delay(200);
        Stop_Played();
        state_play = 1;
    }
    else
    {
        Wtn6_Play(tempvoices[0], ONCE, 1);
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
#if 0
	GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);
	delay_us(400);
#else
	GPIO_SetBits(wtn_CLK_PORT, wtn_CLK);
	delay_us(400);
	GPIO_ResetBits(wtn_CLK_PORT, wtn_CLK);
    for(rt_uint8_t i = 0; i < 5; i++)
    {
        delay_us(1000);
    }
#endif
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
