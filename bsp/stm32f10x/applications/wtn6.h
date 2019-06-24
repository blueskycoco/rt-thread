#ifndef __WTN6_H__
#define __WTN6_H__

#include "stm32f10x.h"

//#define BUSY   PBout(0)			//оƬ״̬���
//#define CLK    PBin(1)			//ʱ��
//#define DATA   PBin(2)			//����

#define wtn_BUSY   GPIO_Pin_15			//оƬ״̬���
#define wtn_CLK    GPIO_Pin_14			//ʱ��pa15
#define wtn_DATA   GPIO_Pin_13			//����pb4

#define wtn_BUSY_PORT GPIOD
#define wtn_CLK_PORT GPIOD
#define wtn_DATA_PORT GPIOD

/*��������*/
#define CMD_Stop   0xFE			//ֹͣ����
#define CMD_LOOP   0xF2			//ѭ������
#define CMD_JOIN   0xF3			//���벥��
#define CMD_MUTE	 0xF8			//���뾲��


/*������ַ*/
#define VOICE_COUNTDOWN        	0x00        //10�뵹��ʱ
#define VOICE_BUFANG        		0x01        //����
#define VOICE_CHEFANG        		0x02        //����
#define VOICE_ERRORTIP        	0x03        //������
#define VOICE_DUIMA        			0x04        //����ɹ�
#define VOICE_DUIMAMS        		0x05        //����ģʽ	
#define VOICE_DUIMASB        		0x06        //����ʧ��	
#define VOICE_FCALARM        		0x07        //���𱨾�
#define VOICE_FANGQUYM        	0x08        //��������
#define VOICE_WELCOME        		0x09        //��ӭʹ���ƶܳ���������	
#define VOICE_HZALARM        		0x0a        //���ֱ���
#define VOICE_JIANPAN        		0x0b        //����(����)
#define VOICE_JIAOLIUDD        	0x0c        //������ϵ�	
#define VOICE_JIAOLIUHF        	0x0d        //������ָ�
#define VOICE_JJALARM        		0x0e        //��������
#define VOICE_ALARM2        		0x0f        //���ѽ��뾯���������ٳ���	
#define VOICE_ALARM1        		0x10        //���ѽ��뾯�����򣬳�����Ա���ڻ��ٸ���	
#define VOICE_CHANGEIP        	0x11        //���IPģ������Ҳ���
#define VOICE_XIANCHEFANG       0x12        //���ȳ���
#define VOICE_AGAIN        			0x13        //������
#define VOICE_SHOUJI        		0x14        //�ֻ�(����)
#define VOICE_TUICHUDM        	0x15        //�˳�����ģʽ	
#define VOICE_NOMOKUAI        	0x16        //δ��⵽ͨ��ģ��
#define VOICE_YANSHIBF        	0x17        //��ʱ����ģʽ������	
#define VOICE_YAOKONG        		0x19        //ң��(����)
#define VOICE_YLALARM        		0x1a        //ҽ�Ʊ���
#define VOICE_ZHONGXIN        	0x1b        //����(����)
#define VOICE_ZHUJIGZ        		0x1c        //��������	

/*�ڶ��� ����������ַ*/
#define VOICE_UPDATE        		0x18        //׼������
#define VOICE_ANJIAN        		0x1d        //����
#define VOICE_DINGSHI        		0x1e        //��ʱ
#define VOICE_HYGLIN        	  0x1f        //��ӭ����
#define VOICE_HFSUCCESS         0x20        //�ָ��ɹ�
#define VOICE_BFTIXING          0x21        //��������
#define VOICE_WEIXIN            0x22        //΢��
#define VOICE_FENQU             0x23        //����
#define VOICE_FANGQU            0x24        //����
#define VOICE_RQALARM           0x25        //ȼ������

#define VOICE_JIAOFEI        		0x26        //�����ڣ���ɷ�
#define VOICE_ALARM3        		0x27        //����


//typedef enum
//{
//	VOICE_BUFANG=0x00,							
//	VOICE_CHEFANG=0x01,							
//	VOICE_DUIMA=0x02,								
//	VOICE_DUIMAMS=0x03,							
//	VOICE_DUIMASB=0x04,							
//	VOICE_WELCOME=0x05,							
//	VOICE_HUIFU=0x06,								//�ָ��ɹ�	
//	VOICE_JIAOLIUDD=0x07,						
//	VOICE_ALARM1=0x08,							
//	VOICE_ALARM2=0x09,							
//	VOICE_SHANCHU=0x0A,							//ɾ���ɹ�	
//	VOICE_TUICHUDM=0x0B,						
//	VOICE_YANSHIBF=0x0C,						
//	VOICE_ZHUJIGZ=0x0D							
//}Wtn6_VoiceTypeDef;

/*
*���ŷ�ʽ
* once=0x00
* loop=0x01
*/
typedef enum
{
	ONCE=0x00,						//����һ��
	LOOP=0x01							//ѭ�����ţ�ֱ��������������
	
}Wtn6_PlayTypeDef;

/*������01-16������*/
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

/*�ⲿ�ӿ�*/
extern void Wtn6_Init(void);
extern void Wtn6_Set_Volumne(Wtn6_VolumeDef volumne);
extern void Wtn6_Play(u8 voice,Wtn6_PlayTypeDef PlayType, u8 flag);
extern void Wtn6_JoinPlay(u8 voices[],u8 size,u8 muteTimes);
extern void Stop_Playing(void);

/*�ڲ��ӿ�*/
static void Set_Loop(void);
static u8 Is_Playing(void);
//static void Stop_Playing(void);
static void Send_Command(u8 command);
#endif
