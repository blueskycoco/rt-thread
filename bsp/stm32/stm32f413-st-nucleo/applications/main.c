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
static struct rt_thread usb_thread;
ALIGN(RT_ALIGN_SIZE)
static char usb_thread_stack[1024];
static struct rt_semaphore tx_sem_complete;
rt_device_t ts_device;
rt_uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size;

/* defined the LED1 pin: PB0 */
#define LED1_PIN    GET_PIN(B, 0)
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
/* defined the LED3 pin: PB14 */
#define LED3_PIN    GET_PIN(B, 14)
/* defined the USER KEY pin: PC13 */
#define KEY_PIN    GET_PIN(C, 13)
#define SPI_CS_PIN    GET_PIN(A, 4)

static rt_uint32_t read_ts()
{
    rt_hwtimerval_t val,val1;
    rt_device_read(ts_device, 0, &val, sizeof(val));
    return (val.sec*1000000+val.usec);
}
static void handle_heart(rt_device_t device, rt_uint8_t *data, rt_size_t size)
{
	rt_uint8_t tx[64] = {0};
	rt_uint32_t t2 = 0, t3 = 0;
	rt_uint8_t ts2_ascii[8], ts3_ascii[8], crc_ascii[8];
	t2 = read_ts();
	//rt_kprintf("=> %d\n", t2);
	if (data[0] == ':' &&
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
		ByteToHexStr(t2, ts2_ascii);
		rt_memcpy(tx+23, ts2_ascii, 8);
		tx[31] = 0x3a;
		t3 = read_ts();
		ByteToHexStr(t3, ts3_ascii);
		rt_memcpy(tx+32, ts3_ascii, 8);
		tx[40] = 0x3a;
		uint32_t crc = heart_cmd_crc(tx, 41);
		ByteToHexStr(crc,crc_ascii);
		if (crc_ascii[0] == '0' &&
			crc_ascii[1] == '0' &&
			crc_ascii[2] == '0') {
			crc_ascii[0] = ' ';
			crc_ascii[1] = ' ';
			crc_ascii[2] = ' ';
		} else if (crc_ascii[0] == '0' &&
			   crc_ascii[1] == '0'	) {
			crc_ascii[0] = ' ';
			crc_ascii[1] = ' ';
		} else if (crc_ascii[0] == '0')
			crc_ascii[0] = ' ';
		rt_memcpy(tx+41, crc_ascii, 8);
		tx[49] = 0x3a;
		tx[50] = 0x03;
        	if (rt_device_write(device, /*HID_REPORT_ID_GENERAL*/0x02, tx+1, 50)
        			== 50)
		{
            		//rt_sem_take(&tx_sem_complete, RT_WAITING_FOREVER);
			rt_kprintf("heart out ok, t2 %d, t3 %d\n", t2, t3);
		}
		else
			rt_kprintf("heart out failed\n");

	}

}
static void usb_thread_entry(void *parameter)
{
    int8_t i8MouseTable[] = { -16, -16, -16, 0, 16, 16, 16, 0};
    uint8_t u8MouseIdx = 0;
    uint8_t u8MoveLen=0, u8MouseMode = 1;
    uint8_t pu8Buf[4];

    rt_device_t device = (rt_device_t)parameter;

    rt_sem_init(&tx_sem_complete, "tx_complete_sem_hid", 1, RT_IPC_FLAG_FIFO);

    //rt_device_set_tx_complete(device, event_hid_in);

    rt_kprintf("Ready.\n");

    while (1)
    {
        rt_sem_take(&tx_sem_complete, RT_WAITING_FOREVER);
    	handle_heart(device, hid_rcv, g_hid_size);
    } // while(1)
}
static void dump_data(rt_uint8_t *data, rt_size_t size)
{
    rt_size_t i;
    rt_uint8_t *ptr = data;
    for (i = 0; i < size; i++)
    {
        rt_kprintf("%c", *ptr++);
    }
    rt_kprintf("\n");

	if (data[0] == ':' &&
		data[1] == '@' &&
		data[2] == ':' &&
		data[3] == 'K') {
		rt_memset(hid_rcv, 0, 64);
		rt_memcpy(hid_rcv, data, size);
		g_hid_size = size;
    		rt_sem_release(&tx_sem_complete);
	}
}
static void dump_report(struct hid_report * report)
{
    dump_data(report->report,report->size);
}
void HID_Report_Received(hid_report_t report)
{
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
    err = rt_device_control(ts_device, HWTIMER_CTRL_MODE_SET, &mode);
    val.sec = 5*60*60;
    val.usec = 0;
    rt_kprintf("SetTime: Sec %d, Usec %d\n", val.sec, val.usec);
    if (rt_device_write(ts_device, 0, &val, sizeof(val)) != sizeof(val))
    	    rt_kprintf("set timer failed\n");
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
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SPI_CS_PIN, PIN_MODE_OUTPUT);

    generic_hid_init();
    while (count++)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_LOW);
    }

    return RT_EOK;
}
