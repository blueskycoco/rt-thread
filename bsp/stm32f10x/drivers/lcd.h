#ifndef __GPIO_LCD__
#define __GPIO_LCD__
#include "stm32f10x.h"
/**************************************************************************************************/
/*|  PIN  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12  |  13  |  14  |*/
/*|       |     |     |     |     |seg0 |seg1 |seg2 |seg3 |seg4 |seg5 |seg6 | seg7 | seg8 | seg9 |*/
/*| com1  |com1 |     |     |     | 1A  |  /  | 2A  | p9  | p21 | p8  | p4  | p20  | p1   | p15  |*/
/*| com2  |     |com2 |     |     | 1F  | 1B  | 2F  | 2B  | p24 | p7  | p25 | p19  | p10  | p14  |*/
/*| com3  |     |     |com3 |     | 1E  | 1G  | 2E  | 2G  | p23 | p6  | p3  | p18  | p11  | p13  |*/
/*| com4  |     |     |     |com4 | 1D  | 1C  | 2D  | 2C  | p22 | p5  | p2  | p17  | p16  | p12  |*/
/**************************************************************************************************/
/* segx x=偶数=高4bit；x=奇数=低四4bit；                                                          */
/* seg0_3 两个数字管；p9有无交流电图标；                                           								*/
/* seg4 电池图标；                                                                  							*/
/* seg5 p8=故障图标；p7=报警图标；p6=撤防图标;p5=布防图标																					*/
/* seg6 p4=上传图标，p25=下载图标，p3=云端图标，p2=网卡图标                                       */ 
/* seg7_8 信号强度图标，其中，p1=SIM卡图标                                                        */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                             */
/**************************************************************************************************/

#define MODE_COM                0
#define MODE_DATA               1

#define LCD_PIN_CS              GPIO_Pin_9//GPIO_PIN_7
#define LCD_PIN_RD              GPIO_Pin_8
#define LCD_PIN_WR              GPIO_Pin_7//GPIO_PIN_9
#define LCD_PIN_DATA            GPIO_Pin_2//GPIO_PIN_10

#define COMMAND_SYS_DIS         ((uint8_t)0x00) /*关闭系统振荡器和LCD 偏压发生器*/
#define COMMAND_SYS_EN          ((uint8_t)0x01) /*打开系统振荡器*/
#define COMMAND_LCD_OFF         ((uint8_t)0x02) /*关闭LCD偏压发生器*/
#define COMMAND_LCD_ON          ((uint8_t)0x03) /*打开LCD偏压发生器*/
#define COMMAND_TIMER_DIS       ((uint8_t)0x04) /*时基输出失效*/
#define COMMAND_WDT_DIS         ((uint8_t)0x05) /*看门狗溢出标志输出失效*/
#define COMMAND_TIMER_EN        ((uint8_t)0x06) /*时基输出使能*/
#define COMMAND_WDT_EN          ((uint8_t)0x07) /*看门狗溢出标志输出有效*/
#define COMMAND_TONE_OFF        ((uint8_t)0x08) /*关闭声音输出*/
#define COMMAND_TONE_ON         ((uint8_t)0x09) /*打开声音输出*/
#define COMMAND_CLR_TIMER       ((uint8_t)0x0C) /*时基发生器清零*/
#define COMMAND_CLR_WDT         ((uint8_t)0x0E) /*清除WDT状态*/
#define COMMAND_XTAL_32K        ((uint8_t)0x14) /*系统时钟源_晶振*/
#define COMMAND_RC_256K         ((uint8_t)0x18) /*系统时钟源_片内RC振荡器*/
#define COMMAND_EXT_256K        ((uint8_t)0x1C) /*系统时钟源_外部时钟源*/
#define COMMAND_BIAS12_2COM     ((uint8_t)0x20) /*LCD 1/2偏压选项,2个输出口*/
#define COMMAND_BIAS12_3COM     ((uint8_t)0x24) /*LCD 1/2偏压选项,3个输出口*/
#define COMMAND_BIAS12_4COM     ((uint8_t)0x28) /*LCD 1/2偏压选项,4个输出口*/
#define COMMAND_BIAS13_2COM     ((uint8_t)0x21) /*LCD 1/3偏压选项,2个输出口*/
#define COMMAND_BIAS13_3COM     ((uint8_t)0x25) /*LCD 1/3偏压选项,3个输出口*/
#define COMMAND_BIAS13_4COM     ((uint8_t)0x29) /*LCD 1/3偏压选项,4个输出口*/
#define COMMAND_TONE_4K         ((uint8_t)0x40) /*声音频率4KHz*/
#define COMMAND_TONE_2K         ((uint8_t)0x50) /*声音频率2KHz*/
#define COMMAND_IRQ_DIS         ((uint8_t)0x80) /*使/IRQ 输出失效*/
#define COMMAND_IRQ_EN          ((uint8_t)0x88) /*使/IRQ 输出有效*/
#define COMMAND_TOPT            ((uint8_t)0xE0) /*测试模式*/
#define COMMAND_TNORMAL         ((uint8_t)0xE3) /*普通模式*/

