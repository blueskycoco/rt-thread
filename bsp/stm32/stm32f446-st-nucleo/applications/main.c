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
#include <ctype.h>
#include "crc.h"
#include "mem_list.h"
static struct rt_thread uart_thread;
ALIGN(RT_ALIGN_SIZE)
static char uart_thread_stack[1024];
static struct rt_semaphore rx_ind;
rt_device_t ts_device;
rt_uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size;
rt_sem_t        isr_sem;
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
#if 0
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
#endif

static rt_err_t uart_data_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&rx_ind);
	return RT_EOK;
}

static rt_uint32_t read_ts()
{
	rt_hwtimerval_t val,val1;
	rt_device_read(ts_device, 0, &val, sizeof(val));
	return (val.sec*1000000+val.usec);
}

void dump_host_cmd(rt_uint8_t *cmd, rt_uint32_t len)
{
	rt_uint32_t i;

	rt_kprintf("\r\n=====================================>\r\nhost_cmd[%d]: ",
			len);
	for (i=0; i<len; i++) {
		rt_kprintf("%02x ", cmd[i]);
		if (i % 16 == 0 && i != 0)
			rt_kprintf("\r\n");
	}
}
static void uart_thread_entry(void *parameter)
{
	int len;
	rt_uint8_t uart_buf[64];

	rt_device_t device = (rt_device_t)parameter;

	rt_sem_init(&rx_ind, "uart_rx_ind", 1, RT_IPC_FLAG_FIFO);
	
	rt_device_set_rx_indicate(device, uart_data_ind);

	rt_kprintf("Ready.\n");

	while (1)
	{
		rt_sem_take(&rx_ind, RT_WAITING_FOREVER);
		/* handle host command */
		len = 0;
		do {
			len += rt_device_read(device, 0, uart_buf+len, 64-len);
			if (len > 0) {
				if (len == 64) {
					insert_mem(TYPE_H2D, uart_buf, len);
					break;
				}
			} else {
				break;
			}
		} while (1);
	}
}

static int timestamp_init(void)
{
	int err = 0;
	rt_hwtimer_mode_t mode;
	rt_hwtimerval_t val;
	
	if ((ts_device = rt_device_find("timer3")) == RT_NULL)
	{
		rt_kprintf("No Device: timer3\n");
		return -1;
	}

	if (rt_device_open(ts_device, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
	{
		rt_kprintf("Open timer3 Fail\n");
		return -1;
	}
	
	mode = HWTIMER_MODE_PERIOD;
	err = rt_device_control(ts_device, HWTIMER_CTRL_MODE_SET, &mode);
	val.sec = 10*60*60;
	val.usec = 0;
	rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
	if (rt_device_write(ts_device, 0, &val, sizeof(val)) != sizeof(val))
		rt_kprintf("set timer failed\n");
}

static int mcu_cmd_init(void)
{
	int err = 0;
	rt_device_t uart_device;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	uart_device = rt_device_find("uart6");

	RT_ASSERT(uart_device != RT_NULL);

	err = rt_device_open(uart_device, RT_DEVICE_FLAG_RDWR |
			RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX);

	if (err != RT_EOK)
	{
		rt_kprintf("open dev failed!\n");
		return -1;
	}

	config.baud_rate = BAUD_RATE_2000000;
	config.bufsz = 64;
	rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG, &config);

	rt_thread_init(&uart_thread,
			"uart_app",
			uart_thread_entry, uart_device,
			uart_thread_stack, sizeof(uart_thread_stack),
			10, 20);

	rt_thread_startup(&uart_thread);

	return 0;
}
int main(void)
{
	int count = 1;
	//	rt_thread_t tid = rt_thread_create("icm", icm_thread_entry, RT_NULL,
	//						2048, 28, 20);
	//  	rt_thread_startup(tid);
	/* set LED2 pin mode to output */
	rt_memlist_init();	
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	timestamp_init();
	mcu_cmd_init();
	while (count++)
	{
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_thread_mdelay(500);
	}

	return RT_EOK;
}
#ifdef FINSH_USING_MSH
int dump_len(void)
{
	rt_uint8_t *data;
	rt_uint32_t i;
	rt_uint16_t len, all_len;
	do {
		remove_mem(TYPE_H2D, &data, &len);
		if (data == RT_NULL)
			break;
		all_len += len;
		rt_kprintf("============>[%04d]: ", all_len);
		for (i=0; i<len; i++) {
			rt_kprintf("%c", data[i]);
		}
	} while (1);
	return 0;
}
MSH_CMD_EXPORT(dump_len, dump len);
#endif /* FINSH_USING_MSH */
