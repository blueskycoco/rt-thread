
#include <stm32f0xx.h>
#include <rthw.h>
#include <rtthread.h>
//#define DLY 10
//extern void Delay(__IO uint32_t nCount);
GPIO_InitTypeDef	GPIO_InitStructure;
#define  SkipROM    0xCC     //跳过ROM
//#define  SearchROM  0xF0  //搜索ROM
//#define  ReadROM    0x33  //读ROM
//#define  MatchROM   0x55  //匹配ROM
//#define  AlarmROM   0xEC  //告警ROM
#define  StartConvert    0x44  //开始温度转换，在温度转换期间总线上输出0，转换结束后输出1
#define  ReadScratchpad  0xBE  //读暂存器的9个字节
//#define  WriteScratchpad 0x4E  //写暂存器的温度告警TH和TL
//#define  CopyScratchpad  0x48  //将暂存器的温度告警复制到EEPROM，在复制期间总线上输出0，复制完后输出1
//#define  RecallEEPROM    0xB8    //将EEPROM的温度告警复制到暂存器中，复制期间输出0，复制完成后输出1
//#define  ReadPower       0xB4    //读电源的供电方式：0为寄生电源供电；1为外部电源供电
//#define EnableINT()  
//#define DisableINT()
#define DS_PORT   GPIOA
#define DS_DQIO   GPIO_Pin_1
#define DS_RCC_PORT  RCC_AHBPeriph_GPIOA
//#define DS_PRECISION 0x7f   //精度配置寄存器 1f=9位; 3f=10位; 5f=11位; 7f=12位;
//#define DS_AlarmTH  0x64
//#define DS_AlarmTL  0x8a
//#define DS_CONVERT_TICK 1000
#define ResetDQ() GPIO_ResetBits(DS_PORT,DS_DQIO)
#define SetDQ()  GPIO_SetBits(DS_PORT,DS_DQIO)
#define GetDQ()  GPIO_ReadInputDataBit(DS_PORT,DS_DQIO)
static unsigned char TempX_TAB[16]={0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x09};
void Delay_us(unsigned long Nus) 
{  
 /*SysTick->LOAD=Nus*9;          //时间加载       
 SysTick->CTRL|=0x01;             //开始倒数     
 while(!(SysTick->CTRL&(1<<16))); //等待时间到达  
 SysTick->CTRL=0X00000000;        //关闭计数器 
 SysTick->VAL=0X00000000;         //清空计数器      
*/
	uint32_t ticks;
    uint32_t told,tnow,tcnt=0;
    uint32_t reload = SysTick->LOAD; /* LOAD的值 */
                 
    ticks = Nus * 100;                  /* 需要的节拍数 */               
    tcnt = 0;
    told = SysTick->VAL;             /* 刚进入时的计数器值 */

    while(1)
    {
        tnow = SysTick->VAL;    
        if(tnow != told)
        {    
            /* 这里注意一下SYSTICK是一个递减的计数器 */    
            if(tnow < told)
            {
                tcnt += told - tnow;    
            }
            /* 重新装载递减 */
            else 
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            /*时间超过/等于要延迟的时间,则退出 */
            if(tcnt >= ticks)break;
        }  
    }
} 
unsigned char ResetDS18B20(void)
{
 unsigned char resport=1;
 unsigned long x=0;
 SetDQ();
 Delay_us(50);
 
 ResetDQ();
 Delay_us(500);  //500us （该时间的时间范围可以从480到960微秒）
 SetDQ();
 Delay_us(40);  //40us
 //resport = GetDQ();
 //rt_kprintf("ResetDS18B20 1\r\n");
//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//GPIO_Init(GPIOA, &GPIO_InitStructure);	

 while(GetDQ())// x++;
 {
 	 x++;
 	//Delay_us(500);  //500us
 	if(x>10000)
		break;
 	}
 //		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//		GPIO_Init(GPIOA, &GPIO_InitStructure);	
Delay_us(500);
 SetDQ();
// rt_kprintf("ResetDS18B20 2\r\n");
if(x>10000)
	resport=0;
 return resport;
}
void DS18B20WriteByte(unsigned char Dat)
{
 unsigned char i;
 for(i=8;i>0;i--)
 {
   ResetDQ();     //在15u内送数到数据线上，DS18B20在15-60u读数
  Delay_us(5);    //5us
  if(Dat & 0x01)
   SetDQ();
  else
   ResetDQ();
  Delay_us(65);    //65us
  SetDQ();
  Delay_us(2);    //连续两位间应大于1us
  Dat >>= 1; 
 } 
}
unsigned char DS18B20ReadByte(void)
{
 unsigned char i,Dat;
 SetDQ();
 Delay_us(5);
 for(i=8;i>0;i--)
 {
   Dat >>= 1;
    ResetDQ();     //从读时序开始到采样信号线必须在15u内，且采样尽量安排在15u的最后
  Delay_us(5);   //5us
  SetDQ();
  Delay_us(5);   //5us
  //		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

  if(GetDQ())
    Dat|=0x80;
  else
   Dat&=0x7f;  
	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

  Delay_us(65);   //65us
  SetDQ();
 }
 return Dat;
}
/*
void ReadRom(unsigned char *Read_Addr)
{
 unsigned char i;
 DS18B20WriteByte(ReadROM);
  
 for(i=8;i>0;i--)
 {
  *Read_Addr=DS18B20ReadByte();
  Read_Addr++;
 }
}
void DS18B20Init(unsigned char Precision,unsigned char AlarmTH,unsigned char AlarmTL)
{
 ResetDS18B20();
 DS18B20WriteByte(SkipROM); 
 DS18B20WriteByte(WriteScratchpad);
 DS18B20WriteByte(AlarmTL);
 DS18B20WriteByte(AlarmTH);
 DS18B20WriteByte(Precision);
 ResetDS18B20();
 DS18B20WriteByte(SkipROM); 
 DS18B20WriteByte(CopyScratchpad);
 	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

 while(!GetDQ());  //等待复制完成 ///////////
 	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

}
*/
unsigned char DS18B20StartConvert(void)
{
	unsigned char result=0;
	result= ResetDS18B20();
	if(result==0)
	{
		return 0;
	}else
	{
		 DS18B20WriteByte(SkipROM); 
		 DS18B20WriteByte(StartConvert); 
	}
	return 1;
}
void DS18B20_Configuration(void)
{
 //GPIO_InitTypeDef GPIO_InitStructure;
 
 RCC_AHBPeriphClockCmd(DS_RCC_PORT, ENABLE);
 GPIO_InitStructure.GPIO_Pin = DS_DQIO;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //开漏输出
 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //2M时钟速度
 GPIO_Init(DS_PORT, &GPIO_InitStructure);
}

