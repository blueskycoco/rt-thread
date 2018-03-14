/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>
#include "led.h"
// led define
#ifdef STM32_SIMULATOR
#define led1_rcc                    RCC_APB2Periph_GPIOA
#define led1_gpio                   GPIOA
#define led1_pin                    (GPIO_Pin_5)

#define led2_rcc                    RCC_APB2Periph_GPIOA
#define led2_gpio                   GPIOA
#define led2_pin                    (GPIO_Pin_6)

#else
#ifndef STM32F103ZET6
#define led1_rcc                    RCC_APB2Periph_GPIOD
#define led1_gpio                   GPIOD
#define led1_pin                    (GPIO_Pin_5)

#define led2_rcc                    RCC_APB2Periph_GPIOC
#define led2_gpio                   GPIOC
#define led2_pin                    (GPIO_Pin_4)

#define ARM_LED_rcc                 RCC_APB2Periph_GPIOE
#define ARM_LED_gpio                GPIOE
#define ARM_LED_pin                 (GPIO_Pin_1)

#define WIRE_LED_rcc                RCC_APB2Periph_GPIOE
#define WIRE_LED_gpio               GPIOE
#define WIRE_LED_pin                (GPIO_Pin_3)

#define ALARM_LED_rcc               RCC_APB2Periph_GPIOE
#define ALARM_LED_gpio              GPIOE
#define ALARM_LED_pin               (GPIO_Pin_2)

#define WIRELESS_LED_rcc            RCC_APB2Periph_GPIOE
#define WIRELESS_LED_gpio           GPIOE
#define WIRELESS_LED_pin            (GPIO_Pin_4)

#define NET_LED_rcc                 RCC_APB2Periph_GPIOE
#define NET_LED_gpio                GPIOE
#define NET_LED_pin                 (GPIO_Pin_5)

#define FAIL_LED_rcc                RCC_APB2Periph_GPIOE
#define FAIL_LED_gpio               GPIOE
#define FAIL_LED_pin                (GPIO_Pin_6)

#define AUX_LED_rcc                RCC_APB2Periph_GPIOA
#define AUX_LED_gpio               GPIOA
#define AUX_LED_pin1                (GPIO_Pin_7)
#define AUX_LED_pin2                (GPIO_Pin_6)

#else
#define led1_rcc                    RCC_APB2Periph_GPIOD
#define led1_gpio                   GPIOD
#define led1_pin                    (GPIO_Pin_5)

#endif
#endif // led define #ifdef STM32_SIMULATOR

void rt_hw_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(led1_rcc|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = led1_pin;
    GPIO_Init(led1_gpio, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = ARM_LED_pin;
    GPIO_Init(ARM_LED_gpio, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = WIRE_LED_pin;
    GPIO_Init(WIRE_LED_gpio, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = ALARM_LED_pin;
    GPIO_Init(ALARM_LED_gpio, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = WIRELESS_LED_pin;
    GPIO_Init(WIRELESS_LED_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = NET_LED_pin;
    GPIO_Init(NET_LED_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = FAIL_LED_pin;
    GPIO_Init(FAIL_LED_gpio, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = AUX_LED_pin1;
    GPIO_Init(AUX_LED_gpio, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = AUX_LED_pin2;
    GPIO_Init(AUX_LED_gpio, &GPIO_InitStructure);
}

void rt_hw_led_on(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        GPIO_ResetBits(led1_gpio, led1_pin);
        break;
		
    case ARM_LED:
        GPIO_SetBits(ARM_LED_gpio, ARM_LED_pin);
        break;
		
	case WIRE_LED:
		GPIO_SetBits(WIRE_LED_gpio, WIRE_LED_pin);
		break;
	
	case ALARM_LED:
		GPIO_SetBits(ALARM_LED_gpio, ALARM_LED_pin);
		break;
	
	case WIRELESS_LED:
		GPIO_SetBits(WIRELESS_LED_gpio, WIRELESS_LED_pin);
		break;
	
	case NET_LED:
		GPIO_SetBits(NET_LED_gpio, NET_LED_pin);
		break;
		
	case FAIL_LED:
		GPIO_SetBits(FAIL_LED_gpio, FAIL_LED_pin);
		break;
	case AUX_LED0:
		GPIO_SetBits(AUX_LED_gpio, AUX_LED_pin1);
		GPIO_ResetBits(AUX_LED_gpio, AUX_LED_pin2);
		break;	
	case AUX_LED1:
		GPIO_SetBits(AUX_LED_gpio, AUX_LED_pin2);
		GPIO_ResetBits(AUX_LED_gpio, AUX_LED_pin1);
		break;
	case AUX_LED2:
		GPIO_SetBits(AUX_LED_gpio, AUX_LED_pin1);
		GPIO_SetBits(AUX_LED_gpio, AUX_LED_pin2);
		break;
    default:
        break;
    }
}

void rt_hw_led_off(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        GPIO_SetBits(led1_gpio, led1_pin);
        break;
		
	case ARM_LED:
		GPIO_ResetBits(ARM_LED_gpio, ARM_LED_pin);
		break;
		
	case WIRE_LED:
		GPIO_ResetBits(WIRE_LED_gpio, WIRE_LED_pin);
		break;
	
	case ALARM_LED:
		GPIO_ResetBits(ALARM_LED_gpio, ALARM_LED_pin);
		break;
	
	case WIRELESS_LED:
		GPIO_ResetBits(WIRELESS_LED_gpio, WIRELESS_LED_pin);
		break;
	
	case NET_LED:
		GPIO_ResetBits(NET_LED_gpio, NET_LED_pin);
		break;
		
	case FAIL_LED:
		GPIO_ResetBits(FAIL_LED_gpio, FAIL_LED_pin);
		break;
    default:
        break;
    }
}

