/*
 * File      : drv_usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-08-01     xiaonong     the first version for samv71f7xx
 */

#include "drv_usart.h"
#include "rt_board.h"

#include <rtdevice.h>
#include "board.h"
/** USART1 pin RX */
#define PIN_USART1_RXD_DBG \
	{PIO_PA21A_RXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART1 pin TX */
#define PIN_USART1_TXD_DBG \
	{PIO_PB4D_TXD1, PIOB, ID_PIOB, PIO_PERIPH_D, PIO_DEFAULT}
//#define PINS_USART1        PIN_USART1_TXD_DBG, PIN_USART1_RXD_DBG

#define CONSOLE_PINS      {PIN_USART1_TXD_DBG, PIN_USART1_RXD_DBG}
extern sXdmad dmaDrv;

/* STM32 uart driver */
struct samv71_uart
{
    Usart *UartHandle;
    IRQn_Type irq;
	UsartChannel UsartTx;
	UsartChannel UsartRx;
	UsartDma Usartd;
	uint32_t mode;
	uint32_t baud;
};
rt_size_t samv71_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);

static rt_err_t samv71_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct samv71_uart *uart;
	const Pin pPins[] = CONSOLE_PINS;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct samv71_uart *)serial->parent.user_data;
	// Disable the MATRIX registers write protection
	MATRIX->MATRIX_WPMR  = MATRIX_WPMR_WPKEY_PASSWD;
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

	PIO_Configure(pPins, PIO_LISTSIZE(pPins));

	// Reset & disable receiver and transmitter, disable interrupts
	uart->UartHandle->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RSTSTA;
	uart->UartHandle->US_IDR = 0xFFFFFFFF;
	PMC_EnablePeripheral(ID_USART1);
	uart->UartHandle->US_BRGR = (BOARD_MCK / cfg->baud_rate) / 16;
	uart->baud = cfg->baud_rate;
	// Configure mode register
	uart->UartHandle->US_MR
		= (US_MR_USART_MODE_NORMAL | US_MR_PAR_NO | US_MR_USCLKS_MCK
		   | US_MR_CHRL_8_BIT);
	uart->mode = (US_MR_USART_MODE_NORMAL | US_MR_PAR_NO | US_MR_USCLKS_MCK
		   | US_MR_CHRL_8_BIT);
	// Enable receiver and transmitter
	uart->UartHandle->US_CR = US_CR_RXEN | US_CR_TXEN;
	//NVIC_ClearPendingIRQ(uart->irq);
	//NVIC_SetPriority(uart->irq , 1);
	//USART_EnableIt(uart->UartHandle, US_IER_RXRDY);
    return RT_EOK;
}

static rt_err_t samv71_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct samv71_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct samv71_uart *)serial->parent.user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        UART_DISABLE_IRQ(uart->irq);
        /* disable interrupt */
		USART_DisableIt(uart->UartHandle, US_IER_RXRDY);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        ///UART_ENABLE_IRQ(uart->irq);
        /* enable interrupt */
        //USART_EnableIt(uart->UartHandle, US_IER_RXRDY);
        break;
    }

    return RT_EOK;
}

static int samv71_putc(struct rt_serial_device *serial, char c)
{
    struct samv71_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct samv71_uart *)serial->parent.user_data;

	while ((uart->UartHandle->US_CSR & US_CSR_TXEMPTY) == 0);
	uart->UartHandle->US_THR = c;

    return 1;
}

static int samv71_getc(struct rt_serial_device *serial)
{
    int ch;
    struct samv71_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct samv71_uart *)serial->parent.user_data;

    ch = -1;
    if (uart->UartHandle->US_CSR & US_CSR_RXRDY)
    {
        ch = uart->UartHandle->US_RHR & 0xff;
    }
	
    return ch;
}

static const struct rt_uart_ops samv71_uart_ops =
{
    samv71_configure,
    samv71_control,
    samv71_putc,
    samv71_getc,
    samv71_dma_transmit,
};

#if defined(RT_USING_UART1)
/* UART1 device driver structure */

static struct samv71_uart uart1;
struct rt_serial_device serial1;

