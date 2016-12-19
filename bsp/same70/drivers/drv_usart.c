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
 * 2015-08-01     xiaonong     the first version for same70f7xx
 */

#include "drv_usart.h"
#include "rt_board.h"

#include <rtdevice.h>
#include "board.h"
/** USART0 pin RX */
#define PIN_USART0_RXD_DBG \
	{PIO_PB0C_RXD0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
/** USART0 pin TX */
#define PIN_USART0_TXD_DBG \
	{PIO_PB1C_TXD0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}

#define CONSOLE_PINS      {PIN_USART0_TXD_DBG, PIN_USART0_RXD_DBG}

/* STM32 uart driver */
struct same70_uart
{
    Usart *UartHandle;
    IRQn_Type irq;
};

static rt_err_t same70_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct same70_uart *uart;
	const Pin pPins[] = CONSOLE_PINS;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct same70_uart *)serial->parent.user_data;
	// Disable the MATRIX registers write protection
	MATRIX->MATRIX_WPMR  = MATRIX_WPMR_WPKEY_PASSWD;
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

	PIO_Configure(pPins, PIO_LISTSIZE(pPins));

	// Reset & disable receiver and transmitter, disable interrupts
	uart->UartHandle->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RSTSTA;
	uart->UartHandle->US_IDR = 0xFFFFFFFF;
	PMC_EnablePeripheral(ID_USART0);
	uart->UartHandle->US_BRGR = (BOARD_MCK / cfg->baud_rate) / 16;

	// Configure mode register
	uart->UartHandle->US_MR
		= (US_MR_USART_MODE_NORMAL | US_MR_PAR_NO | US_MR_USCLKS_MCK
		   | US_MR_CHRL_8_BIT);

	// Enable receiver and transmitter
	uart->UartHandle->US_CR = US_CR_RXEN | US_CR_TXEN;
	//NVIC_ClearPendingIRQ(uart->irq);
	//NVIC_SetPriority(uart->irq , 1);
	//USART_EnableIt(uart->UartHandle, US_IER_RXRDY);
    return RT_EOK;
}

static rt_err_t same70_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct same70_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct same70_uart *)serial->parent.user_data;

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
        UART_ENABLE_IRQ(uart->irq);
        /* enable interrupt */
        USART_EnableIt(uart->UartHandle, US_IER_RXRDY);
        break;
    }

    return RT_EOK;
}

static int same70_putc(struct rt_serial_device *serial, char c)
{
    struct same70_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct same70_uart *)serial->parent.user_data;

	while ((uart->UartHandle->US_CSR & US_CSR_TXEMPTY) == 0);
	uart->UartHandle->US_THR = c;

    return 1;
}

static int same70_getc(struct rt_serial_device *serial)
{
    int ch;
    struct same70_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct same70_uart *)serial->parent.user_data;

    ch = -1;
    if (uart->UartHandle->US_CSR & US_CSR_RXRDY)
    {
        ch = uart->UartHandle->US_RHR & 0xff;
    }
	
    return ch;
}

static const struct rt_uart_ops same70_uart_ops =
{
    same70_configure,
    same70_control,
    same70_putc,
    same70_getc,
};

#if defined(RT_USING_UART1)
/* UART1 device driver structure */

static struct same70_uart uart1;
struct rt_serial_device serial1;

void USART0_Handler(void)
{
    struct same70_uart *uart;

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
#endif /* RT_USING_UART1 */

int same70_hw_usart_init(void)
{
    struct same70_uart *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef RT_USING_UART1
    uart = &uart1;
    uart->UartHandle = USART0;
	uart->irq = USART0_IRQn;
    serial1.ops    = &same70_uart_ops;
    serial1.config = config;

    /* register UART1 device */
    rt_hw_serial_register(&serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART1 */

    return 0;
}
INIT_BOARD_EXPORT(same70_hw_usart_init);
