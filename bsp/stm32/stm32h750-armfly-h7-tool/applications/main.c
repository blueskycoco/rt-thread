/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-25     zylx         first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <finsh.h>
/* defined the LED0 pin: PI8 */
#define LED0_PIN    GET_PIN(I, 8)
#define LED1_PIN    GET_PIN(C, 15)
int vcom_init(void)
{
	/* set console */
	rt_console_set_device("vcom");

#if defined(RT_USING_POSIX)    
	/* backup flag */
	dev_old_flag = ioctl(libc_stdio_get_console(), F_GETFL, (void *) RT_NULL);
	/* add non-block flag */
	ioctl(libc_stdio_get_console(), F_SETFL, (void *) (dev_old_flag | O_NONBLOCK));
	/* set tcp shell device for console */
	libc_stdio_set_console("vcom", O_RDWR);

	/* resume finsh thread, make sure it will unblock from
	 * last device receive */
	rt_thread_t tid = rt_thread_find(FINSH_THREAD_NAME);
	if (tid)
	{
		rt_thread_resume(tid);
		rt_schedule();
	}
#else
	/* set finsh device */
	finsh_set_device("vcom");
#endif /* RT_USING_POSIX */

	return 0;
}
int main(void)
{
	int count = 1;
	/* set LED0 pin mode to output */
	rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	vcom_init();
	while (count++)
	{
		rt_pin_write(LED0_PIN, PIN_HIGH);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_thread_mdelay(500);
		rt_pin_write(LED0_PIN, PIN_LOW);
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
	}
	return RT_EOK;
}
