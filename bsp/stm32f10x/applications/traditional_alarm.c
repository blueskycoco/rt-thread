#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f10x.h"
void init_traditional(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}
uint8_t traditional_alarm1(uint8_t type)
{
	switch (type) {
		case 0:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7) == SET) 
				return 1;
			break;
		case 1:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8) == SET) 
				return 1;
			break;
		case 2:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9) == SET) 
				return 1;
			break;
		case 3:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_10) == SET) 
				return 1;
			break;
		default:
			break;
			}
		return 0;
}
uint8_t traditional_alarm(uint8_t type)
{
	static uint8_t alarm[4] = {0};
	uint8_t result = 0;

	switch (type) {
		case 0:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7) == SET) {
				if (alarm[0] == 0) {
					alarm[0] = 1;
					result = 1;
					rt_kprintf("gpioe7 int\r\n");
				}
			} else {
					rt_kprintf("gpioe7 clear\r\n");
				alarm[0] = 0;
			}
			break;
		case 1:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8) == SET) {
				if (alarm[1] == 0) {
					alarm[1] = 1;
					result = 1;
					rt_kprintf("gpioe8 int\r\n");
				}
			} else {
					rt_kprintf("gpioe8 clear\r\n");
				alarm[1] = 0;
			}
			break;
		case 2:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9) == SET) {
				if (alarm[2] == 0) {
					alarm[2] = 1;
					result = 1;
					rt_kprintf("gpioe9 int\r\n");
				}
			} else {
					rt_kprintf("gpioe9 clear\r\n");
				alarm[2] = 0;
			}
			break;
		case 3:
			if (GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_10) == SET) {
				if (alarm[3] == 0) {
					alarm[3] = 1;
					result = 1;
					rt_kprintf("gpioe10 int\r\n");
				}
			} else {
					rt_kprintf("gpioe10 clear\r\n");
				alarm[3] = 0;
			}
			break;
		default:
			break;
	}
	return result;
}
