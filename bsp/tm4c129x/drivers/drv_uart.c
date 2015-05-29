/*
 * File      : drv_uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009-2013 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-05-18     Bernard      The first version for LPC40xx
 * 2014-07-18     ArdaFu       Port to TM4C129X
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
//#include <components.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
typedef struct hw_uart_device
{
    uint32_t hw_base; // base address
}hw_uart_t;

#define mUartGetHwPtr(serial) ((hw_uart_t*)(serial->parent.user_data))

static rt_err_t hw_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{	
	uint32_t config = 0;
	hw_uart_t* uart;
    RT_ASSERT(serial != RT_NULL);
    uart = mUartGetHwPtr(serial);

	MAP_UARTDisable(uart->hw_base);
	// build UART Configuration parameter structure
    switch(cfg->data_bits)
	{
	case DATA_BITS_9:
		// enable 9bit address mode and set DATA_BIT_8
		MAP_UART9BitEnable(uart->hw_base);
	case DATA_BITS_8:
		config |= UART_CONFIG_WLEN_8;
		break;
	case DATA_BITS_7:
		config |= UART_CONFIG_WLEN_7;
		break;
	case DATA_BITS_6:
		config |= UART_CONFIG_WLEN_6;
		break;
	case DATA_BITS_5:
		config |= UART_CONFIG_WLEN_5;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	switch(cfg->parity)
	{
	case PARITY_ODD:
		config |= UART_CONFIG_PAR_ODD;
		break;
	case PARITY_EVEN:
		config |= UART_CONFIG_PAR_EVEN;
		break;
	case PARITY_NONE:
		config |= UART_CONFIG_PAR_NONE;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	switch(cfg->stop_bits)
	{
	case STOP_BITS_1:
		config |= UART_CONFIG_STOP_ONE;
		break;
	case STOP_BITS_2:
		config |= UART_CONFIG_STOP_TWO;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	
	// Initialize UART0 peripheral with given to corresponding parameter
    MAP_UARTConfigSetExpClk(uart->hw_base, SysClock, cfg->baud_rate, config);
	MAP_UARTFIFOEnable(uart->hw_base);
//rt_kprintf("set uart baud %d\r\n",cfg->baud_rate);
	// Enable the UART.
	MAP_UARTEnable(uart->hw_base);
    return RT_EOK;
}

static rt_err_t hw_control(struct rt_serial_device *serial, int cmd, void *arg)
{
	hw_uart_t* uart;
    RT_ASSERT(serial != RT_NULL);
    uart = mUartGetHwPtr(serial);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        MAP_UARTIntDisable(uart->hw_base, UART_INT_RX | UART_INT_RT |UART_INT_TX);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        MAP_UARTIntEnable(uart->hw_base, UART_INT_RX | UART_INT_RT |UART_INT_TX);
        break;
    }

    return RT_EOK;
}

static int hw_putc(struct rt_serial_device *serial, char c)
{
	hw_uart_t* uart;
    RT_ASSERT(serial != RT_NULL);
    uart = mUartGetHwPtr(serial);
	
    MAP_UARTCharPut(uart->hw_base, *((uint8_t *)&c));
    return 1;
}

static int hw_getc(struct rt_serial_device *serial)
{
	hw_uart_t* uart;
	RT_ASSERT(serial != RT_NULL);
	uart = mUartGetHwPtr(serial);

	return MAP_UARTCharGetNonBlocking(uart->hw_base);
}

static const struct rt_uart_ops hw_uart_ops =
{
    hw_configure,
    hw_control,
    hw_putc,
    hw_getc,
};

#if defined(RT_USING_UART0)
/* UART0 device driver structure */
struct rt_serial_device serial0;
hw_uart_t uart0 =
{
    UART0_BASE,
};

void UART0_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart0;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial0, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial0, RT_SERIAL_EVENT_TX_DONE);
    }	
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#if defined(RT_USING_UART1)
/* UART0 device driver structure */
struct rt_serial_device serial1;
hw_uart_t uart1 =
{
    UART1_BASE,
};

void UART1_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart1;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial1, RT_SERIAL_EVENT_TX_DONE);
    }		
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#if defined(RT_USING_UART2)
/* UART0 device driver structure */
struct rt_serial_device serial2;
hw_uart_t uart2 =
{
    UART2_BASE,
};

void UART2_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart2;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial2, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial2, RT_SERIAL_EVENT_TX_DONE);
    }		
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#if defined(RT_USING_UART3)
/* UART0 device driver structure */
struct rt_serial_device serial3;
hw_uart_t uart3 =
{
    UART3_BASE,
};

void UART3_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart3;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial3, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial3, RT_SERIAL_EVENT_TX_DONE);
    }		
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#if defined(RT_USING_UART4)
/* UART0 device driver structure */
struct rt_serial_device serial4;
hw_uart_t uart4 =
{
    UART4_BASE,
};

void UART4_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart4;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial4, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial4, RT_SERIAL_EVENT_TX_DONE);
    }		
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif
#if defined(RT_USING_UART6)
/* UART0 device driver structure */
struct rt_serial_device serial6;
hw_uart_t uart6 =
{
    UART6_BASE,
};

