/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"

#include "drv_uart.h"


/**
 * @addtogroup K64
 */

/*@{*/

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{

}

/*******************************************************************************
 * Function Name  : SysTick_Configuration
 * Description    : Configures the SysTick for OS tick.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void  SysTick_Configuration(void)
{
	/*while(!(ICS->S & ICS_S_LOCK_MASK));
	SIM->SOPT = SIM_SOPT_SWDE_MASK | SIM_SOPT_RSTPE_MASK;
	SIM->PINSEL |= SIM_PINSEL_SPI0PS_MASK;
	SIM_SOPT |= SIM_SOPT_CLKOE_MASK;
	FEI_to_FEE();
	SIM_SCGC |= SIM_SCGC_UART0_MASK | SIM_SCGC_UART1_MASK | SIM_SCGC_UART2_MASK;*/
	
    //SystemCoreClockUpdate();              /* Update Core Clock Frequency        */
    SysTick_Config(20000000L/RT_TICK_PER_SECOND); /* Generate interrupt each 1 ms       */
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}
/**
 * This function will initial Tower board.
 */
void rt_hw_board_init()
{
    /* NVIC Configuration */
    NVIC_Configuration();
    /* Configure the SysTick */
    SysTick_Configuration();
    rt_hw_uart_init();
	
#ifdef RT_USING_CONSOLE
    rt_console_set_device(CONSOLE_DEVICE);
#endif
}

/*@}*/