void USART1_Handler(void)
{
    struct samv71_uart *uart;

    uart = &uart1;

    /* enter interrupt */
    rt_interrupt_enter();
	    
    if (uart->UartHandle->US_CSR & US_CSR_RXRDY)
    {
        rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_RX_IND);
        /* Clear RXNE interrupt flag */
        //__HAL_UART_SEND_REQ(&uart->UartHandle, UART_RXDATA_FLUSH_REQUEST);
    }
    /* leave interrupt */
    rt_interrupt_leave();
}
static void samv71_USARTD_Tx_Cb(uint32_t channel, struct samv71_uart *pArg)
{
	UsartChannel *pUsartdCh = pArg->Usartd.pTxChannel;

	if (channel != pUsartdCh->ChNum)
		return;

	/* Release the DMA channels */
	//XDMAD_FreeChannel(pArg->Usartd.pXdmad, pUsartdCh->ChNum);

	//pUsartdCh->dmaProgress = 1;
	USARTD_DisableTxChannels(&(pArg->Usartd), &(pArg->UsartTx));
	rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_TX_DMADONE);
}
static void samv71_USARTD_Rx_Cb(uint32_t channel, struct samv71_uart *pArg)
{

	UsartChannel *pUsartdCh = pArg->Usartd.pRxChannel;

	if (channel != pUsartdCh->ChNum)
		return;

	/* Release the DMA channels */
	//XDMAD_FreeChannel(pArg->Usartd.pXdmad, pUsartdCh->ChNum);
	//pUsartdCh->dmaProgress = 1;
	USARTD_DisableRxChannels(&(pArg->Usartd), &(pArg->UsartRx));
	SCB_InvalidateDCache_by_Addr((uint32_t *)pUsartdCh->pBuff, pUsartdCh->BuffSize);
	rt_hw_serial_isr(&serial1, (pUsartdCh->BuffSize << 8)|RT_SERIAL_EVENT_RX_DMADONE);
}

rt_size_t samv71_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    struct samv71_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct samv71_uart *)serial->parent.user_data;
	if (direction == RT_SERIAL_DMA_TX)
	{
		rt_memset(&(uart->UsartTx), 0, sizeof(UsartChannel));
		uart->UsartTx.BuffSize = size;
		uart->UsartTx.pBuff = buf;
		uart->UsartTx.dmaProgress = 1;
		uart->UsartTx.dmaProgrammingMode = XDMAD_SINGLE;
		uart->Usartd.pXdmad = &dmaDrv;
		uart->Usartd.pTxChannel = &(uart->UsartTx);
		uart->Usartd.pTxChannel->callback = (UsartdCallback)samv71_USARTD_Tx_Cb;
		uart->Usartd.pTxChannel->pArgument= uart;
		USARTD_Configure(&(uart->Usartd), ID_USART1, uart->mode, uart->baud, BOARD_MCK);
		USARTD_EnableTxChannels(&(uart->Usartd), &(uart->UsartTx));
		USARTD_SendData(&(uart->Usartd));
	}
	else
	{
		rt_memset(&(uart->UsartRx), 0, sizeof(UsartChannel));
		uart->UsartRx.BuffSize = size;
		uart->UsartRx.pBuff = buf;
		uart->UsartRx.dmaProgress = 1;
		uart->UsartRx.dmaProgrammingMode = XDMAD_SINGLE;
		uart->Usartd.pXdmad = &dmaDrv;
		uart->Usartd.pRxChannel = &(uart->UsartRx);
		uart->Usartd.pRxChannel->callback = (UsartdCallback)samv71_USARTD_Rx_Cb;
		uart->Usartd.pRxChannel->pArgument= uart;
		USARTD_Configure(&(uart->Usartd), ID_USART1, uart->mode, uart->baud, BOARD_MCK);
		USARTD_EnableRxChannels(&(uart->Usartd), &(uart->UsartRx));
		USARTD_RcvData(&(uart->Usartd));
	}
	return size;
}
#endif /* RT_USING_UART1 */

int samv71_hw_usart_init(void)
{
    struct samv71_uart *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef RT_USING_UART1
    uart = &uart1;
    uart->UartHandle = USART1;
	uart->irq = USART1_IRQn;
    serial1.ops    = &samv71_uart_ops;
	config.bufsz = 0;
    serial1.config = config;
    /* register UART1 device */
    rt_hw_serial_register(&serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX | RT_DEVICE_FLAG_DMA_RX,
                          uart);
#endif /* RT_USING_UART1 */

    return 0;
}
INIT_BOARD_EXPORT(samv71_hw_usart_init);
