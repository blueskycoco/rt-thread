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

void HTB_SetNumberValue(u8 *number,u8 value)
{
  u8 p1=*number&0x08;
  switch(value)
  {
    case 0:
      *number=0xF5|p1;
      break;
    case 1:
      *number=0x05|p1;
      break;
    case 2:
      *number=0xB6|p1;
      break; 
    case 3:
      *number=0x97|p1;
      break;
    case 4:
      *number=0x47|p1;
      break;
    case 5:
      *number=0xD3|p1;
      break;
    case 6:
      *number=0xF3|p1;
      break;
    case 7:
      *number=0x85|p1;
      break;
    case 8:
      *number=0xF7|p1;
      break;
    case 9:
      *number=0xD7|p1;
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
        htbRam.ico_seg7_8|=0x04;//显示布防
      }
      else
      {
        htbRam.ico_seg7_8&=0xFB;//隐藏布防
      }
      break;
    case ICO_CHEFANG:                                                           /*撤防图标*/
      if(value)
      {
        htbRam.ico_seg69|=0x10;//显示撤防
      }
      else
      {
        htbRam.ico_seg69&=0xEF;//隐藏撤防
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
  htbRam.ico_seg4_5&=0xF0;
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
  htbRam.ico_seg4_5&=0x0F;
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
  //发送没用的第9位
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

void HTB_Lcd_Show() 
{ 
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  for (int i=0; i<=16;i++)
   HTB_Write_8bitData(0xFF);
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

/**
   * @brief 清除所有图标
   * @param None
   * @retval None
   */
void HtbLcdClear()
{
  HTB_Lcd_Clr();
}
/**
   * @brief 显示所有图标
   * @param None
   * @retval None
   */
void HtbLcdShow()
{
  HTB_Lcd_Show();
}
/**
   * @brief 设置错误码
   * @param 十进制错误码 范围0-99
   * @retval None
   */
void SetErrorCode(u8 value)
{
  u8 height=value/10;
  u8 low=value%10;
  HTB_SetNumberValue(&htbRam.num_seg0_1,height);
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  HTB_Write_8bitData(htbRam.num_seg0_1);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
  
  HTB_SetNumberValue(&htbRam.num_seg2_3,low);
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(2);
  HTB_Write_8bitData(htbRam.num_seg2_3);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}
/**
   * @brief 设置电池
   * @param 十进制数据
   *         0-4表示电池的0-4个等级，0等级图标不显示，1等级显示一个格...4等级显示所有格
   * @retval None
   */
void SetBatteryIco(u8 value)
{
  u8 level=value%5;
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(4);
  HTB_SetBatteryIco((HTB_LEVEL)level);
  HTB_Write_8bitData(htbRam.ico_seg4_5);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}
/**
   * @brief 设置wifi信号
   * @param 十进制数据
   *         0-4表示wifi的 0-4个等级，0等级图标不显示，1等级显示一个圆弧...4等级显示所有圆弧
   * @retval None
   */
void SetWifiIco(u8 value)
{
  u8 level=value%5;
  GPIO_ResetBits(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(4);
  HTB_SetWifiIco((HTB_LEVEL)level);
  HTB_Write_8bitData(htbRam.ico_seg4_5);
  GPIO_SetBits(LCD_GPIOE,LCD_PIN_CS);
}
/**
   * @brief 设置信号强度图标
   * @param 十进制数据
   *         0-5表示信号的 0-5个等级，0等级信号强度图标不显示，无信号图标显示，1等级显示一个格...5等级显示所有格
   * @retval None
   */
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
/**
   * @brief 设置网络制式 2G 、3G等图标
   * @param 十进制数据 范围0-4
   *         0：不显示
   *         1：显示2G
   *         2：显示3G
   *         3：显示4G
   *         4：显示5G
   * @retval None
   */
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
/**
   * @brief 设置状态图标
   * @param 十进制数据 范围0-6
   *         0：显示布防
   *         1：显示撤防
   *         2：显示报警
   *         3：显示故障
   *         4：显示交流电
   *         5：显示网络连接
   *         6：显示流量卡连接
   * @retval None
   */
void SetStateIco(u8 value,HTB_ICO_STATE ico_state)
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
  HTB_SetStateIco(sim,ico_state);
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

void HTB_Lcd_Init()
{
  /*GPIO_ResetBits(LCD_PIN_CS);
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
  GPIO_SetBits(LCD_PIN_CS);*/
  
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

