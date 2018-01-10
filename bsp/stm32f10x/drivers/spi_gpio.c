#include <rtthread.h>
#include <stm32f10x.h>
#include <stdint.h>
#include "cc1101.h"

#define PIN_CLK		GPIO_Pin_1
#define PORT_CLK		GPIOC
#define PIN_MOSI		GPIO_Pin_0
#define PORT_MOSI		GPIOC
#define PIN_MISO		GPIO_Pin_2
#define PORT_MISO		GPIOC
#define PIN_CS		GPIO_Pin_5
#define PORT_CS		GPIOC
#define PIN_GDO0	GPIO_Pin_4
#define PORT_GDO0	GPIOC
#define PIN_GDO2	GPIO_Pin_3
#define PORT_GDO2	GPIOC
#define GPIO_PortSourceX GPIO_PortSourceGPIOC
#define GPIO_PinSourceX GPIO_PinSource4
#define EXTI_IRQnX EXTI4_IRQn
#define EXTI_LineX	EXTI_Line4
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
	rt_event_init(&cc1101_event, "cc1101_event", RT_IPC_FLAG_FIFO );	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Pin = PIN_CS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT_CS, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_MOSI;
	GPIO_Init(PORT_MOSI, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin = PIN_CLK;
	GPIO_Init(PORT_CLK, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_InitStructure.GPIO_Pin = PIN_MISO;
	GPIO_Init(PORT_MISO, &GPIO_InitStructure);	
}
void trxRfSpiInterruptInit()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
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

	EXTI_InitStructure.EXTI_Line = EXTI_LineX;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(EXTI_LineX);
}
void mosi(int type)
{
    if(type)
    {
        GPIO_SetBits(PORT_MOSI,PIN_MOSI);
    }
    else
    {
        GPIO_ResetBits(PORT_MOSI,PIN_MOSI);
    }
}
void clk(int type)
{
    if(type)
    {
        GPIO_SetBits(PORT_CLK,PIN_CLK);
    }
    else
    {
        GPIO_ResetBits(PORT_CLK,PIN_CLK);
    }
}
int miso()
{
    if(GPIO_ReadInputDataBit(PORT_MISO,PIN_MISO) ==SET)
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

static void trxReadWriteBurstSingle(uint8 addr,uint8 *pData,uint16 len)
{
	uint16 i;
	if(addr&RADIO_READ_ACCESS)
	{
		if(addr&RADIO_BURST_ACCESS)
		{
			for (i = 0; i < len; i++)
			{
				*pData = spi_send_rcv(0);
				pData++;
			}
		}
		else
		{
			*pData = spi_send_rcv(0);
		}
	}
	else
	{
		if(addr&RADIO_BURST_ACCESS)
		{
			for (i = 0; i < len; i++)
			{
				spi_send_rcv(*pData);
				pData++;
			}
		}
		else
		{
			spi_send_rcv(*pData);
		}
	}
	return;
}
rfStatus_t trx8BitRegAccess(uint8 accessType, uint8 addrByte, uint8 *pData, uint16 len)
{
	uint8 readValue;
	RF_SPI_BEGIN();
	while(miso());
	readValue = spi_send_rcv(accessType|addrByte);
	trxReadWriteBurstSingle(accessType|addrByte,pData,len);
	RF_SPI_END();
	return(readValue);
}

rfStatus_t trxSpiCmdStrobe(uint8 cmd)
{
	uint8 rc;
	RF_SPI_BEGIN();
	while(miso());
	rc = spi_send_rcv(cmd);
	RF_SPI_END();
	return(rc);
}

