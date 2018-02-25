#include "stm32f10x.h"
#include "usart.h"
#include "board.h"
#include <rtdevice.h>

#include "lcd.h"

HTB_RAM htbRam;
void ms_delay(int ms)
{
	volatile rt_uint32_t cnt = 0;
	while (cnt < ms*100)
		cnt++;
}
void SetFirstTo0(u8 value)
{
  HTB_SetNumberValue(&htbRam.num_seg0_1,value);
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  HTB_Write_8bitData(htbRam.num_seg0_1);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
  
  HTB_SetNumberValue(&htbRam.num_seg2_3,value);
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(2);
  HTB_Write_8bitData(htbRam.num_seg2_3);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
  
  
  /*GPIO_ResetBits(LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(7);
  HTB_Write_8bitData(0xFF);
  GPIO_SetBits(LCD_PIN_CS);*/
}

void SetBatteryWifiIco(u8 value)
{
  u8 level=value%5;
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(4);
  if(value<6)
  {
    HTB_SetWifiIco((HTB_LEVEL)level);
  }
  else
  {
    HTB_SetBatteryIco((HTB_LEVEL)level);
  }
  HTB_Write_8bitData(htbRam.ico_seg4_5);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}

void SetSignalIco(u8 value)
{
  u8 level=value%6;
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(7);
  HTB_SetSignalIco((HTB_LEVEL)level);
  HTB_Write_8bitData(htbRam.ico_seg7_8);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}

void SetSimTypeIco(u8 value)
{
  u8 level=value%5;
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(9);
  HTB_SIM sim;
  switch(level)
  {
    case 0:
      sim=SIM_0G;
      break;
    case 1:
      sim=SIM_2G;
      break;
    case 2:
      sim=SIM_3G;
      break;
    case 3:
      sim=SIM_4G;
      break;
    case 4:
      sim=SIM_5G;
      break;
  }
  
  HTB_SetSimTypeIco(sim);
  HTB_Write_L4bitData(htbRam.ico_seg69);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}

void SetStateIco(u8 value)
{
  u8 level=value%7;
  HTB_ICO sim;
  switch(level)
  {
    case 0:
      sim=ICO_BUFANG;
      break;
    case 1:
      sim=ICO_CHEFANG;
      break;
    case 2:
      sim=ICO_BAOJING;
      break;
    case 3:
      sim=ICO_GUZHANG;
      break;
    case 4:
      sim=ICO_JIAOLIU;
      break;
    case 5:
      sim=ICO_NETWORK;
      break;
    case 6:
      sim=ICO_SIM;
      break;
  }
  HTB_SetStateIco(sim,ICO_ON);
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  switch(sim)
  {
    case ICO_BUFANG:                                                            /*布防图标*/
      HTB_Write_Address(7);
      //写全部
      HTB_Write_8bitData(htbRam.ico_seg7_8);
      break;
    case ICO_CHEFANG:                                                           /*撤防图标*/
    case ICO_BAOJING:                                                           /*报警图标*/
    case ICO_SIM:
    case ICO_GUZHANG:                                                           /*故障图标*/
      HTB_Write_Address(6);
      //写高位
      HTB_Write_H4bitData(htbRam.ico_seg69);
      break;
      
    case ICO_JIAOLIU:                                                           /*有无交流电*/
      HTB_Write_Address(2);
      //写全部
      HTB_Write_8bitData(htbRam.num_seg2_3);
      break;
    case ICO_NETWORK:                                                           /*有无网线*/
      HTB_Write_Address(0);
      //写全部
      HTB_Write_8bitData(htbRam.num_seg0_1);
      break;
  }
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}

