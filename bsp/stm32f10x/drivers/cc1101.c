#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x.h>
#include <rtdevice.h>
#include "cc1101.h"
#define HW 1
#define GDO0_H (1<<0)
#define GDO0_L (1<<1)
//#define STM32F103ZET6
#ifdef STM32F103ZET6
#define PIN_CS		GPIO_Pin_4
#define PORT_CS		GPIOA
#define PIN_GDO0	GPIO_Pin_5
#define PORT_GDO0	GPIOC
#define PIN_GDO2	GPIO_Pin_4
#define PORT_GDO2	GPIOC
#define GPIO_PortSourceX GPIO_PortSourceGPIOC
#define GPIO_PinSourceX GPIO_PinSource5
#define EXTI_IRQnX EXTI9_5_IRQn
#define EXTI_LineX	EXTI_Line5
#else
#define PIN_CS		GPIO_Pin_1
#define PORT_CS		GPIOA
#define PIN_GDO0	GPIO_Pin_2
#define PORT_GDO0	GPIOA
#define PIN_GDO2	GPIO_Pin_3
#define PORT_GDO2	GPIOA
#define GPIO_PortSourceX GPIO_PortSourceGPIOA
#define GPIO_PinSourceX GPIO_PinSource2
#define EXTI_IRQnX EXTI2_IRQn
#define EXTI_LineX	EXTI_Line2
#endif
struct rt_event cc1101_event;
#if 1
void cs(int type)
{
	if(type)
	{
		GPIO_SetBits(PORT_CS,PIN_CS);
	}
	else
	{
		GPIO_ResetBits(PORT_CS,PIN_CS);
	}
}
void mosi(int type)
{
	if(type)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_7);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);
	}
}
void clk(int type)
{
	if(type)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	}
}
int miso()
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) ==SET)
		return 1;
	else
		return 0;
}
void _nop_()
{
	volatile long i,j;
	for(i=0;i<5;i++)
		j=0;
}
uint8_t spi_send_rcv(uint8_t data)
{
	uint8_t i,temp;
	temp = 0;

	clk(0);
	for(i=0; i<8; i++)
	{
		if(data & 0x80)
		{
			mosi(1);
		}
		else mosi(0);
		data <<= 1;

		clk(1); 
		_nop_();
		_nop_();

		temp <<= 1;
		if(miso())temp++; 
		clk(0);
		_nop_();
		_nop_();	
	}
	return temp;
}
void spi_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI  MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_CS;
	GPIO_Init(PORT_CS, &GPIO_InitStructure);

	/* SPI MISO pin
	 * configuration */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI NSS
	 * pin
	 * configuration
	 *      *
	 *      */

}
void halWait(uint16_t timeout) {
	do {
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_(); 
	} while (--timeout);
}
void reset_cs()
{
	cs(0);
	clk(0);
	cs(1);
	rt_thread_delay(10);
	cs(1);
	rt_thread_delay(1);
	cs(0);
	rt_thread_delay(1);
	cs(1);
	rt_thread_delay(10);
}
#else
void spi_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	SPI_I2S_DeInit(SPI1);

	/* Enable the SPI peripheral */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* Enable SCK, MOSI, MISO and NSS GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	/* SPI pin mappings */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_0);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* SPI SCK pin
	 * configuration
	 * */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI
	 * MOSI
	 * pin
	 * configuration
	 * */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI
	 * MISO
	 * pin
	 * configuration
	 * */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI
	 * NSS
	 * pin
	 * configuration
	 *      *
	 *      */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIN_CS;
	GPIO_Init(PORT_CS, &GPIO_InitStructure);

	/* SPI
	 * configuration
	 *      *
	 *      -------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	/* Initializes
	 * the
	 * SPI
	 * communication
	 * */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &(SPI_InitStructure));

	/* Initialize
	 * the
	 * FIFO
	 * threshold
	 * */
	SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_HF);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
	/* Enable
	 * NSS
	 * output
	 * for
	 * master
	 * mode
	 * */
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_ERR, ENABLE);
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);
	SPI_Cmd(SPI1, ENABLE);
}
int check_status(uint8_t bit)
{
	int i=0;
	while(SPI_I2S_GetITStatus(SPI1,bit)!=SET)
	{
		i++;
		if(i==100){
			DEBUG("check bit %x timeout\r\n",bit);
			return RT_FALSE;
		}
		rt_thread_delay(1);
	}
	return RT_TRUE;
}
uint8_t spi_send_rcv(uint8_t *data,int len)
{
	int i=0;
	uint8_t r=0;
	/* Enable the SPI peripheral */


	for(i=0;i<len;i++)
	{
		if(check_status(SPI_I2S_IT_TXE))
			SPI_SendData8(SPI1, data[i]);
		if(check_status(SPI_I2S_IT_RXNE))
			r=SPI_ReceiveData8(SPI1);	   
	}

	return r;
}