void UART6_IRQHandler(void)
{
	uint32_t intsrc;
    hw_uart_t *uart = &uart6;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart->hw_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial6, RT_SERIAL_EVENT_RX_IND);
    }
	if (intsrc & (UART_INT_TX))
    {
        MAP_UARTIntClear(uart->hw_base, intsrc);
        rt_hw_serial_isr(&serial6, RT_SERIAL_EVENT_TX_DONE);
    }		
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

int rt_hw_uart_init(int use_uart)
{
	hw_uart_t* uart;
	struct serial_configure config;

	config.baud_rate = BAUD_RATE_115200;
	config.bit_order = BIT_ORDER_LSB;
	config.data_bits = DATA_BITS_8;
	config.parity    = PARITY_NONE;
	config.stop_bits = STOP_BITS_1;
	config.invert    = NRZ_NORMAL;
	config.bufsz     = RT_SERIAL_RB_BUFSZ;

#ifdef RT_USING_UART0
	uart = &uart0;
	serial0.ops    = &hw_uart_ops;
	serial0.config = config;

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
	MAP_GPIOPinConfigure(GPIO_PA1_U0TX);

	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	/* preemption = 1, sub-priority = 1 */
	//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

	/* Enable Interrupt for UART channel */
	UARTIntRegister(uart->hw_base, UART0_IRQHandler);
	MAP_IntEnable(INT_UART0);
	MAP_UARTEnable(uart->hw_base);
	/* register UART0 device */
	rt_hw_serial_register(&serial0, "uart0",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif
	if(use_uart){
#ifdef RT_USING_UART1
	uart = &uart1;
	serial1.ops	 = &hw_uart_ops;
	serial1.config = config;

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	MAP_GPIOPinConfigure(GPIO_PB0_U1RX);
	MAP_GPIOPinConfigure(GPIO_PB1_U1TX);

	MAP_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	/* preemption = 1, sub-priority = 1 */
	//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

	/* Enable Interrupt for UART channel */
	UARTIntRegister(uart->hw_base, UART1_IRQHandler);
	MAP_IntEnable(INT_UART1);
	MAP_UARTEnable(uart->hw_base);
	MAP_IntPrioritySet(INT_UART1, 0);

	/* register UART0 device */
	rt_hw_serial_register(&serial1, "uart1",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif
#ifdef RT_USING_UART2
	uart = &uart2;
	serial2.ops	 = &hw_uart_ops;
	serial2.config = config;

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	MAP_GPIOPinConfigure(GPIO_PD4_U2RX);
	MAP_GPIOPinConfigure(GPIO_PD5_U2TX);

	MAP_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

	/* preemption = 1, sub-priority = 1 */
	//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

	/* Enable Interrupt for UART channel */
	UARTIntRegister(uart->hw_base, UART2_IRQHandler);
	MAP_IntEnable(INT_UART2);
	MAP_UARTEnable(uart->hw_base);
	MAP_IntPrioritySet(INT_UART2, 0);

	/* register UART0 device */
	rt_hw_serial_register(&serial2, "uart2",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif
#ifdef RT_USING_UART3
	uart = &uart3;
	serial3.ops	 = &hw_uart_ops;
	serial3.config = config;

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_GPIOPinConfigure(GPIO_PA4_U3RX);
	MAP_GPIOPinConfigure(GPIO_PA5_U3TX);

	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);

	/* preemption = 1, sub-priority = 1 */
	//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

	/* Enable Interrupt for UART channel */
	UARTIntRegister(uart->hw_base, UART3_IRQHandler);
	MAP_IntEnable(INT_UART3);
	MAP_UARTEnable(uart->hw_base);
	MAP_IntPrioritySet(INT_UART3, 0);

	/* register UART0 device */
	rt_hw_serial_register(&serial3, "uart3",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif
#ifdef RT_USING_UART4
	uart = &uart4;
	serial4.ops	 = &hw_uart_ops;
	serial4.config = config;

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_GPIOPinConfigure(GPIO_PA2_U4RX);
	MAP_GPIOPinConfigure(GPIO_PA3_U4TX);

	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);

	/* preemption = 1, sub-priority = 1 */
	//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

	/* Enable Interrupt for UART channel */
	UARTIntRegister(uart->hw_base, UART4_IRQHandler);
	MAP_IntEnable(INT_UART4);
	MAP_UARTEnable(uart->hw_base);
	MAP_IntPrioritySet(INT_UART4, 0);

	/* register UART0 device */
	rt_hw_serial_register(&serial4, "uart4",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif
#ifdef RT_USING_UART6
		uart = &uart6;
		serial6.ops  = &hw_uart_ops;
		serial6.config = config;
	
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
		MAP_GPIOPinConfigure(GPIO_PP0_U6RX);
		MAP_GPIOPinConfigure(GPIO_PP1_U6TX);
	
		MAP_GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
	
		/* preemption = 1, sub-priority = 1 */
		//IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));
	
		/* Enable Interrupt for UART channel */
		UARTIntRegister(uart->hw_base, UART6_IRQHandler);
		MAP_IntEnable(INT_UART6);
		MAP_UARTEnable(uart->hw_base);
		MAP_IntPrioritySet(INT_UART6, 0);
		/* register UART0 device */
		rt_hw_serial_register(&serial6, "uart6",RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX| RT_DEVICE_FLAG_INT_TX,uart);
#endif

	}
	return 0;
}
//INIT_BOARD_EXPORT(rt_hw_uart_init);
