/*
 * File      : usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-11-15     bright       the first version
 */

#include <stm32f0xx.h>
//#include <rtdevice.h>
#include "usart.h"
#include "led.h"
/* USART1 */
#define UART1_GPIO_TX			GPIO_Pin_2
#define UART1_GPIO_TX_SOURCE	GPIO_PinSource2
#define UART1_GPIO_RX			GPIO_Pin_3
#define UART1_GPIO_RX_SOURCE	GPIO_PinSource3
#define UART1_GPIO_AF			GPIO_AF_1
#define UART1_GPIO				GPIOA
#define RT_SERIAL_RB_BUFSZ 64
struct serial_ringbuffer
{
    unsigned char  buffer[RT_SERIAL_RB_BUFSZ];
    unsigned short put_index, get_index;
};
struct serial_ringbuffer *rbuffer;


int uart_config()
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	/*rbuffer=rt_malloc(sizeof(struct serial_ringbuffer));
	rt_memset(rbuffer->buffer, 0, sizeof(rbuffer->buffer));
	rbuffer->put_index = 0;
	rbuffer->get_index = 0;
*/
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(UART1_GPIO, UART1_GPIO_TX_SOURCE, UART1_GPIO_AF);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(UART1_GPIO, UART1_GPIO_RX_SOURCE, UART1_GPIO_AF);

	/* Configure USART Tx, Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX | UART1_GPIO_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART */
	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	USART_Cmd(USART1, ENABLE);
	return 0;
}

int uart_send(int index,unsigned char byte)
{
	while (!(USART1->ISR & USART_FLAG_TXE));
	USART1->TDR = byte;
}
char uart_recv(int index)
{
	char ch;
	ch = -1;
	if (USART1->ISR & USART_FLAG_RXNE)
	{
		ch = USART1->RDR & 0xff;
		
	}
	return ch;

}

void serial_ringbuffer_putc(struct serial_ringbuffer *rbuffer,
                                      char                      ch)
{
    rt_base_t level;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    rbuffer->buffer[rbuffer->put_index] = ch;
    rbuffer->put_index = (rbuffer->put_index + 1) & (RT_SERIAL_RB_BUFSZ - 1);

    /* if the next position is read index, discard this 'read char' */
    if (rbuffer->put_index == rbuffer->get_index)
    {
        rbuffer->get_index = (rbuffer->get_index + 1) & (RT_SERIAL_RB_BUFSZ - 1);
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);
}
int serial_ringbuffer_getc(struct serial_ringbuffer *rbuffer)
{
    int ch;
    rt_base_t level;

    ch = 0xff;
    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    if (rbuffer->get_index != rbuffer->put_index)
    {
        ch = rbuffer->buffer[rbuffer->get_index];
        rbuffer->get_index = (rbuffer->get_index + 1) & (RT_SERIAL_RB_BUFSZ - 1);
    }
    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return ch;
}

void USART1_IRQHandler(void)
{

	int ch=-1;
	/* enter interrupt */
	
	rt_interrupt_enter();
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		while (1)
		{
			ch = uart_recv(0);
			if (ch == 0xff)
			break;
			//uart_send(0,ch);
			serial_ringbuffer_putc(rbuffer, ch);
		}
		/* clear interrupt */
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		/* clear interrupt */
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}
void wifi_send(const char *s,int len)
{
	int i;
	for(i=0;i<len;i++)
	uart_send(0,s[i]);
}
unsigned long wifi_rcv(char *s,int size)
{
	//unsigned char *ptr;
	int i=0,j;
	unsigned long read_nbytes;
	//ptr = (unsigned char  *)s;
	
	int ch;
	j=0;
	for(i=0;i<size;i++)
	{
		while((ch=serial_ringbuffer_getc(rbuffer))==0xff)
		{
			j++;
			Delay_us(1);
			if(j==3000)
				return 0;
		}
		*s=ch;
		s++;
	}
	//ptr=(unsigned char *)s;
	//for(i=0;i<size;i++)
	//uart_send(0,*(ptr+i));
	return size;
}
//INIT_DEVICE_EXPORT(uart_config);

