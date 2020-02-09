/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-08     obito0   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

/* defined the LED0 pin: PC13 */
#define LED0_PIN    GET_PIN(B, 14)
#define LED1_PIN    GET_PIN(B, 15)
static rt_device_t dev;
static struct rt_semaphore sem;

static rt_err_t air_input(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&sem);
	return RT_EOK;
}

static void air_thread(void *parm)
{
	char ch;
	char buf[32] = {0};
	int len = 0;
	int i = 0;
	rt_uint8_t cmd[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
	rt_kprintf("startup air thread\n");
	rt_device_write(dev, 0, cmd, sizeof(cmd));
	while (1)
	{
		while((len = rt_device_read(dev, -1, &ch, 1)) != 1) {
			rt_sem_take(&sem, RT_WAITING_FOREVER);
		}
		if (len == 1) {
			buf[i++] = ch;
			if (ch == 0xff)
				rt_kprintf("%02x\r\n", ch);
			else
				rt_kprintf("%02x ", ch);
		}
	}
}
int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	dev = rt_device_find("uart2");
	if (!dev){
		rt_kprintf("can not find uart2\n");
	} else {
		rt_kprintf("find uart3\n");
		rt_sem_init(&sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
		rt_device_open(dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
		rt_device_set_rx_indicate(dev, air_input);
		rt_thread_t thr = rt_thread_create("air", air_thread, RT_NULL, 1024, 25,
				10);
		if (thr != RT_NULL)
			rt_thread_startup(thr);
		else
			rt_kprintf("create air thread failed\r\n");
	}
	st7585_init();	
    while (count)
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