#define LCD_GPIOE               GPIOE
#define LCD_GPIOB               GPIOB
#define DELAY_MS                5000

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
  SIM_NB=0x02,
  SIM_4G=0x04,
  SIM_5G=0x08
} HTB_SIM;

typedef enum
{
  ICO_BUFANG,                           /*布防图标*/
  ICO_CHEFANG,                          /*撤防图标*/
  ICO_BAOJING,                          /*报警图标*/
  ICO_GUZHANG,                          /*故障图标*/
  ICO_JIAOLIU,                          /*有无交流电*/
  ICO_NETWORK,                          /*有无网线*/
  ICO_SIM,                          		/*有无SIM卡*/
	ICO_NET,															/*链接网路*/
	ICO_LOAD,															/*下载数据*/
	ICO_UPDATE														/*上传数据*/
}HTB_ICO;

typedef enum
{
  ICO_OFF=0,                            /*隐藏图标*/
  ICO_ON=1                              /*显示图标*/
}HTB_ICO_STATE;
/**************************************************************************************************/
/*|  PIN  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12  |  13  |  14  |*/
/*|       |     |     |     |     |seg0 |seg1 |seg2 |seg3 |seg4 |seg5 |seg6 | seg7 | seg8 | seg9 |*/
/*| com1  |com1 |     |     |     | 1A  |  /  | 2A  | p9  | p21 | p8  | p4  | p20  | p1   | p15  |*/
/*| com2  |     |com2 |     |     | 1F  | 1B  | 2F  | 2B  | p24 | p7  | p25 | p19  | p10  | p14  |*/
/*| com3  |     |     |com3 |     | 1E  | 1G  | 2E  | 2G  | p23 | p6  | p3  | p18  | p11  | p13  |*/
/*| com4  |     |     |     |com4 | 1D  | 1C  | 2D  | 2C  | p22 | p5  | p2  | p17  | p16  | p12  |*/
/**************************************************************************************************/
/* segx x=偶数=高4bit；x=奇数=低四4bit；                                                          */
/* seg0_3 两个数字管；p9有无交流电图标；                                           								*/
/* seg4 电池图标；                                                                  							*/
/* seg5 p8=故障图标；p7=报警图标；p6=撤防图标;p5=布防图标																					*/
/* seg6 p4=上传图标，p25=下载图标，p3=云端图标，p2=网卡图标                                       */ 
/* seg7_8 信号强度图标，其中，p1=SIM卡图标                                                        */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                             */
/**************************************************************************************************/
typedef struct
{
  u8 num_seg0_1;                                /*数字1*/
  u8 num_seg2_3;                                /*数字2*/
  u8 ico_seg4_5;                                /*seg4(h_4bit)=电池和状态图标*/
  u8 ico_seg7_8;                                /*信号强度*/
  u8 ico_seg69;                                 /*各种图标,seg6和seg9=2G--5G*/
} HTB_RAM;
//-------------------------------------外部接口-----------------------------------------
void SetErrorCode(u8 value);                            /*设置错误码*/
void SetBatteryIco(u8 value);                           /*设置电池图标*/
void SetStateIco(u8 value,HTB_ICO_STATE ico_state);     /*状态图标*/
void SetSignalIco(u8 value);                            /*信号强度*/
void SetSimTypeIco(u8 value);                           /*2G-5G*/
void HtbLcdClear(void);                                 /*清空所有图标*/
void HtbLcdShow(void);                                  /*显示所有图标*/
void GPIO_Lcd_Init(void);
#endif