void reset_cs()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIN_CS;
	GPIO_Init(PORT_CS, &GPIO_InitStructure);

	GPIO_SetBits(PORT_CS,PIN_CS);

	rt_thread_delay(1);

	GPIO_ResetBits(PORT_CS,PIN_CS);

	rt_thread_delay(1);

	GPIO_SetBits(PORT_CS,PIN_CS);

	rt_thread_delay(2);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_0);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	return;
}
#endif
int cc1101_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	rt_bool_t status = RT_FALSE;

	rt_event_init(&cc1101_event, "cc1101_event", RT_IPC_FLAG_FIFO );

	spi_init();

	cc1101_hw_init();


	/* cc1101 int init
	 *      * */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  PIN_GDO0;
	GPIO_Init(PORT_GDO0, &GPIO_InitStructure);
#if HW
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSourceX);	

	/* Configure the
	 * SPI interrupt
	 * priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI_IRQnX;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_LineX;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Clear
	 * DM9000A
	 * EXTI
	 * line
	 * pending
	 * bit
	 * */
	EXTI_ClearITPendingBit(EXTI_LineX);
#endif
	return RT_TRUE;
}

void cc1101_isr()
{

	if(GPIO_ReadInputDataBit(PORT_GDO0, PIN_GDO0) ==SET)
	{
		rt_event_send(&cc1101_event,GDO0_H);

	}
	else
	{
		rt_event_send(&cc1101_event,GDO0_L);

	}
}

int wait_int(int flag)
{
	rt_uint32_t ev;
	if(flag)
	{
		/*wait for gdo0 to h */
#if 1
#if CC1101_RCV
		if( rt_event_recv( &cc1101_event, GDO0_H, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ) != RT_EOK ) 
#else
			if( rt_event_recv( &cc1101_event, GDO0_H, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 200, &ev ) != RT_EOK ) 
#endif
			{
#if !CC1101_RCV
				rt_kprintf("wait for h failed\r\n");
				cc1101_hw_init();
#endif
				return RT_FALSE;
			}
#else
		while(GPIO_ReadInputDataBit(PORT_GDO2,PIN_GDO2)==RESET);
#endif
	}
	else
	{
		/*wait for gdo0 to l */
#if 1
#if CC1101_RCV
		if( rt_event_recv( &cc1101_event, GDO0_L, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ) != RT_EOK ) 
#else
			if( rt_event_recv( &cc1101_event, GDO0_L, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 200, &ev ) != RT_EOK ) 
#endif
			{
#if !CC1101_RCV
				rt_kprintf("wait for l failed\r\n");
				cc1101_hw_init();
#endif
				return RT_FALSE;
			}
#else
		while(GPIO_ReadInputDataBit(PORT_GDO2,PIN_GDO2)==SET);
#endif
	}
	return RT_TRUE;
}

void cc1101_send(uint8_t *buf,uint8_t len)
{
	return cc1101_send_packet(buf,len);
}

uint8_t cc1101_recv(uint8_t len)
{
	uint8_t *len1;
	len1=&len;
	uint8_t buf[11];
	cc1101_rcv_packet(buf, len1);

	return *len1;
}
void cc1101_read_reg(uint8_t reg)
{
	uint8_t val=read_cc1101(reg, RT_NULL, 0,2);
	rt_kprintf("reg %x , val is %x\r\n",reg,val);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT_ALIAS(cc1101_send, csend,send data by cc1101);
FINSH_FUNCTION_EXPORT_ALIAS(cc1101_recv, crcv,recv data by cc1101);
FINSH_FUNCTION_EXPORT_ALIAS(cc1101_init, cinit,init cc1101);
FINSH_FUNCTION_EXPORT_ALIAS(cc1101_read_reg,creg,read reg);
#endif

