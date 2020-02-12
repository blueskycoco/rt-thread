#include <stm32f0xx.h>

#define SDA GPIO_Pin_2
#define SCL GPIO_Pin_3
GPIO_InitTypeDef  GPIO_InitStructure;
void delay()
{
	volatile int i=0,j;
	for(i=0;i<1500;i++)
		j=3;
	//rt_thread_delay(1);
}
int i2c_start()  
{
	//GPIO_SetBits(GPIOA, SDA);
	//GPIO_SetBits(GPIOA, SCL);
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	//GPIO_InitStructure.GPIO_Pin = SDA;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);	
	//GPIO_InitStructure.GPIO_Pin = SCL;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	delay();
	GPIO_ResetBits(GPIOA, SDA);
	delay();  
}  

void i2c_stop()  
{  
	GPIO_SetBits(GPIOA, SCL);
	GPIO_ResetBits(GPIOA, SDA);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = SDA;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	delay();  
	GPIO_SetBits(GPIOA, SDA);
}  
unsigned char i2c_read_ack()  
{  
	unsigned char r;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = SDA;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOA, SCL);
	r = GPIO_ReadInputDataBit(GPIOA,SDA);
	delay();  
	GPIO_SetBits(GPIOA, SCL);
	delay();  
	//rt_kprintf("ack is %d\n",r);
return r;  
}  
void i2c_write_byte(unsigned char b)  
{  
	int i,j=0;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = SDA;

	for (i=7; i>=0; i--) 
	{  
		GPIO_ResetBits(GPIOA, SCL);
		if(b&(1<<i))
			GPIO_SetBits(GPIOA, SDA);
		else
			GPIO_ResetBits(GPIOA, SDA);	
		if(j==0)
		{			
			GPIO_Init(GPIOA, &GPIO_InitStructure);	
			j=1;
		}		
		delay();  
		GPIO_SetBits(GPIOA, SCL);
		delay();  
	}  
	i2c_read_ack();
}
int i2c_send_ack()  
{  
	GPIO_ResetBits(GPIOA, SCL);
	GPIO_ResetBits(GPIOA, SDA);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = SDA;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	delay();  
	GPIO_SetBits(GPIOA, SCL);
	delay();  
}  

unsigned char i2c_read_byte()  
{  
	int i;  
	unsigned char r = 0;  
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = SDA;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	for (i=7; i>=0; i--) {
		GPIO_ResetBits(GPIOA, SCL);
		delay();  
		r = (r <<1) | GPIO_ReadInputDataBit(GPIOA,SDA);
		GPIO_SetBits(GPIOA, SCL);
		delay();  
	}  
	i2c_send_ack();
	return r;  
}  
void i2c_read(unsigned char addr, unsigned char sub_addr,unsigned char* buf, int len)  
{  
	int i;  
	unsigned char t;  
	i2c_start();
	i2c_write_byte(addr<<1);  
	i2c_write_byte(sub_addr);  
	i2c_stop();
	rt_thread_delay(1);
	i2c_start();
	t = (addr << 1) | 1; 
	i2c_write_byte(t);  
	for (i=0; i<len; i++)  
	buf[i] = i2c_read_byte();  
	i2c_stop();
}  

void i2c_write (unsigned char addr, unsigned char* buf, int len)  
{  
int i;  
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

i2c_start();
i2c_write_byte(addr<<1);  
for (i=0; i<len; i++)  
{
	i2c_write_byte(0x00);
	i2c_write_byte(buf[i]);
}
i2c_stop();
}  

