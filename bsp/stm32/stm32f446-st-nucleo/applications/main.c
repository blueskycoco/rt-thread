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
#include "tslib.h"

/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(A, 5)
#define LED1_PIN    GET_PIN(A, 11)
#define LED3_PIN    GET_PIN(A, 12)
extern void ads7843_init();
extern uint8_t cal_finished;
struct tsdev *ts;
extern calibration cal;
extern int wifi_init();
int main(void)
{
	int count = 1;
	int xres = 240;
	int yres = 320;
	/* set LED2 pin mode to output */
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);
	stm32_lcd_init();
	fb_clr(BLACK);
	ads7843_init();
	wifi_init();
	//rt_thread_delay(200);
	//ts_calibrate();
	cal_finished = 1;
	cal.a[0] = -1;
	cal.a[1] = 538;
	cal.a[2] = -1026664;
	cal.a[3] = 716;
	cal.a[4] = -6;
	cal.a[5] = -1157373;
	cal.a[6] = 65536;
	ts = ts_open_module();
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
