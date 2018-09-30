/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <board.h>
#include <rtthread.h>
#include "master.h"
#include "prop.h"
/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
extern	struct rt_event g_info_event;
extern rt_uint8_t 	cur_status;
extern rt_uint16_t g_sub_event_code;
extern rt_uint8_t g_delay_out;
extern rt_uint8_t g_alarm_voice;
extern rt_uint8_t g_alarmType;
extern rt_uint8_t g_delay_in;
extern rt_uint8_t s1;
extern rt_uint8_t time_protect;
rt_uint8_t duima_key=0;
extern rt_uint8_t g_remote_protect;
extern rt_uint8_t 	g_mute;
rt_uint8_t s_bufang=0;
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

//void SysTick_Handler(void)
//{
//    // definition in boarc.c
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

#ifdef  RT_USING_LWIP
/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
    extern void rt_dm9000_isr(void);

    /* enter interrupt */
    rt_interrupt_enter();

    /* Clear the DM9000A EXTI line pending bit */
    EXTI_ClearITPendingBit(EXTI_Line4);

    rt_dm9000_isr();

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_LWIP */

#ifdef STM32F103ZET6
void EXTI9_5_IRQHandler(void)
{
	extern void cc1101_isr(void);
	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line5))
	{	 
		cc1101_isr();	
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}
#else
void ldelay()
{
	volatile long i,j;
	for(i=0;i<100000;i++)
		j=i;
}
void EXTI9_5_IRQHandler(void)
{
	rt_uint32_t i=0;
	rt_uint8_t level = 0;
	static rt_uint32_t s_cnt = 0;
	static rt_uint32_t e_cnt = 0;
	static rt_uint8_t short_press=0;
	rt_uint32_t diff = 0;
	static rt_uint8_t flag = 0;
	//extern void cc1101_isr(void);
	extern rt_uint8_t g_main_state;
	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line9))
	{	 
		//rt_kprintf("code button int\r\n");
		#if 0
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == RESET)
		{
			i++;
			ldelay();
		}
		if (i>100)
		{			
			rt_event_send(&(g_info_event), INFO_EVENT_FACTORY_RESET);
		}
		else if(i>10)
		{	
			if (g_main_state==0)
				rt_event_send(&(g_info_event), INFO_EVENT_CODING);
			else					
				rt_event_send(&(g_info_event), INFO_EVENT_NORMAL);
		}
		/*else if(i>5)
		{		
			rt_event_send(&(g_info_event), INFO_EVENT_NORMAL);
		}*/
		rt_kprintf("i is %d, state %d\r\n", i,g_main_state);
		#else
		/*if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == RESET) {
			while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == RESET)
			{
				i++;
				ldelay();
				if (i>5) {
					break;
				}
			}			
		} else {
			while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == SET)
			{
				i++;
				ldelay();
				if (i>5) {
					break;
				}
			}
		}*/
		if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) == RESET) {
			s_cnt = rt_tick_get();
			flag=1;
			rt_kprintf("S button press %d\r\n",s_cnt - e_cnt);
			if (s_cnt - e_cnt < 40)
			{
				short_press++;
				if (short_press >= 2) {
					short_press=0;
					if (cur_status) {
						if((cur_status || (!cur_status && g_delay_out!=0) || g_alarm_voice))
						{
							cur_status = 0;
							g_alarmType =0;
							g_delay_out = 0;
							g_alarm_voice = 0;
							g_delay_in = 0;
							fqp.status=cur_status;
							s1=0;
							time_protect = 1;
							duima_key=1;
							g_mute=0;
							g_remote_protect=0;
							rt_kprintf("switch protect off\r\n");
							handle_protect_off();
						}
					} else {
						if (!cur_status && g_delay_out==0)
						{
							rt_kprintf("switch protect on\r\n");
							g_remote_protect=0;
							g_sub_event_code = 0x2002;
							time_protect = 1;
							duima_key=1;
							//s_bufang=1;
							handle_protect_on();
						}
					}
				}
			}
			else
			{
				short_press=0;
#if 0
				rt_kprintf("GGGGG %d %d %d\r\n",g_mute,cur_status,s_bufang);
				if (s_bufang) {
					g_mute=1;
					s_bufang=0;
					rt_event_send(&(g_info_event), INFO_EVENT_MUTE);			
					rt_kprintf("got key mute\r\n");
				}
#endif
			}
		} else {
			if (flag) {
				flag=0;
				e_cnt =rt_tick_get(); 
				diff = e_cnt - s_cnt;
				rt_kprintf("S button relese %d\r\n", diff);
				if (diff > 1000) {
					short_press=0;
					rt_kprintf("factory reset");
					rt_event_send(&(g_info_event), INFO_EVENT_FACTORY_RESET);
				} else if (diff > 50) {
					short_press=0;
					rt_kprintf("coding or not");
					if (g_main_state==0)
					rt_event_send(&(g_info_event), INFO_EVENT_CODING);
					else					
					rt_event_send(&(g_info_event), INFO_EVENT_NORMAL);
				}
			}
		}
		#endif
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

void EXTI1_IRQHandler(void)
{
    extern void cc1101_isr(void);
	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line1))
	{	 
		//	rt_kprintf("cc1101 int\r\n");
		//if (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1) == RESET)
			cc1101_isr();	
		//else
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}
#endif
void EXTI0_IRQHandler(void)
{
    extern void factory_isr(void);
	/* enter interrupt */
	rt_interrupt_enter();
	if(EXTI_GetITStatus(EXTI_Line0))
	{	
		//rt_kprintf("factory button int\r\n");
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
	/* leave interrupt */
	rt_interrupt_leave();
}

#ifndef STM32F10X_CL
/* CAN and USB IRQ for stm32 none connectivity line devices
 */
void USB_LP_CAN1_RX0_IRQHandler1(void)
{
#ifdef RT_USING_CAN
    CAN1_RX0_IRQHandler();
#endif
}
void USB_HP_CAN1_TX_IRQHandler1(void)
{
#ifdef RT_USING_CAN
    CAN1_TX_IRQHandler();
#endif
}
#endif


/**
  * @}
  */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
