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

rt_sem_t        isr_sem;
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
#define ICM_INT_PIN GET_PIN(B, 4)
static void icm_isr(void *parameter)
{
    rt_sem_release(isr_sem);
}

static void icm_thread_entry(void *parameter)
{
	rt_int16_t x, y, z;
    	
    	isr_sem = rt_sem_create("icm", 0, RT_IPC_FLAG_FIFO);
    	rt_pin_mode(ICM_INT_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_attach_irq(ICM_INT_PIN, PIN_IRQ_MODE_FALLING, icm_isr, RT_NULL);
	
	icm20603_device_t icm_dev = icm20603_init();
	icm20603_calib_level(icm_dev, 10);
    	rt_pin_irq_enable(ICM_INT_PIN, RT_TRUE);

	while (1)
	{

		if (rt_sem_take(isr_sem, -1) != RT_EOK)
		{
			continue;
		}
		icm20603_get_accel(icm_dev, &x, &y, &z);
		rt_kprintf("accelerometer: X%6d    Y%6d    Z%6d\n", x, y, z);
		icm20603_get_gyro(icm_dev, &x, &y, &z);
		rt_kprintf("gyroscope    : X%6d    Y%6d    Z%6d\n", x, y, z);
	}
}
int main(void)
{
	int count = 1;
	rt_thread_t tid = rt_thread_create("icm", icm_thread_entry, RT_NULL,
						2048, 28, 20);
    	rt_thread_startup(tid);
	/* set LED2 pin mode to output */
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	while (count++)
	{
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_thread_mdelay(500);

	}

	return RT_EOK;
}
