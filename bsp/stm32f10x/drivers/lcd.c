#include "lcd.h"
#include <rtdevice.h>
extern rt_uint8_t g_ac;
extern rt_uint8_t g_low_power;
//#include "delay.h"
struct rt_mutex g_lcd_lock;

//-------------------------------------内部方法-----------------------------------------
static void HTB_SetNumberValue(u8 *num,u8 value);                      /*设置数码*/
static void HTB_SetStateIco(HTB_ICO ico,HTB_ICO_STATE value);          /*状态图标*/
static void HTB_SetSignalIco(HTB_LEVEL value);                         /*信号强度*/
static void HTB_SetSimTypeIco(HTB_SIM value);                          /*2G-5G*/
static void HTB_SetBatteryIco(HTB_LEVEL value);                        /*电池*/

static void GPIO_Pin_ReSet(GPIO_TypeDef *gpiox,u16 pin);
static void GPIO_Pin_Set(GPIO_TypeDef *gpiox,u16 pin);
static void HTB_Lcd_Init(void);

static void HTB_Lcd_Clr(void);
static void HTB_Lcd_Show(void);
static void HTB_Write_Mode(u8 mode);
static void HTB_Write_Command(u8 command);
static void HTB_Write_Address(u8 data);
static void HTB_Write_8bitData(u8 data);
static void HTB_Write_H4bitData(u8 data);
static void HTB_Write_L4bitData(u8 data);
static void HTB_Delay_ms(int ms);

HTB_RAM htbRam;
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
	if (g_low_power)
		return;
  HTB_Lcd_Show();
}
/**
   * @brief 设置错误码
   * @param 十进制错误码 范围0-99
   * @retval None
   */
void SetErrorCode(u8 value)
{
	if (g_low_power)
		return;
	rt_mutex_take(&g_lcd_lock,RT_WAITING_FOREVER);
  u8 height=value/10;
  u8 low=value%10;
  HTB_SetNumberValue(&htbRam.num_seg0_1,height);
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  HTB_Write_8bitData(htbRam.num_seg0_1);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  
  HTB_SetNumberValue(&htbRam.num_seg2_3,low);
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(2);
  HTB_Write_8bitData(htbRam.num_seg2_3);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  rt_mutex_release(&g_lcd_lock);
}
/**
   * @brief 设置电池
   * @param 十进制数据
   *         0-4表示电池的0-4个等级，0等级图标不显示，1等级显示一个格...4等级显示所有格
   * @retval None
   */
void SetBatteryIco(u8 value)
{
	if (g_ac)
		return ;
	if (g_low_power)
		return;
	rt_mutex_take(&g_lcd_lock,RT_WAITING_FOREVER);

  u8 level=value%5;
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(4);
  HTB_SetBatteryIco((HTB_LEVEL)level);
  HTB_Write_H4bitData(htbRam.ico_seg4_5);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  rt_mutex_release(&g_lcd_lock);
}

/**
   * @brief 设置信号强度图标
   * @param 十进制数据
   *         0-5表示信号的 0-5个等级，0等级信号强度图标不显示，无信号图标显示，1等级显示一个格...5等级显示所有格
   * @retval None
   */
void SetSignalIco(u8 value)
{
	if (g_low_power)
		return;
	rt_mutex_take(&g_lcd_lock,RT_WAITING_FOREVER);

  u8 level=value%6;
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(7);
  HTB_SetSignalIco((HTB_LEVEL)level);
  HTB_Write_8bitData(htbRam.ico_seg7_8);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  rt_mutex_release(&g_lcd_lock);
}
/**
   * @brief 设置网络制式 2G 、NB等图标
   * @param 十进制数据 范围0-4
   *         0：不显示
   *         1：显示2G
   *         2：显示NB
   *         3：显示4G
   *         4：显示5G
   * @retval None
   */
