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
#include "icm20603.h"

/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)

int main(void)
{
	int count = 1;
	rt_int16_t x, y, z;
	/* set LED2 pin mode to output */
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	icm20603_device_t icm_dev = icm20603_init();
	icm20603_calib_level(icm_dev, 10);
	while (count++)
	{
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_thread_mdelay(500);

		icm20603_get_accel(icm_dev, &x, &y, &z);
		rt_kprintf("accelerometer: X%6d    Y%6d    Z%6d\n", x, y, z);
		icm20603_get_gyro(icm_dev, &x, &y, &z);
		rt_kprintf("gyroscope    : X%6d    Y%6d    Z%6d\n", x, y, z);
	}

	return RT_EOK;
}
