#include "stm32f10x.h"
#include "numfont.h"
#include "LM096.h"
#include "led.h"
#define SSD1306_LCDWIDTH                    128
#define SSD1306_LCDHEIGHT                   64
#define I2C1_DR_Address                     0x40005410
#define _BV(bit) (1<<(bit))
I2C_InitTypeDef  I2C_InitStructure;
/*延时函数*/
void myDelay(__IO uint32_t nCount)
{
	 for(; nCount != 0; nCount--);
}
/*stm32 iic初始化*/
void pin_init()
{
	 GPIO_InitTypeDef GPIO_InitStructure;
	 /* Enable GPIOB clock */
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	 /* Enable I2C1 clock */
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	 GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
	 GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
	 GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	 GPIO_Init(GPIOE, &GPIO_InitStructure);

	 //I2C Config
	 I2C_SoftwareResetCmd(I2C1,ENABLE);
	 I2C_SoftwareResetCmd(I2C1,DISABLE);
	 I2C_Cmd(I2C1, ENABLE);
	 I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	 I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	 I2C_InitStructure.I2C_OwnAddress1 = 0x79;
	 I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	 I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	 I2C_InitStructure.I2C_ClockSpeed = 100000;
	 I2C_Init(I2C1, &I2C_InitStructure);
}
/*寄存器写操作，data是要写的数据，iscommand用于区分是显示数据还是寄存器数据*/

