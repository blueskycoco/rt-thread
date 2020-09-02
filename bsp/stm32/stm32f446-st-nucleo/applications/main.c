/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "ili9325.h"

/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(A, 5)
#define LED1_PIN    GET_PIN(A, 11)
#define LED3_PIN    GET_PIN(A, 12)
extern void ads7843_init();
int main(void)
{
	int count = 1;
	int xres = 240;
	int yres = 320;
	/* set LED2 pin mode to output */
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);
	ads7843_init();
	stm32_lcd_init();
	fb_clr(BLACK);
	put_cross(  50, 50);
	put_cross(  xres - 50, 50);
	put_cross(  xres - 50, yres - 50);
	put_cross(50,  yres - 50);
	put_cross(xres / 2, yres / 2);
	while (count++)
	{
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_pin_write(LED3_PIN, PIN_LOW);
		rt_thread_mdelay(500);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_pin_write(LED3_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
	}

	return RT_EOK;
}
