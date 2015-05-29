/*
 *	brife: this is at88sc driver code,gpio code 
 */
 #include "s1.h"
 #include <rthw.h>
#include <stm32f0xx.h>

 /*config scl,sda as gpio mode and dir out*/
 #define DELAY 10
 GPIO_InitTypeDef  GPIO_InitStructure;
 void i2c_init(void)
{
	 
	 /* Enable the GPIO_LED Clock */
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);
	 
	 /* Configure the GPIO_LED pin */
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	 GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	 GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*scl set hi or low*/
void i2c_scl_set(unsigned char level)
{
    volatile unsigned long delay = 0;
    if(level)
        GPIO_SetBits(GPIOA, GPIO_Pin_10);
    else
        GPIO_ResetBits(GPIOA, GPIO_Pin_10);
    for (delay = 0; delay < DELAY; )
    {
        delay++;
    }
}
/*sda set hi or low*/
void i2c_sda_set(unsigned char level)
{
    volatile unsigned long delay = 0;
    if(level)
        GPIO_SetBits(GPIOA, GPIO_Pin_9);
    else
        GPIO_ResetBits(GPIOA, GPIO_Pin_9);
    for (delay = 0; delay < DELAY; )
    {
        delay++;
    }
}
/*return sda level */
unsigned char i2c_sda_get(void)
{
		return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9);
}
/*config sda dir input*/
void i2c_sda_input(void)
{
    volatile unsigned long delay = 0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    for (delay = 0; delay < DELAY; )
    {
        delay++;
    }
}
/*config sda dir output*/
void i2c_sda_output(void)
{
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*sleep function*/
void sleep_ms(unsigned long n)
{
	volatile long i,j;
	for(i=0;i<n;i++)
	for(j=0;j<DELAY;j++)
	;
}
