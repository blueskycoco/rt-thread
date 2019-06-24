#ifndef __WTN6_H__
#define __WTN6_H__

#include "stm32f10x.h"

//#define BUSY   PBout(0)			//芯片状态输出
//#define CLK    PBin(1)			//时钟
//#define DATA   PBin(2)			//数据

#define wtn_BUSY   GPIO_Pin_15			//芯片状态输出
#define wtn_CLK    GPIO_Pin_14			//时钟pa15
#define wtn_DATA   GPIO_Pin_13			//数据pb4

#define wtn_BUSY_PORT GPIOD
#define wtn_CLK_PORT GPIOD
#define wtn_DATA_PORT GPIOD

/*命令类型*/
#define CMD_Stop   0xFE			//停止播放
#define CMD_LOOP   0xF2			//循环播放
#define CMD_JOIN   0xF3			//连码播放
#define CMD_MUTE	 0xF8			//插入静音


/*语音地址*/
#define VOICE_COUNTDOWN        	0x00        //10秒倒计时
#define VOICE_BUFANG        		0x01        //布防
#define VOICE_CHEFANG        		0x02        //撤防
#define VOICE_ERRORTIP        	0x03        //错误音
#define VOICE_DUIMA        			0x04        //对码成功
#define VOICE_DUIMAMS        		0x05        //对码模式	
#define VOICE_DUIMASB        		0x06        //对码失败	
#define VOICE_FCALARM        		0x07        //防拆报警
#define VOICE_FANGQUYM        	0x08        //防区已满
#define VOICE_WELCOME        		0x09        //欢迎使用云盾超级报警器	
#define VOICE_HZALARM        		0x0a        //火灾报警
#define VOICE_JIANPAN        		0x0b        //键盘(操作)
#define VOICE_JIAOLIUDD        	0x0c        //交流电断电	
#define VOICE_JIAOLIUHF        	0x0d        //交流电恢复
#define VOICE_JJALARM        		0x0e        //紧急报警
#define VOICE_ALARM2        		0x0f        //你已进入警戒区域，请速撤防	
#define VOICE_ALARM1        		0x10        //你已进入警戒区域，出警人员正在火速赶来	
#define VOICE_CHANGEIP        	0x11        //请把IP模块插入右侧插槽
#define VOICE_XIANCHEFANG       0x12        //请先撤防
#define VOICE_AGAIN        			0x13        //请重试
#define VOICE_SHOUJI        		0x14        //手机(操作)
#define VOICE_TUICHUDM        	0x15        //退出对码模式	
#define VOICE_NOMOKUAI        	0x16        //未检测到通信模块
#define VOICE_YANSHIBF        	0x17        //延时布防模式，音乐	
#define VOICE_YAOKONG        		0x19        //遥控(操作)
#define VOICE_YLALARM        		0x1a        //医疗报警
#define VOICE_ZHONGXIN        	0x1b        //中心(操作)
#define VOICE_ZHUJIGZ        		0x1c        //主机故障	

/*第二版 新增语音地址*/
#define VOICE_UPDATE        		0x18        //准备升级
#define VOICE_ANJIAN        		0x1d        //按键
#define VOICE_DINGSHI        		0x1e        //定时
#define VOICE_HYGLIN        	  0x1f        //欢迎光临
#define VOICE_HFSUCCESS         0x20        //恢复成功
#define VOICE_BFTIXING          0x21        //布防提醒
#define VOICE_WEIXIN            0x22        //微信
#define VOICE_FENQU             0x23        //分区
#define VOICE_FANGQU            0x24        //防区
#define VOICE_RQALARM           0x25        //燃气报警

#define VOICE_JIAOFEI        		0x26        //服务到期，请缴费
#define VOICE_ALARM3        		0x27        //警笛


//typedef enum
//{
//	VOICE_BUFANG=0x00,							
//	VOICE_CHEFANG=0x01,							
//	VOICE_DUIMA=0x02,								
//	VOICE_DUIMAMS=0x03,							
//	VOICE_DUIMASB=0x04,							
//	VOICE_WELCOME=0x05,							
//	VOICE_HUIFU=0x06,								//恢复成功	
//	VOICE_JIAOLIUDD=0x07,						
//	VOICE_ALARM1=0x08,							
//	VOICE_ALARM2=0x09,							
//	VOICE_SHANCHU=0x0A,							//删除成功	
//	VOICE_TUICHUDM=0x0B,						
//	VOICE_YANSHIBF=0x0C,						
//	VOICE_ZHUJIGZ=0x0D							
//}Wtn6_VoiceTypeDef;

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
extern void Wtn6_Play(u8 voice,Wtn6_PlayTypeDef PlayType, u8 flag);
extern void Wtn6_JoinPlay(u8 voices[],u8 size,u8 muteTimes);
extern void Stop_Playing(void);

/*内部接口*/
static void Set_Loop(void);
static u8 Is_Playing(void);
//static void Stop_Playing(void);
static void Send_Command(u8 command);
#endif