void SetSimTypeIco(u8 value)
{
	if (g_low_power)
		return;
	rt_mutex_take(&g_lcd_lock,RT_WAITING_FOREVER);

  u8 level=value%5;
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
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
      sim=SIM_NB;
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
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  rt_mutex_release(&g_lcd_lock);
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
   *         7：链接网路
   *         8：下载数据
   *         9：上传数据
   * @retval None
   */
void SetStateIco(u8 value,HTB_ICO_STATE ico_state)
{
	if (g_low_power)
		return;
	rt_mutex_take(&g_lcd_lock,RT_WAITING_FOREVER);

  u8 level=value%10;
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
		case 7:
      sim=ICO_NET;
      break;
		case 8:
      sim=ICO_LOAD;
      break;
		case 9:
      sim=ICO_UPDATE;
      break;
  }
  HTB_SetStateIco(sim,ico_state);
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  switch(sim)
  {
    case ICO_BUFANG:                                                            /*布防图标*/
    case ICO_CHEFANG:                                                           /*撤防图标*/
    case ICO_BAOJING:                                                           /*报警图标*/
    case ICO_GUZHANG:                                                           /*故障图标*/
      HTB_Write_Address(4);
      //写低位
      HTB_Write_8bitData(htbRam.ico_seg4_5);
      break;
    case ICO_SIM:
			HTB_Write_Address(7);
       //写全部
      HTB_Write_8bitData(htbRam.ico_seg7_8);
			break;
    case ICO_JIAOLIU:                                                           /*有无交流电*/
      HTB_Write_Address(2);
      //写全部
      HTB_Write_8bitData(htbRam.num_seg2_3);
      break;
    case ICO_NETWORK:                                                           /*有无网线*/
    case ICO_NET:                                                           		/*链接网路*/
		case ICO_LOAD:                                                           		/*下载数据*/
		case ICO_UPDATE:                                                           /*上传数据*/
      HTB_Write_Address(6);
      //写高位
      HTB_Write_H4bitData(htbRam.ico_seg69);
      break;
  }
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
  rt_mutex_release(&g_lcd_lock);
}

