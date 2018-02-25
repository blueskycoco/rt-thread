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
/* segx x=偶数=高4bit；x=奇数=低四4bit；                                                           */
/* seg0_3 两个数字管；p6=有无网络图标；p7有无交流电图标；                                           */
/* seg4 电池图标； seg5 wifi图标                                                                   */
/* seg6 p5=sim卡图标，p4=故障图标，p3=报警图标，p2=撤防图标                                         */ 
/* seg7_8 信号强度图标，其中，p1=布放图标                                                           */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                              */
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
  ICO_BUFANG,                           /*布防图标*/
  ICO_CHEFANG,                          /*撤防图标*/
  ICO_BAOJING,                          /*报警图标*/
  ICO_GUZHANG,                          /*故障图标*/
  ICO_JIAOLIU,                          /*有无交流电*/
  ICO_NETWORK,                          /*有无网线*/
  ICO_SIM                               /*有无SIM卡*/
}HTB_ICO;

typedef enum
{
  ICO_OFF=0,                            /*隐藏图标*/
  ICO_ON=1                              /*显示图标*/
}HTB_ICO_STATE;
/**************************************************************************************************/
/*|  PIN  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12  |  13  |  14  |*/
/*|       |     |     |     |     |seg0 |seg1 |seg2 |seg3 |seg4 |seg5 |seg6 | seg7 | seg8 | seg9 |*/
/*| com1  |com1 |     |     |     | 1A  | p6  | 2A  | p7  | s8  | s1  | p5  | p13  | p18  | p17  |*/
/*| com2  |     |com2 |     |     | 1F  | 1B  | 2F  | 2B  | s7  | s2  | p4  | p12  | p1   | p16  |*/
/*| com3  |     |     |com3 |     | 1E  | 1G  | 2E  | 2G  | s6  | s3  | p3  | p11  | p8   | p15  |*/
/*| com4  |     |     |     |com4 | 1D  | 1C  | 2D  | 2C  | s5  | s4  | p2  | p10  | p9   | p14  |*/
/**************************************************************************************************/
/* segx x=偶数=高4bit；x=奇数=低四4bit；                                                           */
/* seg0_3 两个数字管；p6=有无网络图标；p7有无交流电图标；                                           */
/* seg4 电池图标； seg5 wifi图标                                                                   */
/* seg6 p5=sim卡图标，p4=故障图标，p3=报警图标，p2=撤防图标                                         */ 
/* seg7_8 信号强度图标，其中，p1=布放图标                                                           */
/* seg9   p14=2G,p15=3G,p16=4G,p17=5G                                                              */
/**************************************************************************************************/
typedef struct
{
  u8 num_seg0_1;                                /*数字1*/
  u8 num_seg2_3;                                /*数字2*/
  u8 ico_seg4_5;                                /*seg4(h_4bit)=电池和seg5(l_4bit)=wifi*/
  u8 ico_seg7_8;                                /*信号强度*/
  u8 ico_seg69;                                 /*各种图标,seg6和seg9=2G--5G*/
} HTB_RAM;

void SetFirstTo0(u8 value);
void SetBatteryWifiIco(u8 value);
void SetStateIco(u8 value);

void HTB_SetNumberValue(u8 *num,u8 value);
void HTB_SetStateIco(HTB_ICO ico,HTB_ICO_STATE value);
void HTB_SetSignalIco(HTB_LEVEL value);/*信号强度*/
void HTB_SetSimTypeIco(HTB_SIM value);/*2G-5G*/
void HTB_SetWifiIco(HTB_LEVEL value);/*wifi*/
void HTB_SetBatteryIco(HTB_LEVEL value);/*电池*/

void GPIO_Lcd_Init(void);

void HTB_Lcd_Clr();
void HTB_Lcd_Init(void);
void HTB_Write_Mode(u8 mode);
void HTB_Write_Command(u8 command);
void HTB_Write_Address(u8 data);
void HTB_Write_8bitData(u8 data);
void HTB_Write_H4bitData(u8 data);
void HTB_Write_L4bitData(u8 data);
#endif

