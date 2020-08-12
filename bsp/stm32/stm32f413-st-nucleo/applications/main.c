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
#include "crc.h"
#include "oled.h"
static struct rt_thread usb_thread;
ALIGN(RT_ALIGN_SIZE)
	static char usb_thread_stack[1024];
	static struct rt_semaphore tx_sem_complete;
	rt_device_t ts_device;
	rt_uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size = 0;
rt_uint32_t g_vsync_count = 0;
rt_uint32_t g_vsync_t3 = 0;
rt_uint32_t g_heart_t2 = 0;
static struct rt_event light_event;
#define EVENT_VSYNC 0x01
#define EVENT_HEART 0x02
static void handle_heart(rt_device_t device, rt_uint8_t *data, rt_size_t size);
#define VSYNC_INT_PIN GET_PIN(D, 2)

static rt_uint32_t read_ts()
{
	rt_hwtimerval_t val,val1;
	rt_device_read(ts_device, 0, &val, sizeof(val));
	return (val.sec*1000000+val.usec);
}
static void vsync_isr(void *parameter)
{
	if (g_hid_size != 0) {
		g_vsync_t3 = read_ts();
		g_vsync_count++;
		rt_event_send(&light_event, EVENT_VSYNC);
	}
}
static rt_err_t event_hid_in(rt_device_t dev, void *buffer)
{
	rt_sem_release(&tx_sem_complete);
	return RT_EOK;
}

static void handle_heart(rt_device_t device, rt_uint8_t *data, rt_size_t size)
{
	rt_uint8_t tx[64] = {0};
	rt_uint32_t t2 = 0, t3 = 0;
	rt_uint8_t ts2_ascii[9], ts3_heart_ascii[9], ts3_vsync_ascii[9], crc_ascii[9], count_ascii[9];
	if (data == RT_NULL && size == 0) {
		/* vsync */
		tx[0] = 0x02;
		tx[1] = 0x3a;
		tx[2] = '5';
		tx[3] = 0x3a;
		tx[4] = 'S';
		tx[5] = 0x3a;
		rt_sprintf(count_ascii, "%08x", g_vsync_count);
		rt_memcpy(tx+6, count_ascii, 8);
		tx[14] = 0x3a;
		rt_sprintf(ts3_vsync_ascii, "%08x", g_vsync_t3);
		rt_memcpy(tx+15, ts3_vsync_ascii, 8);
		tx[23] = 0x3a;
		uint32_t crc = heart_cmd_crc(tx, 24);
		rt_sprintf(crc_ascii, "%8x", crc);
		rt_memcpy(tx+24, crc_ascii, 8);
		tx[32] = 0x3a;
		tx[33] = 0x03;
		if (rt_device_write(device, 0x02, tx+1, 33) == 33)
		{
			//rt_thread_delay(rt_tick_from_millisecond(50)/1000);
			rt_kprintf("vsync out ok\n");
		}
		else
			rt_kprintf("vsync out failed\n");
	} else if (data[0] == ':' &&
			data[1] == '@' &&
			data[2] == ':' &&
			data[3] == 'K') {
		/* heart T1, T2, T3, T4 */
		tx[0] = 0x02;
		tx[1] = 0x3a;
		tx[2] = 0x41;
		tx[3] = 0x3a;
		tx[4] = 0x4b;
		tx[5] = 0x3a;
		rt_memcpy(tx+6, data+5, 16);
		tx[22] = 0x26;
		rt_sprintf(ts2_ascii, "%08x", g_heart_t2);
		rt_memcpy(tx+23, ts2_ascii, 8);
		tx[31] = 0x3a;
		t3 = read_ts();
		rt_sprintf(ts3_heart_ascii, "%08x", t3);
		rt_memcpy(tx+32, ts3_heart_ascii, 8);
		tx[40] = 0x3a;
		uint32_t crc = heart_cmd_crc(tx, 41);
		rt_sprintf(crc_ascii, "%8x", crc);
		rt_memcpy(tx+41, crc_ascii, 8);
		tx[49] = 0x3a;
		tx[50] = 0x03;
		if (rt_device_write(device, 0x02, tx+1, 50) == 50)
		{
			//rt_thread_delay(rt_tick_from_millisecond(50)/1000);
			rt_kprintf("heart out ok\n");
		}
		else
			rt_kprintf("heart out failed\n");

	}
}
static void usb_thread_entry(void *parameter)
{
	rt_device_t device = (rt_device_t)parameter;
	rt_uint32_t e;

	while (1)
	{
		if (rt_event_recv(&light_event, EVENT_VSYNC | EVENT_HEART,
					RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
					RT_WAITING_FOREVER, &e) != RT_EOK)
		{
			continue;
		}

		if (e & EVENT_VSYNC)
			handle_heart(device, RT_NULL, 0);
		if (e & EVENT_HEART)
			handle_heart(device, hid_rcv, g_hid_size);


	}
}
static void dump_data(rt_uint8_t *data, rt_size_t size)
{
	rt_size_t i;
	rt_uint8_t *ptr = data;
	/*for (i = 0; i < size; i++)
	  {
	  rt_kprintf("%c", *ptr++);
	  }
	  rt_kprintf("\n");*/

	if (data[0] == ':' &&
			data[1] == '@' &&
			data[2] == ':' &&
			data[3] == 'K') {
		rt_memset(hid_rcv, 0, 64);
		rt_memcpy(hid_rcv, data, size);
		g_hid_size = size;
		rt_event_send(&light_event, EVENT_HEART);
	}
}
static void dump_report(struct hid_report * report)
{
	dump_data(report->report,report->size);
}
void HID_Report_Received(hid_report_t report)
{
	g_heart_t2 = read_ts();
	dump_report(report);
}
static int generic_hid_init(void)
{
	int err = 0;
	rt_device_t hid_device;
	rt_hwtimer_mode_t mode;
	rt_hwtimerval_t val;
	hid_device = rt_device_find("hidd");

	RT_ASSERT(hid_device != RT_NULL);

	err = rt_device_open(hid_device, RT_DEVICE_FLAG_RDWR);

	if (err != RT_EOK)
	{
		rt_kprintf("open dev failed!\n");
		return -1;
	}

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
	rt_device_control(ts_device, HWTIMER_CTRL_MODE_SET, &mode);
	
	val.sec = 5*60*60;
	val.usec = 0;
	rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
	if (rt_device_write(ts_device, 0, &val, sizeof(val)) != sizeof(val))
		rt_kprintf("set timer failed\n");

	rt_event_init(&light_event, "event", RT_IPC_FLAG_FIFO);
	rt_sem_init(&tx_sem_complete, "tx_complete_sem_hid", 1, RT_IPC_FLAG_FIFO);

	rt_pin_mode(VSYNC_INT_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_attach_irq(VSYNC_INT_PIN, PIN_IRQ_MODE_RISING, vsync_isr, RT_NULL);
	rt_pin_irq_enable(VSYNC_INT_PIN, RT_TRUE);
	rt_device_set_tx_complete(hid_device, event_hid_in);

	rt_thread_init(&usb_thread,
			"hidd_app",
			usb_thread_entry, hid_device,
			usb_thread_stack, sizeof(usb_thread_stack),
			10, 20);

	rt_thread_startup(&usb_thread);

	return 0;
}
int main(void)
{
	int count = 1;
	/* set LED1 pin mode to output */
	init_oled();
	generic_hid_init();
#if 0
	while (count++)
	{
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED1_PIN, PIN_LOW);
	}
#endif
	return RT_EOK;
}
