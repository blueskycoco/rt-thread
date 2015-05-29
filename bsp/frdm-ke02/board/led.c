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
 *
 */

#include "common.h"
#include "sysinit.h"
#include "ke02_config.h"
#include "led.h"



void rt_hw_led_init(void)
{
	RED_Init();
	GREEN_Init();
	BLUE_Init();

    rt_hw_led_off();
}

void rt_hw_led_uninit(void)
{
}

void rt_hw_led_on()
{
    	BLUE_Toggle();	
}

void rt_hw_led_off()
{
    	BLUE_Toggle();
}
void rt_hw_led_on2()
{
    	RED_Toggle();	
}

void rt_hw_led_off2()
{
    	RED_Toggle();
}