void ssd1306_send_byte_cmd(uint8_t data)
{
	 I2C_GenerateSTART(I2C1, ENABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	 I2C_Send7bitAddress(I2C1, 0x78, I2C_Direction_Transmitter);

	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	 I2C_SendData(I2C1,0x00);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	 I2C_SendData(I2C1,data);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	 I2C_GenerateSTOP(I2C1, ENABLE);
}
void ssd1306_fill_frame_buffer()
{
	 int i=0;

	 I2C_GenerateSTART(I2C1, ENABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	 I2C_Send7bitAddress(I2C1, 0x78, I2C_Direction_Transmitter);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


	 I2C_SendData(I2C1,0x40);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	 for(i=0;i<sizeof(buffer)/sizeof(uint8_t);i++)
	 {

		  I2C_SendData(I2C1,buffer[i]);
		  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	 } 
	 I2C_GenerateSTOP(I2C1, ENABLE);

}

/*对ssd1306进行复位操作*/
void device_rst()
{
	 GPIO_ResetBits(GPIOE, GPIO_Pin_1);//config iic address
	 GPIO_SetBits(GPIOE, GPIO_Pin_0);//rst
	 myDelay(10000);
	 GPIO_ResetBits(GPIOE, GPIO_Pin_0);
	 myDelay(10000);
	 GPIO_SetBits(GPIOE, GPIO_Pin_0);
}    

/*清除显示缓冲区*/
void clear(void) 
{
	 memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}
/*ssd1306芯片的初始化*/
void ssd1306_init() 
{
	 int i;
	 pin_init();
	 device_rst();
	 for(i=0;i<sizeof(init_reg);i++)
	 {
		  ssd1306_send_byte_cmd(init_reg[i]);
	 }
	 clear();
	 /*写入显示序列命令*/

	 for(i=0;i<sizeof(draw_reg);i++)
	 {						
		  ssd1306_send_byte_cmd(draw_reg[i]);
	 }
}

/*绘制指定点的像素*/
void setpixel(uint8_t x, uint8_t y,uint8_t clear) {
	 /*判断是否超出了边界128×64*/
	 if ((x >= SSD1306_LCDWIDTH) || (y >= SSD1306_LCDHEIGHT))
		  return;

	 // x is which column
	 //if (color == WHITE) 
	 /*clear用于确定是背景色黑色，还是前景色，主要在绘制电池图标时使用，TRUE是背景色，FALSE是前景色*/
	 if(clear)
		  buffer[x+ (y/8)*SSD1306_LCDWIDTH] = 0;  
	 else
		  buffer[x+ (y/8)*SSD1306_LCDWIDTH] |= _BV((y%8));  
	 //else
	 //buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~_BV((y%8)); 
}
/*画线接口，从(x0,y0)到(x1,y1)画一条直线*/
void drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	 /*确定x,y坐标的位差,原则是从小坐标的点画向大坐标的点*/
	 uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
	 uint8_t dx, dy,tmp;
	 int8_t err;
	 int8_t ystep;
	 /*交替x0,y0 x1,y1*/
	 if (steep) {
		  //swap(x0, y0);
		  //swap(x1, y1);
		  tmp=x0;
		  x0=y0;
		  y0=tmp;
		  tmp=x1;
		  x1=y1;
		  y1=tmp;
	 }
	 /*交替x0,x1 y0,y1*/
	 if (x0 > x1) {
		  //swap(x0, x1);
		  //swap(y0, y1);
		  tmp=x0;
		  x0=x1;
		  x1=tmp;
		  tmp=y0;
		  y0=y1;
		  y1=tmp;
	 }

	 /*计算画点xy长度*/
	 dx = x1 - x0;
	 dy = abs(y1 - y0);

	 err = dx / 2;

	 /*根据y0,y1的大小，计算step大小*/
	 if (y0 < y1) {
		  ystep = 1;
	 } else {
		  ystep = -1;}
	 /*从x0开始画点*/
	 for (; x0<x1; x0++) {
		  if (steep) {
			   setpixel(y0, x0,0);
		  } else {
			   setpixel(x0, y0,0);
		  }
		  err -= dy;
		  if (err < 0) {
			   y0 += ystep;
			   err += dx;
		  }
	 }
}

/*从xy坐标开始填充宽度w，高度h的区域*/
void fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,uint8_t clear) {

	 // stupidest version - just pixels - but fast with internal buffer!
	 uint8_t i,j;
	 for (i=x; i<x+w; i++) {
		  for (j=y; j<y+h; j++) {
			   setpixel(i, j,clear);
		  }
	 }
}
/*绘图电池图标和数字信息，bat1是第一个电池的电量，bat2是第二个电池的电量，从0到100,c是数字串，总共6个数字，一个.*/
void draw(uint8_t bat1,uint8_t bat2,char *c)
{
	 int x=0,line=0,x1=0,i,j;
	 /*确定参数是否合法*/
	 if(bat1>100||bat1<0||bat2>100||bat2<0||strlen(c)!=7)
		  return ;
	 /*当bat1，bat2是1时，对电池图标做清空处理*/
	 if(bat1==1)
	 {
		  fillrect(2,31,28,15,1);
	 }
	 if(bat2==1)
	 {
		  fillrect(2,48,28,15,1);
	 }
	 /*绘制电池1*/
	 drawline(1,32,28,32);
	 drawline(28,32,28,36);
	 drawline(28,36,32,36);
	 drawline(32,36,32,43);
	 drawline(32,43,28,43);
	 drawline(28,43,28,47);
	 drawline(28,47,1,47);
	 drawline(1,47,1,32);
	 fillrect(1,32,(uint8_t)(bat1*28/100),15,0);
	 /*绘制电池2*/
	 drawline(1,49,28,49);
	 drawline(28,49,28,52);
	 drawline(28,52,32,52);
	 drawline(32,52,32,59);
	 drawline(32,59,28,59);
	 drawline(28,59,28,63);
	 drawline(28,63,1,63);
	 drawline(1,63,1,49);
	 fillrect(1,49,(uint8_t)(bat2*28/100),15,0);
	 while(c[0]!=0)
	 {
		  //need to draw char ,like 3300 4.7
		  if(c[0]!='.')
		  {	/*绘制数字0到9*/
			   x=x1;
			   /*从字库里提取数字的字模到显示缓冲区，一个数字占用4行每行32个点*/
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line)*128)]=font32[(c[0]-48)*128+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+1)*128)-32]=font32[(c[0]-48)*128+32+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+2)*128)-64]=font32[(c[0]-48)*128+64+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+3)*128)-96]=font32[(c[0]-48)*128+96+j];
					x++;				
			   }
		  }
		  else
		  {
			   /*绘制 .*/
			   x=x1;
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line)*128)]=font32[1280+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+1)*128)-32]=font32[1280+32+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+2)*128)-64]=font32[1280+64+j];
					x++;				
			   }
			   for(j=0;j<32;j++)
			   {				
					buffer[x+((line+3)*128)-96]=font32[1280+96+j];
					x++;				
			   }
		  }
		  /*x坐标增加31，一个数字或者.需要32个点*/
		  x1=x1+31;
		  if(x1+31>SSD1306_LCDWIDTH)
		  {/*超过了一行的边界，则行数+4，x从33开始绘制数字，绕过了电池的区域*/
			   x1=33;
			   line=line+4;
		  }

		  c++;

	 }	
}
void display(void) 
{

	 /*填充显示缓冲区到ssd1306*/
	 ssd1306_fill_frame_buffer();
}

