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
 *
 */

#ifndef __LED_H__
#define __LED_H__

#include <rtthread.h>


void rt_hw_led_init(void);
void rt_hw_led_uninit(void);
void rt_hw_led_on(void);
void rt_hw_led_off(void);
#endif /* end  of __LED_H__ */
