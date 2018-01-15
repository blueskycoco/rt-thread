#include <rtthread.h>
#include <stm32f10x.h>
#include <stdint.h>
#include "cc1101.h"
#include "spi.h"
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
#define st(x)      do { x } while (__LINE__ == -1)
struct rt_event cc1101_event;
#define GDO0_H (1<<0)
#define GDO0_L (1<<1)
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

#define RF_SPI_BEGIN()              st( cs(0); )
#define RF_SPI_END()                st( cs(1); )
/*void cc1101_isr()
{	
	rt_kprintf("cc1101 isr\r\n");
	if(GPIO_ReadInputDataBit(PORT_GDO0, PIN_GDO0) ==SET)	
	{		
		rt_event_send(&cc1101_event,GDO0_H);	
	}	else	{	
		rt_event_send(&cc1101_event,GDO0_L);	
	}
}*/
int wait_int(int flag)
{	
	rt_uint32_t ev;	
	if(flag)	
	{		
		/*wait for gdo0 to h */		
		if( rt_event_recv( &cc1101_event, GDO0_H, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ) != RT_EOK ) 		
		{
			
			rt_kprintf("wait for h failed\r\n");				
			//cc1101_hw_init();
				
			return RT_FALSE;			
		}
	}	else	{		
		/*wait for gdo0 to l */
		if( rt_event_recv( &cc1101_event, GDO0_L, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ) != RT_EOK ) 			
		{
			
			rt_kprintf("wait for l failed\r\n");				
			//cc1101_hw_init();
					
			return RT_FALSE;			
		}

	}	
	return RT_TRUE;
}

void trxRfSpiInterfaceInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	SPI_InitTypeDef  SPI_InitStructure;	
	rt_event_init(&cc1101_event, "cc1101_event", RT_IPC_FLAG_FIFO );	
	SPI_I2S_DeInit(SPI1);	
	/* Enable the SPI peripheral */	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);	
	/* Enable SCK, MOSI, MISO and NSS GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);	
	/* SPI pin mappings */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	/* SPI SCK pin	 * configuration	 * */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	/* SPI	 * MOSI	 * pin	 * configuration	 * */	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	/* SPI	 * MISO	 * pin	 * configuration	 * */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	/* SPI	 * NSS	 * pin	 * configuration	 *      *	 *      */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Pin = PIN_CS;	
	GPIO_Init(PORT_CS, &GPIO_InitStructure);	
	/* SPI	 * configuration	 *      *	 *      -------------------------------------------------------*/	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	/* Initializes	 * the	 * SPI	 * communication	 * */	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	
	SPI_Init(SPI1, &(SPI_InitStructure));	
	/* Initialize	 * the	 * FIFO	 * threshold	 * */	
	//SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_HF);	
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);	
	/* Enable	 * NSS	 * output	 * for	 * master	 * mode	 * */	
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_ERR, ENABLE);	
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);	
	SPI_Cmd(SPI1, ENABLE);

}
void trxRfEnableInt()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_LineX;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(EXTI_LineX);
}
void trxRfDisableInt()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_LineX;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
}
int getIntFlag()
{
	return EXTI_GetFlagStatus(EXTI_Line2);
}
void clearIntFlag()
{
	EXTI_ClearFlag(EXTI_LineX);
}
void trxRfSpiInterruptInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin =  PIN_GDO0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT_GDO0, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceX, GPIO_PinSourceX);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI_IRQnX;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 2; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	trxRfEnableInt();

}
int gdo_level()
{
	return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2);
}
int check_status(uint8_t bit)
{	
	int i=0;	
	while(SPI_I2S_GetITStatus(SPI1,bit)!=SET);/*	
	{		
		i++;		
		if(i==100)
		{			
			rt_kprintf("check bit %x timeout\r\n",bit);			
			return RT_FALSE;		
		}		
		rt_thread_delay(1);	
	}	*/
	return RT_TRUE;
}

static void trxReadWriteBurstSingle(uint8 addr,uint8 *pData,uint16 len)
{
	uint16 i;
	if(addr&RADIO_READ_ACCESS)
	{
		if(addr&RADIO_BURST_ACCESS)
		{
			for (i = 0; i < len; i++)
			{
				//if(check_status(SPI_I2S_IT_TXE))
					SPI_SendData8(SPI1, 0);
				check_status(SPI_I2S_IT_TXE);
				if(check_status(SPI_I2S_IT_RXNE))			
					*pData=SPI_ReceiveData8(SPI1);
				pData++;
			}
		}
		else
		{
			//if(check_status(SPI_I2S_IT_TXE))
				SPI_SendData8(SPI1, 0);		
			check_status(SPI_I2S_IT_TXE);
			if(check_status(SPI_I2S_IT_RXNE))			
				*pData=SPI_ReceiveData8(SPI1);
		}
	}
	else
	{
		if(addr&RADIO_BURST_ACCESS)
		{
			for (i = 0; i < len; i++)
			{
				//if(check_status(SPI_I2S_IT_TXE))
					SPI_SendData8(SPI1, *pData);
				check_status(SPI_I2S_IT_TXE);
				pData++;
			}
		}
		else
		{
			//if(check_status(SPI_I2S_IT_TXE))
				SPI_SendData8(SPI1, *pData);
			check_status(SPI_I2S_IT_TXE);
		}
	}
	return;
}
rfStatus_t trx8BitRegAccess(uint8 accessType, uint8 addrByte, uint8 *pData, uint16 len)
{
	uint8 readValue;
	RF_SPI_BEGIN();
	while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) ==SET);
//	if(check_status(SPI_I2S_IT_TXE))
		SPI_SendData8(SPI1, accessType|addrByte);
	check_status(SPI_I2S_IT_TXE);
	if(check_status(SPI_I2S_IT_RXNE))		
		readValue = SPI_ReceiveData8(SPI1);
	trxReadWriteBurstSingle(accessType|addrByte,pData,len);
	RF_SPI_END();
	return(readValue);
}

rfStatus_t trxSpiCmdStrobe(uint8 cmd)
{
	uint8 rc;
	RF_SPI_BEGIN();
	while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) ==SET);
	//if(check_status(SPI_I2S_IT_TXE))
		SPI_SendData8(SPI1, cmd);
	check_status(SPI_I2S_IT_TXE);
	if(check_status(SPI_I2S_IT_RXNE))		
		rc = SPI_ReceiveData8(SPI1);
	RF_SPI_END();
	return(rc);
}