void HTB_Lcd_Clr() 
{ 
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  for (int i=0; i<=16;i++)
   HTB_Write_8bitData(0x00);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS); 
	htbRam.ico_seg4_5=0x00;
  htbRam.ico_seg69=0x00;
  htbRam.ico_seg7_8=0x00;
  htbRam.num_seg0_1=0x00;
  htbRam.num_seg2_3=0x00;
	
}
void HTB_Lcd_Show() 
{ 
	if (g_low_power)
		return;
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_DATA);
  HTB_Write_Address(0);
  for (int i=0; i<=16;i++)
   HTB_Write_8bitData(0xFF);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);    
}
void HTB_SetNumberValue(u8 *number,u8 value)
{
	if (g_low_power)
		return;
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
	if (g_low_power)
		return;
  switch(ico)
  {
    case ICO_BUFANG:                                                            /*布防图标*/
      if(value)
      {
        htbRam.ico_seg4_5|=0x01;//显示布防
				htbRam.ico_seg4_5&=0xFD;//隐藏撤防
      }
      else
      {
        htbRam.ico_seg4_5&=0xFE;//隐藏布防
				htbRam.ico_seg4_5|=0x02;//显示撤防
      }
      break;
    case ICO_CHEFANG:                                                           /*撤防图标*/
      if(value)
      {
        htbRam.ico_seg4_5|=0x02;//显示撤防
				htbRam.ico_seg4_5&=0xFE;//隐藏布防
      }
      else
      {
				htbRam.ico_seg4_5|=0x01;//显示布防
        htbRam.ico_seg4_5&=0xFD;//隐藏撤防
      }
      break;
    case ICO_BAOJING:                                                           /*报警图标*/
      if(value)
      {
        htbRam.ico_seg4_5|=0x04;
      }
      else
      {
        htbRam.ico_seg4_5&=0xFB;
      }
      break;
    case ICO_GUZHANG:                                                           /*故障图标*/
      if(value)
      {
        htbRam.ico_seg4_5|=0x08;
      }
      else
      {
        htbRam.ico_seg4_5&=0xF7;
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
    
    case ICO_SIM:                                                               /*有无SIM卡*/
      if(value)
      {
        htbRam.ico_seg7_8|=0x08;
      }
      else
      {
        htbRam.ico_seg7_8&=0xF7;
      }
      break;
		case ICO_NETWORK:                                                           /*有无网线*/
      if(value)
      {
        htbRam.ico_seg69|=0x10;
      }
      else
      {
        htbRam.ico_seg69&=0xEF;
      }
      break;
		case ICO_NET:                                                           		/*链接网路*/
			if(value)
      {
        htbRam.ico_seg69|=0x20;
      }
      else
      {
        htbRam.ico_seg69&=0x1F;
			}	
			break;
		case ICO_LOAD:                                                           		/*下载数据*/
			if(value)
      {
        htbRam.ico_seg69|=0x60;
      }
      else
      {
        htbRam.ico_seg69&=0xBF;
      }	
			break;
		case ICO_UPDATE:                                                            /*上传数据*/
			if(value)
      {
        htbRam.ico_seg69|=0xA0;
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
	if (g_low_power)
		return;
  u8 p1=htbRam.ico_seg7_8&0x08;
  switch(value)
  {
    case LEVEL0:
      htbRam.ico_seg7_8=0x04|p1;
      break;
    case LEVEL1:
      htbRam.ico_seg7_8=0x03|p1;
      break;
    case LEVEL2:
      htbRam.ico_seg7_8=0x13|p1;
      break;
    case LEVEL3:
      htbRam.ico_seg7_8=0x33|p1;
      break;
    case LEVEL4:
      htbRam.ico_seg7_8=0x73|p1;
      break;
    case LEVEL5:
      htbRam.ico_seg7_8=0xF3|p1;
      break;
  }
}
void HTB_SetSimTypeIco(HTB_SIM value)
{
  u8 simlevel=htbRam.ico_seg69&0xF0;
  htbRam.ico_seg69=value|simlevel;
}

void HTB_SetBatteryIco(HTB_LEVEL value)
{
	if (g_low_power)
		return;
  htbRam.ico_seg4_5&=0x0F;
  switch(value)
  {
    case LEVEL0:
      htbRam.ico_seg4_5|=0x00;
      break;
    case LEVEL1:
      htbRam.ico_seg4_5|=0x80;
      break;
    case LEVEL2:
      htbRam.ico_seg4_5|=0xC0;
      break;
    case LEVEL3:
      htbRam.ico_seg4_5|=0xE0;
      break;
    case LEVEL4:
      htbRam.ico_seg4_5|=0xF0;
      break;
		case LEVEL5:
      break;
  }
}

void GPIO_Lcd_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.GPIO_Pin=LCD_PIN_CS|LCD_PIN_RD|LCD_PIN_WR;
  GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;
  GPIO_Init(LCD_GPIOE,&GPIO_InitStruct);
	GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
	GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_RD);
	GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
  
  GPIO_InitStruct.GPIO_Pin=LCD_PIN_DATA;
  GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;
  GPIO_Init(LCD_GPIOB,&GPIO_InitStruct);
  GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);
	
  HTB_Lcd_Clr();
  HTB_Lcd_Init();
  rt_mutex_init(&(g_lcd_lock),	"lcd_lock",	RT_IPC_FLAG_FIFO);
}

