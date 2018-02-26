#ifndef _LCD__
#define _LCD__
/**************************************************************************************************/
/*|  PIN  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12  |  13  |  14  |*/
/*|       |     |     |     |     |seg0 |seg1 |seg2 |seg3 |seg4 |seg5 |seg6 | seg7 | seg8 | seg9 |*/
/*| com1  |com1 |     |     |     | 1A  | p6  | 2A  | p7  | s8  | s1  | p5  | p13  | p18  | p17  |*/
/*| com2  |     |com2 |     |     | 1F  | 1B  | 2F  | 2B  | s7  | s2  | p4  | p12  | p1   | p16  |*/
/*| com3  |     |     |com3 |     | 1E  | 1G  | 2E  | 2G  | s6  | s3  | p3  | p11  | p8   | p15  |*/
/*| com4  |     |     |     |com4 | 1D  | 1C  | 2D  | 2C  | s5  | s4  | p2  | p10  | p9   | p14  |*/
/**************************************************************************************************/
/* segx x=ż��=��4bit��x=����=����4bit��                                                           */
/* seg0_3 �������ֹܣ�p6=��������ͼ�ꣻp7���޽�����ͼ�ꣻ                                           */
/* seg4 ���ͼ�ꣻ seg5 wifiͼ��                                                                   */
/* seg6 p5=sim��ͼ�꣬p4=����ͼ�꣬p3=����ͼ�꣬p2=����ͼ��                                         */ 
/* seg7_8 �ź�ǿ��ͼ�꣬���У�p1=����ͼ��                                                           */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                              */
/**************************************************************************************************/
//#include "Global.h"

#define MODE_COM                0
#define MODE_DATA               1

#define LCD_PIN_CS              GPIO_Pin_9//GPIO_PIN_7
#define LCD_PIN_RD              GPIO_Pin_8
#define LCD_PIN_WR              GPIO_Pin_7//GPIO_PIN_9
#define LCD_PIN_DATA            GPIO_Pin_2//GPIO_PIN_10

#define COMMAND_SYS_DIS         ((uint8_t)0x00) /*�ر�ϵͳ������LCD ƫѹ������*/
#define COMMAND_SYS_EN          ((uint8_t)0x01) /*��ϵͳ����*/
#define COMMAND_LCD_OFF         ((uint8_t)0x02) /*�ر�LCDƫѹ������*/
#define COMMAND_LCD_ON          ((uint8_t)0x03) /*��LCDƫѹ������*/
#define COMMAND_TIMER_DIS       ((uint8_t)0x04) /*ʱ�����ʧЧ*/
#define COMMAND_WDT_DIS         ((uint8_t)0x05) /*���Ź������־���ʧЧ*/
#define COMMAND_TIMER_EN        ((uint8_t)0x06) /*ʱ�����ʹ��*/
#define COMMAND_WDT_EN          ((uint8_t)0x07) /*���Ź������־�����Ч*/
#define COMMAND_TONE_OFF        ((uint8_t)0x08) /*�ر��������*/
#define COMMAND_TONE_ON         ((uint8_t)0x09) /*���������*/
#define COMMAND_CLR_TIMER       ((uint8_t)0x0C) /*ʱ������������*/
#define COMMAND_CLR_WDT         ((uint8_t)0x0E) /*���WDT״̬*/
#define COMMAND_XTAL_32K        ((uint8_t)0x14) /*ϵͳʱ��Դ_����*/
#define COMMAND_RC_256K         ((uint8_t)0x18) /*ϵͳʱ��Դ_Ƭ��RC����*/
#define COMMAND_EXT_256K        ((uint8_t)0x1C) /*ϵͳʱ��Դ_�ⲿʱ��Դ*/
#define COMMAND_BIAS12_2COM     ((uint8_t)0x20) /*LCD 1/2ƫѹѡ��,2�������*/
#define COMMAND_BIAS12_3COM     ((uint8_t)0x24) /*LCD 1/2ƫѹѡ��,3�������*/
#define COMMAND_BIAS12_4COM     ((uint8_t)0x28) /*LCD 1/2ƫѹѡ��,4�������*/
#define COMMAND_BIAS13_2COM     ((uint8_t)0x21) /*LCD 1/3ƫѹѡ��,2�������*/
#define COMMAND_BIAS13_3COM     ((uint8_t)0x25) /*LCD 1/3ƫѹѡ��,3�������*/
#define COMMAND_BIAS13_4COM     ((uint8_t)0x29) /*LCD 1/3ƫѹѡ��,4�������*/
#define COMMAND_TONE_4K         ((uint8_t)0x40) /*����Ƶ��4KHz*/
#define COMMAND_TONE_2K         ((uint8_t)0x50) /*����Ƶ��2KHz*/
#define COMMAND_IRQ_DIS         ((uint8_t)0x80) /*ʹ/IRQ ���ʧЧ*/
#define COMMAND_IRQ_EN          ((uint8_t)0x88) /*ʹ/IRQ �����Ч*/
#define COMMAND_TOPT            ((uint8_t)0xE0) /*����ģʽ*/
#define COMMAND_TNORMAL         ((uint8_t)0xE3) /*��ͨģʽ*/

