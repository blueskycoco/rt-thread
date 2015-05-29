/*
 * File      : drv_uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */


#include "drv_uart.h"
#include "uart.h"
static struct rt_serial_device _ke02_serial0;  //abstracted serial for RTT
static struct rt_serial_device _ke02_serial1;  //abstracted serial for RTT

struct ke02_serial_device
{
    /* UART base address */
    UART_Type *baseAddress;

    /* UART IRQ Number */
    int irq_num;

    /* device config */
    struct serial_configure config;
};

//hardware abstract device
static struct ke02_serial_device _ke02_node_0 =
{
    (UART_Type *)UART0_BASE,
    UART0_IRQn,
};
static struct ke02_serial_device _ke02_node_1 =
{
    (UART_Type *)UART1_BASE,
    UART1_IRQn,
};

static rt_err_t _configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
	UART_Type *uart_reg;
	uart_reg = ((struct ke02_serial_device *)serial->parent.user_data)->baseAddress;

    switch ((unsigned int)uart_reg)
    {
    /*
     * if you're using other board
     * set clock and pin map for UARTx
     */
    case UART0_BASE:
	{
		UART_ConfigType sConfig;

	sConfig.u32SysClkHz = BUS_CLK_HZ;
	sConfig.u32Baudrate  = UART_PRINT_BITRATE;

	UART_Init (UART0, &sConfig);
        break;
    	}
    case UART1_BASE:
	{
		UART_ConfigType sConfig;

    	sConfig.u32SysClkHz = BUS_CLK_HZ;
    	sConfig.u32Baudrate  = UART_PRINT_BITRATE;
    
    	UART_Init (UART1, &sConfig);
	break;
    	}
    default:
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t _control(struct rt_serial_device *serial, int cmd, void *arg)
{
    UART_Type *uart_reg;
    int uart_irq_num = 0;

    uart_reg = ((struct ke02_serial_device *)serial->parent.user_data)->baseAddress;
    uart_irq_num = ((struct ke02_serial_device *)serial->parent.user_data)->irq_num;
	

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        //uart_reg->C2 &= ~UART_C2_RIE_MASK;
        //disable NVIC
        //NVIC->ICER[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
       // uart_reg->C2 |= UART_C2_RIE_MASK;
        //enable NVIC,we are sure uart's NVIC vector is in NVICICPR1
       // NVIC->ICPR[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
      //  NVIC->ISER[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
        break;
    case RT_DEVICE_CTRL_SUSPEND:
        /* suspend device */
       // uart_reg->C2  &=  ~(UART_C2_RE_MASK |    //Receiver enable
       //                     UART_C2_TE_MASK);     //Transmitter enable
        break;
    case RT_DEVICE_CTRL_RESUME:
        /* resume device */
      //  uart_reg->C2  =  UART_C2_RE_MASK |    //Receiver enable
       //                  UART_C2_TE_MASK;     //Transmitter enable
        break;
    }

    return RT_EOK;
}

static int _putc(struct rt_serial_device *serial, char c)
{
    UART_Type *uart_reg;
    uart_reg = ((struct ke02_serial_device *)serial->parent.user_data)->baseAddress;

    while (!(uart_reg->S1 & UART_S1_TDRE_MASK));
    uart_reg->D = (c & 0xFF);
    return 1;
}

static int _getc(struct rt_serial_device *serial)
{
    UART_Type *uart_reg;
    uart_reg = ((struct ke02_serial_device *)serial->parent.user_data)->baseAddress;

    if (uart_reg->S1 & UART_S1_RDRF_MASK)
        return (uart_reg->D);
    else
        return -1;
}

static const struct rt_uart_ops _ke02_ops =
{
    _configure,
    _control,
    _putc,
    _getc,
};


void UART0_IRQHandler(void)
{
    rt_interrupt_enter();
    rt_hw_serial_isr((struct rt_serial_device*)&_ke02_serial0, RT_SERIAL_EVENT_RX_IND);
    rt_interrupt_leave();
}

void UART1_IRQHandler(void)
{
    rt_interrupt_enter();
    rt_hw_serial_isr((struct rt_serial_device*)&_ke02_serial1, RT_SERIAL_EVENT_RX_IND);
    rt_interrupt_leave();
}

void rt_hw_uart_init(void)
{
    struct serial_configure config;

    /* fake configuration */
    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;
	config.bufsz	 = RT_SERIAL_RB_BUFSZ;

    _ke02_serial0.ops    = &_ke02_ops;
    _ke02_serial0.config = config;
    _ke02_serial1.ops    = &_ke02_ops;
    _ke02_serial1.config = config;
	//uart3 conflict with sdhc pin*/
    rt_hw_serial_register(&_ke02_serial0, "uart0",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          (void*)&_ke02_node_0);
    
    rt_hw_serial_register(&_ke02_serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          (void*)&_ke02_node_1);
}

void rt_hw_console_output(const char *str)
{
    while(*str != '\0')
    {
        if (*str == '\n')
            _putc(&_ke02_serial1,'\r');
        _putc(&_ke02_serial1,*str);
        str++;
    }
}
void
out_char (char ch)
{
	_putc(&_ke02_serial1, ch);
}