void HTB_Lcd_Clr() 
{ 
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  for (int i=0; i<=16;i++)
   HTB_Write_8bitData(0x00);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);    
}
void HTB_SetNumberValue(u8 *number,u8 value)
{
  switch(value)
  {
    case 0:
      *number=0xF5;
      break;
    case 1:
      *number=0x05;
      break;
    case 2:
      *number=0xB6;
      break; 
    case 3:
      *number=0x97;
      break;
    case 4:
      *number=0x47;
      break;
    case 5:
      *number=0xD3;
      break;
    case 6:
      *number=0xF3;
      break;
    case 7:
      *number=0x85;
      break;
    case 8:
      *number=0xF7;
      break;
    case 9:
      *number=0xD7;
      break;
  default:
    break;
  }
}

void HTB_SetStateIco(HTB_ICO ico,HTB_ICO_STATE value)
{
  switch(ico)
  {
    case ICO_BUFANG:                                                            /*布防图标*/
      if(value)
      {
        htbRam.ico_seg7_8|=0x04;
      }
      else
      {
        htbRam.ico_seg7_8&=0xFB;
      }
      break;
    case ICO_CHEFANG:                                                           /*撤防图标*/
      if(value)
      {
        htbRam.ico_seg69|=0x10;
      }
      else
      {
        htbRam.ico_seg69&=0xEF;
      }
      break;
    case ICO_BAOJING:                                                           /*报警图标*/
      if(value)
      {
        htbRam.ico_seg69|=0x20;
      }
      else
      {
        htbRam.ico_seg69&=0xDF;
      }
      break;
    case ICO_GUZHANG:                                                           /*故障图标*/
      if(value)
      {
        htbRam.ico_seg69|=0x40;
      }
      else
      {
        htbRam.ico_seg69&=0xBF;
      }
      break;
      
    case ICO_JIAOLIU:                                                           /*有无交流电*/
      if(value)
      {
        htbRam.num_seg2_3|=0x08;
      }
      else
      {
        htbRam.num_seg2_3&=0xF7;
      }
      break;
    case ICO_NETWORK:                                                           /*有无网线*/
      if(value)
      {
        htbRam.num_seg0_1|=0x08;
      }
      else
      {
        htbRam.num_seg0_1&=0xF7;
      }
      break;
    case ICO_SIM:                                                               /*有无SIM卡*/
      if(value)
      {
        htbRam.ico_seg69|=0x80;
      }
      else
      {
        htbRam.ico_seg69&=0x7F;
      }
      break;
  }
}
void HTB_SetSignalIco(HTB_LEVEL value)
{
  u8 p1=htbRam.ico_seg7_8&0x04;
  switch(value)
  {
    case LEVEL0:
      htbRam.ico_seg7_8=0x02|p1;
      break;
    case LEVEL1:
      htbRam.ico_seg7_8=0x09|p1;
      break;
    case LEVEL2:
      htbRam.ico_seg7_8=0x19|p1;
      break;
    case LEVEL3:
      htbRam.ico_seg7_8=0x39|p1;
      break;
    case LEVEL4:
      htbRam.ico_seg7_8=0x79|p1;
      break;
    case LEVEL5:
      htbRam.ico_seg7_8=0xF9|p1;
      break;
  }
}
void HTB_SetSimTypeIco(HTB_SIM value)
{
  u8 simlevel=htbRam.ico_seg69&0xF0;
  htbRam.ico_seg69=value|simlevel;
}

void HTB_SetWifiIco(HTB_LEVEL value)
{
  switch(value)
  {
    case LEVEL0:
      htbRam.ico_seg4_5|=0x00;
      break;
    case LEVEL1:
      htbRam.ico_seg4_5|=0x08;
      break;
    case LEVEL2:
      htbRam.ico_seg4_5|=0x0C;
      break;
    case LEVEL3:
      htbRam.ico_seg4_5|=0x0E;
      break;
    case LEVEL4:
      htbRam.ico_seg4_5|=0x0F;
      break;
  }
}
void HTB_SetBatteryIco(HTB_LEVEL value)
{
  switch(value)
  {
    case LEVEL0:
      htbRam.ico_seg4_5|=0x00;
      break;
    case LEVEL1:
      htbRam.ico_seg4_5|=0x10;
      break;
    case LEVEL2:
      htbRam.ico_seg4_5|=0x30;
      break;
    case LEVEL3:
      htbRam.ico_seg4_5|=0x70;
      break;
    case LEVEL4:
      htbRam.ico_seg4_5|=0xF0;
      break;
  }
}

