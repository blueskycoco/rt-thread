#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "button.h"
/*
  * PD3 factory 
  * PD4 code
  */
  
void button_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 2; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(EXTI_Line9);
	EXTI_ClearITPendingBit(EXTI_Line0);
}
vu16 ADC_ConvertedValue;
rt_uint16_t get_bat(void)
{
	/*ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);  */
	return ADC_ConvertedValue;
}
void battery_init()
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)ADC1->DR;//ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	//配置ADC
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 2, ADC_SampleTime_71Cycles5);
	//ADC1选择信道8,音序器等级1,采样时间13.5个周期
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_71Cycles5);
	//ADC1选择信道9,音序器等级2,采样时间13.5个周期
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 3, ADC_SampleTime_239Cycles5); //温度采样时间较长
	//ADC1选择信道14,音序器等级2,采样时间13.5个周期
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	 
}
void bell_ctl(int level)
{
	if (level)	{	
		rt_kprintf("bell on\r\n");
		GPIO_SetBits(GPIOE, GPIO_Pin_11);
	} else {
		rt_kprintf("bell off\r\n");
		GPIO_ResetBits(GPIOE, GPIO_Pin_11);
	}
}
rt_uint8_t check_ac()
{
	return (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)?1:0);
}
void buzzer_ctl(int level)
{
	return ;
	if (level == BUZZER_OK)		
	{
		GPIO_SetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(40);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
	} else if (level == BUZZER_ERROR) {	
		GPIO_SetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(8);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(8);
		GPIO_SetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(8);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(8);
		GPIO_SetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(8);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
	} else {
		GPIO_SetBits(GPIOD, GPIO_Pin_6);
		rt_thread_delay(10);
		GPIO_ResetBits(GPIOD, GPIO_Pin_6);
	}
}
rt_uint8_t check_s1()
{
	static rt_uint8_t s1=0;
	rt_uint8_t s1_state = GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0);
	if (s1_state == 1 && s1==0) {
		s1 = 1;
		return 2;//fangchai
	} else if(s1_state == 0 && s1 == 1) {
		s1 = 0;
		return 1;//huifu
	}
	return 0;
}
void speaker_ctl(int flag)
{	
	if (flag)		
		GPIO_ResetBits(GPIOD, GPIO_Pin_8);
	else
		GPIO_SetBits(GPIOD, GPIO_Pin_8);
}
rt_uint8_t battery_insert()
{
	//GPIO_ResetBits(GPIOB, GPIO_Pin_7);
	return GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6);
}