#define LCD_GPIOE               GPIOE
#define LCD_GPIOB               GPIOB
#define DELAY_MS                ((uint8_t)1)

typedef enum
{
  LEVEL0,
  LEVEL1,
  LEVEL2,
  LEVEL3,
  LEVEL4,
  LEVEL5
} HTB_LEVEL;

typedef enum
{
  SIM_0G=0x00,
  SIM_2G=0x01,
  SIM_3G=0x02,
  SIM_4G=0x04,
  SIM_5G=0x08
} HTB_SIM;

typedef enum
{
  WIFI,
  BATTERY
} HTB_STYLE;

typedef enum
{
  ICO_BUFANG,                           /*����ͼ��*/
  ICO_CHEFANG,                          /*����ͼ��*/
  ICO_BAOJING,                          /*����ͼ��*/
  ICO_GUZHANG,                          /*����ͼ��*/
  ICO_JIAOLIU,                          /*���޽�����*/
  ICO_NETWORK,                          /*��������*/
  ICO_SIM                               /*����SIM��*/
}HTB_ICO;

typedef enum
{
  ICO_OFF=0,                            /*����ͼ��*/
  ICO_ON=1                              /*��ʾͼ��*/
}HTB_ICO_STATE;
/**************************************************************************************************/
/*|  PIN  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12  |  13  |  14  |*/
/*|       |     |     |     |     |seg0 |seg1 |seg2 |seg3 |seg4 |seg5 |seg6 | seg7 | seg8 | seg9 |*/
/*| com1  |com1 |     |     |     | 1A  | p6  | 2A  | p7  | s8  | s1  | p5  | p13  | p18  | p17  |*/
/*| com2  |     |com2 |     |     | 1F  | 1B  | 2F  | 2B  | s7  | s2  | p4  | p12  | p1   | p16  |*/
/*| com3  |     |     |com3 |     | 1E  | 1G  | 2E  | 2G  | s6  | s3  | p3  | p11  | p8   | p15  |*/
/*| com4  |     |     |     |com4 | 1D  | 1C  | 2D  | 2C  | s5  | s4  | p2  | p10  | p9   | p14  |*/
/**************************************************************************************************/
/* segx x=ż��=��4bit��x=����=����4bit��                                                           */
/* seg0_3 �������ֹܣ�p6=��������ͼ�ꣻp7���޽�����ͼ�ꣻ                                           */
/* seg4 ���ͼ�ꣻ seg5 wifiͼ��                                                                   */
/* seg6 p5=sim��ͼ�꣬p4=����ͼ�꣬p3=����ͼ�꣬p2=����ͼ��                                         */ 
/* seg7_8 �ź�ǿ��ͼ�꣬���У�p1=����ͼ��                                                           */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                              */
/**************************************************************************************************/
typedef struct
{
  u8 num_seg0_1;                                /*����1*/
  u8 num_seg2_3;                                /*����2*/
  u8 ico_seg4_5;                                /*seg4(h_4bit)=��غ�seg5(l_4bit)=wifi*/
  u8 ico_seg7_8;                                /*�ź�ǿ��*/
  u8 ico_seg69;                                 /*����ͼ��,seg6��seg9=2G--5G*/
} HTB_RAM;
//-------------------------------------�ⲿ�ӿ�-----------------------------------------
void SetErrorCode(u8 value);                            /*���ô�����*/
void SetBatteryIco(u8 value);                           /*���õ��ͼ��*/
void SetWifiIco(u8 value);                              /*����wifiͼ��*/
void SetStateIco(u8 value,HTB_ICO_STATE ico_state);     /*״̬ͼ��*/
void SetSignalIco(u8 value);                            /*�ź�ǿ��*/
void SetSimTypeIco(u8 value);                           /*2G-5G*/
void HtbLcdClear();                                     /*�������ͼ��*/
void HtbLcdShow();                                      /*��ʾ����ͼ��*/
void GPIO_Lcd_Init();

#endif