void HTB_Write_Mode(u8 mode)
{
  /*
  *这里只实现 101、100命令
  *READ                 110
  *WRITE                101
  *READ-MODIFY-RITE     101
  *COMMAND              100
  */
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  ms_delay(DELAY_MS);
  GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);           //data=1,写入命令中的1;
  ms_delay(DELAY_MS);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  ms_delay(DELAY_MS);
  
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  ms_delay(DELAY_MS);
  GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);         //data=0,写入命令中的0;
  ms_delay(DELAY_MS);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  ms_delay(DELAY_MS);
  
  //接下来写入命令中的第三位，根据模式匹配
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  ms_delay(DELAY_MS);
  if(mode)                              //mode=1,写模式
  {
    GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);         //data=1,写入命令中的1;
  }
  else
  {
    GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);         //data=0,写入命令中的0;
  }
  ms_delay(DELAY_MS);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  ms_delay(DELAY_MS);
}

void HTB_Write_Command(u8 value)
{
  for(u8 i=0;i<8;i++)
  {
    GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    ms_delay(DELAY_MS);
    GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);
  }
  //发送没有的第9位
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);
  ms_delay(DELAY_MS);
  GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);
  ms_delay(DELAY_MS);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);
  ms_delay(DELAY_MS);
}

void HTB_Write_Address(u8 value)
{
  /*读写命令格式 类型码+地址+数据
  *如：110+a5 a4 a3 a2 a1 a0+d0 d1 d2 d3*/
  value=value<<2;                                                           //地址只有6位
  for(u8 i=0;i<6;i++)
  {
    GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    ms_delay(DELAY_MS);
    GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
  }
}

void HTB_Write_8bitData(u8 value)
{
  for(u8 i=0;i<8;i++)
  {
    GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    ms_delay(DELAY_MS);
    GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
  }
}

void HTB_Write_H4bitData(u8 value)
{
  for(u8 i=0;i<4;i++)
  {
    GPIO_ResetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_SetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_ResetBits(LCD_GPIOB,LCD_PIN_DATA);
    }
    ms_delay(DELAY_MS);
    GPIO_SetBits(LCD_GPIOE,LCD_PIN_WR);
    ms_delay(DELAY_MS);
  }
}
void HTB_Write_L4bitData(u8 value)
{
  value=value<<4;
  HTB_Write_H4bitData(value);
}

void HTB_Lcd_Init()
{
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_COM);
  HTB_Write_Command(COMMAND_SYS_EN);                                           //关闭系统震荡器
  HTB_Write_Command(COMMAND_LCD_ON);                                            //打开偏压器
  HTB_Write_Command(COMMAND_TIMER_EN);                                         //时基输出失效
  HTB_Write_Command(COMMAND_WDT_DIS);                                           //看门狗失效
  HTB_Write_Command(COMMAND_TONE_OFF);                                          //声音输出关闭
  HTB_Write_Command(COMMAND_RC_256K);                                          //外部时钟
  HTB_Write_Command(COMMAND_BIAS13_4COM);                                       //LCD 1/3偏压选项,4个输出口
  HTB_Write_Command(COMMAND_IRQ_DIS);                                           //使/IRQ 输出失效
  HTB_Write_Command(COMMAND_TNORMAL);                                           //普通模式
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}

void GPIO_Lcd_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin   = LCD_PIN_CS|LCD_PIN_RD|LCD_PIN_WR;
	GPIO_Init(LCD_GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_PIN_DATA;
	GPIO_Init(LCD_GPIOB, &GPIO_InitStructure);

	HTB_Lcd_Clr();

	HTB_Lcd_Init();
}

