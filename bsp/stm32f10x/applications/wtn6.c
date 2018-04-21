#include "wtn6.h"
//#include "delay.h"

/*
*外部接口，初始化IO接口
#define BUSY   PBout(0)			//芯片状态输出
#define CLK    PBin(1)			//时钟
#define DATA   PBin(2)			//数据
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
*外部接口，设置播放音量
*取值范围 0-16 , 16声音最大
*/
void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne)
{
	Send_Command(volumne);
}
/*
*外部接口，播放一条语音
*voice 语音地址
*playType 播放一次或者循环播放
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
*外部接口，播放多段语音
* voice 语音地址集合
* muteTimes 每段语音之间的间隔时间 10ms 为单位,推荐muteTimes=3
*-------------------------------------------------------------*
* 											常用语音组合方式											*
*-------------------------------------------------------------*
* VOICE_JIANPAN  |  VOICE_BUFANG  | 键盘布防									*
*-------------------------------------------------------------*
* VOICE_SHOUJI   |  VOICE_BUFANG  | 手机布防									*
*-------------------------------------------------------------*
* VOICE_YAOKONG  |  VOICE_BUFANG  | 遥控布防									*
*-------------------------------------------------------------*
* VOICE_ZHONGXIN |  VOICE_BUFANG  | 中心布防									*
*-------------------------------------------------------------*
*-------------------------------------------------------------*
* VOICE_JIANPAN  |  VOICE_CHEFANG | 键盘撤防									*
*-------------------------------------------------------------*
* VOICE_SHOUJI   |  VOICE_CHEFANG | 手机撤防									*
*-------------------------------------------------------------*
* VOICE_YAOKONG  |  VOICE_CHEFANG | 遥控撤防									*
*-------------------------------------------------------------*
* VOICE_ZHONGXIN |  VOICE_CHEFANG | 中心撤防									*
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
*内部接口，将芯片设置循环播放
*/
void Set_Loop(void)
{
	Send_Command(CMD_LOOP);
}
/*
*内部接口，查询芯片是否正在播放中
*/
u8 Is_Playing(void)
{
	return GPIO_ReadInputDataBit(wtn_BUSY_PORT,wtn_BUSY);
}
/*
*内部接口，停止正在播放的语音
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
*内部接口，向芯片发送命令数据
*command:语音地址以及命令代码
*/
static void Send_Command(u8 command)
{
	u8 index;
	GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);
	delay_us(400);
	for(index=0;index<8;index++)
	{
		GPIO_ResetBits(wtn_CLK_PORT,wtn_CLK);//拉低时钟
		if(command&0x01)
		{
			GPIO_SetBits(wtn_DATA_PORT,wtn_DATA);//赋值
		}
		else
		{
			GPIO_ResetBits(wtn_DATA_PORT,wtn_DATA);//赋值
		}
		delay_us(100);			//40us-320us之间，推荐延时300us
		GPIO_SetBits(wtn_CLK_PORT,wtn_CLK); //拉高时钟,上升沿提交数据
		delay_us(100);			//40us-320us之间，推荐延时300us
		command=command>>1;
	}
	GPIO_SetBits(wtn_CLK_PORT,wtn_CLK);
	GPIO_SetBits(wtn_DATA_PORT,wtn_DATA);
}
