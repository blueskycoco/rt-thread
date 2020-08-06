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
#include "icm20603.h"
#include "crc.h"
static struct rt_thread usb_thread;
ALIGN(RT_ALIGN_SIZE)
static char usb_thread_stack[1024];
static struct rt_semaphore tx_sem_complete;
rt_device_t ts_device;
rt_uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size;
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


static rt_err_t event_hid_in(rt_device_t dev, void *buffer)
{
    rt_sem_release(&tx_sem_complete);
    return RT_EOK;
}

static rt_uint32_t read_ts()
{
    rt_hwtimerval_t val,val1;
    rt_device_control(ts_device, HWTIMER_CTRL_STOP, RT_NULL);
    rt_device_read(ts_device, 0, &val, sizeof(val));
    val1.sec = 10;
    val1.usec = 0;
    if (rt_device_write(ts_device, 0, &val1, sizeof(val1)) != sizeof(val1))
    	    rt_kprintf("set timer failed\n");
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
#if 0
	    u8MouseMode ^= 1;
        if (u8MouseMode)
        {
            if (u8MoveLen > 14)
            {
                /* Update new report data */
                pu8Buf[0] = 0x00;
                pu8Buf[1] = i8MouseTable[u8MouseIdx & 0x07];
                pu8Buf[2] = i8MouseTable[(u8MouseIdx + 2) & 0x07];
                pu8Buf[3] = 0x00;
                u8MouseIdx++;
                u8MoveLen = 0;
            }
        }
        else
        {
            pu8Buf[0] = pu8Buf[1] = pu8Buf[2] = pu8Buf[3] = 0;
        }

        u8MoveLen++;

        if (rt_device_write(device, HID_REPORT_ID_GENERAL, pu8Buf, 4) == 0)
        {
            /* Sleep 200 Milli-seconds */
            rt_thread_mdelay(200);
        }
        else
        {
            /* Wait it done. */
            rt_sem_take(&tx_sem_complete, RT_WAITING_FOREVER);
        }
#endif

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
    //rt_kprintf("\nHID Recived:");
    //rt_kprintf("\nReport ID %02x \n", report->report_id);
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
    val.sec = 10;
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
	rt_thread_t tid = rt_thread_create("icm", icm_thread_entry, RT_NULL,
						2048, 28, 20);
    	rt_thread_startup(tid);
	/* set LED2 pin mode to output */
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	generic_hid_init();
	while (count++)
	{
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_thread_mdelay(500);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_thread_mdelay(500);

	}

	return RT_EOK;
}
