#ifndef __WTN6_H__
#define __WTN6_H__

#include "stm32f10x.h"

//#define BUSY   PBout(0)			//оƬ״̬���
//#define CLK    PBin(1)			//ʱ��
//#define DATA   PBin(2)			//����

#define BUSY   GPIO_Pin_3			//оƬ״̬���pb3
#define CLK    GPIO_Pin_15			//ʱ��pa15
#define DATA   GPIO_Pin_4			//����pb4

#define BUSY_PORT GPIOB
#define CLK_PORT GPIOA
#define DATA_PORT GPIOB

#define CMD_Stop   0xFE			//ֹͣ����
#define CMD_LOOP   0xF2			//ֹͣ����

/*������ַ*/
typedef enum
{
	VOICE_BUFANG=0x00,							//�����ɹ�
	VOICE_CHEFANG=0x01,							//�����ɹ�
	VOICE_DUIMA=0x02,								//����ɹ�
	VOICE_DUIMAMS=0x03,							//����ģʽ	
	VOICE_DUIMASB=0x04,							//����ʧ��	
	VOICE_WELCOME=0x05,							//��ӭʹ���ƶܳ���������	
	VOICE_HUIFU=0x06,								//�ָ��ɹ�	
	VOICE_JIAOLIUDD=0x07,						//������ϵ�	
	VOICE_ALARM1=0x08,							//���ѽ��뾯�����򣬳�����Ա���ڻ��ٸ���	
	VOICE_ALARM2=0x09,							//���ѽ��뾯���������ٳ���	
	VOICE_SHANCHU=0x0A,							//ɾ���ɹ�	
	VOICE_TUICHUDM=0x0B,						//�˳�����ģʽ	
	VOICE_YANSHIBF=0x0C,						//��ʱ����ģʽ������	
	VOICE_ZHUJIGZ=0x0D							//��������	
}Wtn6_VoiceTypeDef;

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
extern void Wtn6_Play(Wtn6_VoiceTypeDef voice,Wtn6_PlayTypeDef PlayType);

/*�ڲ��ӿ�*/
static void Set_Loop(void);
u8 Is_Playing(void);
void Stop_Playing(void);
static void Send_Command(u8 command);
#endif
