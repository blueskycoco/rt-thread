#ifndef __WTN6_H__
#define __WTN6_H__

#include "stm32f10x.h"

//#define BUSY   PBout(0)			//芯片状态输出
//#define CLK    PBin(1)			//时钟
//#define DATA   PBin(2)			//数据

#define BUSY   GPIO_Pin_3			//芯片状态输出pb3
#define CLK    GPIO_Pin_15			//时钟pa15
#define DATA   GPIO_Pin_4			//数据pb4

#define BUSY_PORT GPIOB
#define CLK_PORT GPIOA
#define DATA_PORT GPIOB

#define CMD_Stop   0xFE			//停止播放
#define CMD_LOOP   0xF2			//停止播放

/*语音地址*/
typedef enum
{
	VOICE_BUFANG=0x00,							//布防成功
	VOICE_CHEFANG=0x01,							//撤防成功
	VOICE_DUIMA=0x02,								//对码成功
	VOICE_DUIMAMS=0x03,							//对码模式	
	VOICE_DUIMASB=0x04,							//对码失败	
	VOICE_WELCOME=0x05,							//欢迎使用云盾超级报警器	
	VOICE_HUIFU=0x06,								//恢复成功	
	VOICE_JIAOLIUDD=0x07,						//交流电断电	
	VOICE_ALARM1=0x08,							//你已进入警戒区域，出警人员正在火速赶来	
	VOICE_ALARM2=0x09,							//你已进入警戒区域，请速撤防	
	VOICE_SHANCHU=0x0A,							//删除成功	
	VOICE_TUICHUDM=0x0B,						//退出对码模式	
	VOICE_YANSHIBF=0x0C,						//延时布防模式，音乐	
	VOICE_ZHUJIGZ=0x0D							//主机故障	
}Wtn6_VoiceTypeDef;

/*
*播放方式
* once=0x00
* loop=0x01
*/
typedef enum
{
	ONCE=0x00,						//播放一次
	LOOP=0x01							//循环播放，直到有新语音播放
	
}Wtn6_PlayTypeDef;

/*音量，01-16逐渐增大*/
typedef enum
{
	LEVEL01=0xE0,
	LEVEL02=0xE1,
	LEVEL03=0xE2,
	LEVEL04=0xE3,
	LEVEL05=0xE4,
	LEVEL06=0xE5,
	LEVEL07=0xE6,
	LEVEL08=0xE7,
	LEVEL09=0xE8,
	LEVEL10=0xE9,
	LEVEL11=0xEA,
	LEVEL12=0xEB,
	LEVEL13=0xEC,
	LEVEL14=0xED,
	LEVEL15=0xEE,
	LEVEL16=0xEF
	
}Wtn6_VolumeDef;

/*外部接口*/
extern void Wtn6_Init(void);
extern void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne);
extern void Wtn6_Play(Wtn6_VoiceTypeDef voice,Wtn6_PlayTypeDef PlayType);

/*内部接口*/
static void Set_Loop(void);
u8 Is_Playing(void);
void Stop_Playing(void);
static void Send_Command(u8 command);
#endif