void HTB_Lcd_Init()
{
  /*GPIO_Pin_ReSet(LCD_PIN_CS);
  HTB_Write_Mode(MODE_COM);
  HTB_Write_Command(COMMAND_SYS_EN);                                           //关闭系统震荡器
  HTB_Write_Command(COMMAND_LCD_ON);                                            //打开偏压器
  HTB_Write_Command(COMMAND_TIMER_EN);                                         //时基输出失效
  HTB_Write_Command(COMMAND_WDT_DIS);                                           //看门狗失效
  HTB_Write_Command(COMMAND_TONE_OFF);                                          //声音输出关闭
  HTB_Write_Command(COMMAND_EXT_256K);                                          //外部时钟
  HTB_Write_Command(COMMAND_BIAS13_4COM);                                       //LCD 1/3偏压选项,4个输出口
  HTB_Write_Command(COMMAND_IRQ_DIS);                                           //使/IRQ 输出失效
  HTB_Write_Command(COMMAND_TNORMAL);                                           //普通模式
  GPIO_Pin_Set(LCD_PIN_CS);*/
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_CS);
  HTB_Write_Mode(MODE_COM);
  HTB_Write_Command(COMMAND_SYS_EN);                                            //关闭系统震荡器
  HTB_Write_Command(COMMAND_LCD_ON);                                            //打开偏压器
  HTB_Write_Command(COMMAND_TIMER_EN);                                          //时基输出失效
  HTB_Write_Command(COMMAND_WDT_DIS);                                           //看门狗失效
  HTB_Write_Command(COMMAND_TONE_OFF);                                          //声音输出关闭
  HTB_Write_Command(COMMAND_RC_256K);                                           //内部时钟
  HTB_Write_Command(COMMAND_BIAS13_4COM);                                       //LCD 1/3偏压选项,4个输出口
  HTB_Write_Command(COMMAND_IRQ_DIS);                                           //使/IRQ 输出失效
  HTB_Write_Command(COMMAND_TNORMAL);                                           //普通模式
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_CS);
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
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);           //data=1,写入命令中的1;
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  HTB_Delay_ms(DELAY_MS);
  
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);         //data=0,写入命令中的0;
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  HTB_Delay_ms(DELAY_MS);
  
  //接下来写入命令中的第三位，根据模式匹配
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);           //wr=0;
  HTB_Delay_ms(DELAY_MS);
  if(mode)                              //mode=1,写模式
  {
    GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);         //data=1,写入命令中的1;
  }
  else
  {
    GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);         //data=0,写入命令中的0;
  }
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);             //wr=1;
  HTB_Delay_ms(DELAY_MS);
}

void HTB_Write_Command(u8 value)
{
  for(u8 i=0;i<8;i++)
  {
		HTB_Delay_ms(DELAY_MS);
    GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);
    }
    HTB_Delay_ms(DELAY_MS);
    GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
  }
  //发送没用的第9位
	HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);
  HTB_Delay_ms(DELAY_MS);
  GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
  HTB_Delay_ms(DELAY_MS);
}

void HTB_Write_Address(u8 value)
{
  /*读写命令格式 类型码+地址+数据
  *如：110+a5 a4 a3 a2 a1 a0+d0 d1 d2 d3*/
  value=value<<2;                                                           //地址只有6位
  for(u8 i=0;i<6;i++)
  {
    GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);
    }
    HTB_Delay_ms(DELAY_MS);
    GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
  }
}

void HTB_Write_8bitData(u8 value)
{
  for(u8 i=0;i<8;i++)
  {
    GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);
    }
    HTB_Delay_ms(DELAY_MS);
    GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
  }
}

void HTB_Write_H4bitData(u8 value)
{
  for(u8 i=0;i<4;i++)
  {
    GPIO_Pin_ReSet(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
    if((value>>(7-i)) & 0x01)
    {
      GPIO_Pin_Set(LCD_GPIOB,LCD_PIN_DATA);
    }
    else
    {
      GPIO_Pin_ReSet(LCD_GPIOB,LCD_PIN_DATA);
    }
    HTB_Delay_ms(DELAY_MS);
    GPIO_Pin_Set(LCD_GPIOE,LCD_PIN_WR);
    HTB_Delay_ms(DELAY_MS);
  }
}
void HTB_Write_L4bitData(u8 value)
{
  value=value<<4;
  HTB_Write_H4bitData(value);
}

void GPIO_Pin_ReSet(GPIO_TypeDef  *gpiox,u16 pin)
{
  GPIO_WriteBit(gpiox,pin,Bit_RESET);
}
void GPIO_Pin_Set(GPIO_TypeDef  *gpiox,u16 pin)
{
  GPIO_WriteBit(gpiox,pin,Bit_SET);
}

void HTB_Delay_ms(int ms)
{
	volatile u32 cnt = 1000;
	while(cnt)
	{
		cnt--;
	}
}

