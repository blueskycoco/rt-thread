/*
 * File      : led.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-13-05     bright       the first version
 */

#ifndef __LED_H__
#define __LED_H__

#include <rthw.h>
#include <rtthread.h>
#include <stm32f0xx.h>

#define rt_hw_led1_on()   GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define rt_hw_led1_off()  GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define rt_hw_led2_on()   GPIO_ResetBits(GPIOB, GPIO_Pin_1)
#define rt_hw_led2_off()  GPIO_SetBits(GPIOB, GPIO_Pin_1)

int rt_hw_led_init(void);

#endif