int  ds18b20_start(void)
{
	 DS18B20_Configuration();
	// DS18B20Init(DS_PRECISION, DS_AlarmTH, DS_AlarmTL);
	 return DS18B20StartConvert();
}
unsigned short ds18b20_read(void)
{
	unsigned char TemperatureL,TemperatureH;
	unsigned int  Temperature,result=0;
	//rt_kprintf("read 0\r\n");
	result=ResetDS18B20();
	if(result==0)
		return 0;
	else
	{
		DS18B20WriteByte(SkipROM); 
		DS18B20WriteByte(ReadScratchpad);
		TemperatureL=DS18B20ReadByte();
		TemperatureH=DS18B20ReadByte(); 
		//rt_kprintf("read 1\r\n");
		result=ResetDS18B20();
		if(result==0)
			return 0;
		else
		{
			if(TemperatureH & 0x80)
			{
				TemperatureH=(~TemperatureH) | 0x08;
				TemperatureL=~TemperatureL+1;
				if(TemperatureL==0)
					TemperatureH+=1;
			}
			TemperatureH=(TemperatureH<<4)+((TemperatureL&0xf0)>>4);
			TemperatureL=TempX_TAB[TemperatureL&0x0f];
			//bit0-bit7为小数位，bit8-bit14为整数位，bit15为正负位
			Temperature=TemperatureH;
			Temperature=(Temperature<<8) | TemperatureL; 
			//rt_kprintf("read 2\r\n");
			DS18B20StartConvert();
		}
	}
	//rt_kprintf("read 3\r\n");
 return  Temperature;
}
//INIT_DEVICE_EXPORT(ds18b20_start);

