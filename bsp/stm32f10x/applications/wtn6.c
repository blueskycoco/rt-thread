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
*外部接口，初始化IO接口
*/
void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne)
{
	Send_Command(volumne);
}
/*
*外部接口，初始化IO接口
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
*内部接口，将芯片设置循环播放
*/
static void Set_Loop(void)
{
	Send_Command(CMD_LOOP);
}
/*
*内部接口，查询芯片是否正在播放中
*/
static u8 Is_Playing(void)
{
	return GPIO_ReadInputDataBit(BUSY_PORT,BUSY);
	//return BUSY;
}
/*
*内部接口，停止正在播放的语音
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
*内部接口，向芯片发送命令数据
*command:语音地址以及命令代码
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
		GPIO_ResetBits(CLK_PORT,CLK);//拉低时钟
		if(command&0x01)
		{
			GPIO_SetBits(DATA_PORT,DATA);//赋值
		}
		else
		{
			GPIO_ResetBits(DATA_PORT,DATA);//赋值
		}
		delay_us(300);			//延时300us
		//rt_thread_delay(1);
		GPIO_SetBits(CLK_PORT,CLK); //拉高时钟,上升沿提交数据
		delay_us(300);			//延时300us
		//rt_thread_delay(1);
		command=command>>1;
	}
	GPIO_ResetBits(CLK_PORT,CLK);
	GPIO_ResetBits(DATA_PORT,DATA);
}
