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
#include "adc.h"

static void RCC_Configuration(void)
{
	  /* Enable GPIO clock */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);
	  /* Enable USART clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

static void GPIO_Configuration(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;

	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_Init(GPIOB, &GPIO_InitStructure);
  	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);
}
#if 1

void key_config()
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
	  EXTI_InitTypeDef EXTI_InitStructure;


	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);    /* Configure the SPI interrupt priority */
	  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
	  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	  EXTI_Init(&EXTI_InitStructure);    /* Clear DM9000A EXTI line pending bit */
	  EXTI_ClearITPendingBit(EXTI_Line0);

}
#endif
int rt_hw_adc_init(void)
{
	  ADC_InitTypeDef     ADC_InitStructure;
	 key_config();
	  RCC_Configuration();
	  GPIO_Configuration();
	  /* ADCs DeInit */  
	  ADC_DeInit(ADC1);

	  /* Initialize ADC structure */
	  ADC_StructInit(&ADC_InitStructure);

	  /* Configure the ADC1 in continous mode withe a resolutuion equal to
	   * 12 bits  */
	  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 
	  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	  ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	  ADC_Init(ADC1, &ADC_InitStructure); 
	  //ADC_DiscModeCmd(ADC1,ENABLE);
	  /* Convert the ADC1 Channel 11 with 239.5 Cycles as
	   * sampling time */ 
	  ADC_ChannelConfig(ADC1, ADC_Channel_9|ADC_Channel_5, ADC_SampleTime_239_5Cycles);   
	  //ADC_GetCalibrationFactor(ADC1);
	  //ADC_ChannelConfig(ADC1, ADC_Channel_5 , ADC_SampleTime_239_5Cycles);   
	  /* ADC Calibration */
	  ADC_GetCalibrationFactor(ADC1);

	  /* Enable ADCperipheral[PerIdx] */
	  ADC_Cmd(ADC1, ENABLE);     

	  /* Wait the ADRDY falg */
	  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)); 
}
/*channel 5 for shidu , channel 9 for battery vol */
uint16_t read_adc(unsigned char channel)
{
	int i;
	  __IO uint16_t  ADC1ConvertedValue1 = 0, ADC1ConvertedValue2=0,ADC1ConvertedVoltage = 0;
	  
	for(i=0;i<10;i++)
	{   
	ADC_StartOfConversion(ADC1);
	  /* Test EOC flag */
	  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	
	  /* Get ADC1 converted data */
	  ADC1ConvertedValue1 +=ADC_GetConversionValue(ADC1);
	  if(channel==9)
	  {
	  	  //ADC_StartOfConversion(ADC1);
		  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	
		  ADC1ConvertedValue2 +=ADC_GetConversionValue(ADC1);		  
	  }	  
	  ADC_StopOfConversion(ADC1);
	  rt_thread_delay(10);
	}
	  /* Compute the voltage */
	  if(channel==9)
	  ADC1ConvertedVoltage = ((ADC1ConvertedValue2 *3300)/0xFFF)/10;
	  else
	  ADC1ConvertedVoltage = ((ADC1ConvertedValue1 *3300)/0xFFF)/10;	
	  return ADC1ConvertedVoltage;
}
//INIT_DEVICE_EXPORT(rt_hw_adc_init);

