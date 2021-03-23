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
#include "mem_list.h"
#include <fal.h>
#include "param.h"
#include "utils.h"
#include "mcu.h"
#define DRV_DEBUG
#define LOG_TAG             "main.mcu"
#include <drv_log.h>
static struct rt_thread usb_thread;
rt_device_t hid_device;
ALIGN(RT_ALIGN_SIZE)
	static char usb_thread_stack[1024];
	//static struct rt_semaphore tx_sem_complete;
	rt_uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size = 0;
rt_uint32_t g_vsync_count = 0;
rt_uint32_t g_vsync_t3 = 0;
rt_uint32_t g_heart_t2 = 0;
//static struct rt_event light_event;
#define EVENT_VSYNC 0x01
#define EVENT_HEART 0x02
static void handle_heart(rt_device_t device, rt_uint8_t *data, rt_size_t size);
#define VSYNC_INT_PIN GET_PIN(D, 2)
#define SCL_PIN GET_PIN(C, 4)
#define SDA_PIN GET_PIN(C, 5)
#define LED1_PIN GET_PIN(B, 14)
#if 0
static void vsync_isr(void *parameter)
{
	if (g_hid_size != 0) {
		g_vsync_t3 = read_ts();
		g_vsync_count++;
		rt_event_send(&light_event, EVENT_VSYNC);
	}
}
#endif
static rt_err_t event_hid_in(rt_device_t dev, void *buffer)
{
	notify_event(EVENT_ST2OV);
	//rt_sem_release(&tx_sem_complete);
	return RT_EOK;
}
#if 0
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
		uint32_t crc = crc32(tx, 24);
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
		uint32_t crc = crc32(tx, 41);
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
#endif
void hid_out(uint8_t *data, uint16_t len)
{
	int i;
	rt_uint8_t *ptr = data;

	rt_kprintf("\r\n");
	for (i = 0; i < len; i++) {
		if (i%16 == 0 && i!=0)
			rt_kprintf("\r\n");
		rt_kprintf("%02x ", *ptr++);
	}
	rt_kprintf("\r\n");
	if (rt_device_write(hid_device, 0x01, data+1, (len-1)) != (len-1))
		rt_kprintf("hid out failed\r\n");
}
static void dump_data(rt_uint8_t *data, rt_size_t size)
{
	rt_size_t i;
	rt_uint8_t *ptr = data;
	
	rt_kprintf("\r\n");
	for (i = 0; i < size; i++) {
		if (i%16 == 0 && i!=0)
			rt_kprintf("\r\n");
		rt_kprintf("%02x ", *ptr++);
	}
	rt_kprintf("\r\n");

	if (!insert_mem(TYPE_H2D, data, 64))
		LOG_W("lost h2d packet\r\n");
	notify_event(EVENT_OV2ST);
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
	hid_device = rt_device_find("hidd");

	RT_ASSERT(hid_device != RT_NULL);

	err = rt_device_open(hid_device, RT_DEVICE_FLAG_RDWR);

	if (err != RT_EOK)
	{
		rt_kprintf("open dev failed!\n");
		return -1;
	}

	//rt_event_init(&light_event, "event", RT_IPC_FLAG_FIFO);
	//rt_sem_init(&tx_sem_complete, "tx_complete_sem_hid", 1, RT_IPC_FLAG_FIFO);

	rt_device_set_tx_complete(hid_device, event_hid_in);
#if 0
	rt_pin_mode(VSYNC_INT_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_attach_irq(VSYNC_INT_PIN, PIN_IRQ_MODE_RISING, vsync_isr, RT_NULL);
	rt_pin_irq_enable(VSYNC_INT_PIN, RT_TRUE);
	rt_thread_init(&usb_thread,
			"hidd_app",
			usb_thread_entry, hid_device,
			usb_thread_stack, sizeof(usb_thread_stack),
			10, 20);

	rt_thread_startup(&usb_thread);
#endif
	return 0;
}
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
   
    /* resume finsh thread, make sure it will unblock from last device receive */
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
//INIT_APP_EXPORT(vcom_init);

extern int fal_init(void);
INIT_COMPONENT_EXPORT(fal_init);
int main(void)
{
	int count = 1;

	if (!param_init())
		LOG_D("can't startup system\r\n");
	rt_kprintf("SCL %d, SDA %d, %d %d\r\n", SCL_PIN, SDA_PIN,
			GET_PIN(B, 0), GET_PIN(B, 7));
	rt_memlist_init();	
	timestamp_init();
	init_oled();
	protocol_init();
	generic_hid_init();
	//vcom_init();
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	//rt_pin_mode(87, PIN_MODE_OUTPUT);
	//rt_pin_mode(88, PIN_MODE_OUTPUT);
#if 1
	while (count++)
	{
		//rt_pin_write(87, PIN_HIGH);
		//rt_pin_write(88, PIN_HIGH);
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		//rt_pin_write(87, PIN_LOW);
		//rt_pin_write(88, PIN_LOW);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_thread_mdelay(500);
	}
#endif
	return RT_EOK;
}
