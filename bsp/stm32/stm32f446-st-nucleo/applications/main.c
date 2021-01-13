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
#include "icm20603.h"
#include "icm42688.h"
#include <fal.h>
#include "param.h"
#include "utils.h"
#include "mcu.h"

#define DRV_DEBUG
#define LOG_TAG             "main.mcu"
#include <drv_log.h>

extern int fal_init(void);

static struct rt_thread uart_thread;
ALIGN(RT_ALIGN_SIZE)
static char uart_thread_stack[1024];
rt_device_t uart_device;

#ifdef RT_USING_USB_DEVICE	
uint8_t hid_rcv[64] = {0};
rt_size_t g_hid_size;
rt_device_t hid_device;
rt_bool_t 	hid_ready = RT_FALSE;
icm20603_device_t icm_dev;
icm42688_device_t icm_42688;
rt_sem_t sof_sem;
rt_sem_t hid_tx_comp;
#endif
rt_sem_t uart_rx_ind;
rt_sem_t uart_tx_comp;
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
#define LED1_PIN    GET_PIN(B, 0)
#define LED3_PIN    GET_PIN(B, 14)
#define ICM_INT_PIN GET_PIN(B, 4)

static rt_err_t uart_out_finish(rt_device_t dev, void *buffer)
{
    return rt_sem_release(uart_tx_comp);
}

static rt_err_t uart_data_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(uart_rx_ind);
	return RT_EOK;
}

#ifdef RT_USING_USB_DEVICE	

static void icm_isr(void *parameter)
{
	rt_sem_release(sof_sem);
}

static rt_err_t event_hid_finish(rt_device_t dev, void *buffer)
{
    return rt_sem_release(hid_tx_comp);
}
static rt_err_t usb_sof_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(sof_sem);
	return RT_EOK;
}

static void icm_thread_entry(void *parameter)
{
	rt_int16_t ax, ay, az;
	rt_int16_t gx, gy, gz;
	uint8_t buf[64], int_status;
	uint32_t uax, uay, uaz, ugx, ugy, ugz;

	sof_sem = rt_sem_create("icm", 0, RT_IPC_FLAG_FIFO);
	rt_pin_mode(ICM_INT_PIN, PIN_MODE_INPUT_PULLUP);
	rt_pin_attach_irq(ICM_INT_PIN, PIN_IRQ_MODE_FALLING, icm_isr, RT_NULL);

	icm_dev = icm20603_init();
	icm_42688 = icm42688_init();
	//icm20603_calib_level(icm_dev, 10);
	//rt_pin_irq_enable(ICM_INT_PIN, RT_TRUE);

	while (1)
	{
		if (rt_sem_take(sof_sem, -1) != RT_EOK)
		{
			LOG_D("wait imu timeout\r\n");
			continue;
		}
		if (icm20603_int_status(icm_dev) & 0x01 != 0x01)
			LOG_D("icm20603 not ready\r\n");
		if (icm42688_int_status(icm_42688) & 0x08 != 0x01)
			LOG_D("icm42688 not ready\r\n");
		//icm20603_get_gyro(icm_dev, (rt_int16_t *)&gx, (rt_int16_t *)&gy,
		//		(rt_int16_t *)&gz);
		icm20603_get_accel(icm_dev, (rt_int16_t *)&ax, (rt_int16_t *)&ay,
				(rt_int16_t *)&az, (rt_int16_t *)&gx,
				(rt_int16_t *)&gy, (rt_int16_t *)&gz);
		//LOG_D("accelerometer: %10d, %10d, %10d     gyro: %10d, %10d, %10d\r\n",
		//		ax, ay, az, gx, gy, gz);
		uax = (uint32_t)ax;
		uay = (uint32_t)ay;
		uaz = (uint32_t)az;
		ugx = (uint32_t)gx;
		ugy = (uint32_t)gy;
		ugz = (uint32_t)gz;
		buf[0] = 0x02;
		
		buf[1]  = (uax >> 24) & 0xff; 
		buf[2]  = (uax >> 16) & 0xff; 
		buf[3]  = (uax >>  8) & 0xff; 
		buf[4]  = (uax >>  0) & 0xff; 
		
		buf[5]  = (uay >> 24) & 0xff; 
		buf[6]  = (uay >> 16) & 0xff; 
		buf[7]  = (uay >>  8) & 0xff; 
		buf[8]  = (uay >>  0) & 0xff; 
		
		buf[9]  = (uaz >> 24) & 0xff; 
		buf[10] = (uaz >> 16) & 0xff; 
		buf[11] = (uaz >>  8) & 0xff; 
		buf[12] = (uaz >>  0) & 0xff; 
		
		buf[13] = (ugx >> 24) & 0xff; 
		buf[14] = (ugx >> 16) & 0xff; 
		buf[15] = (ugx >>  8) & 0xff; 
		buf[16] = (ugx >>  0) & 0xff; 
		
		buf[17] = (ugy >> 24) & 0xff; 
		buf[18] = (ugy >> 16) & 0xff; 
		buf[19] = (ugy >>  8) & 0xff; 
		buf[20] = (ugy >>  0) & 0xff; 
		
		buf[21] = (ugz >> 24) & 0xff; 
		buf[22] = (ugz >> 16) & 0xff; 
		buf[23] = (ugz >>  8) & 0xff; 
		buf[24] = (ugz >>  0) & 0xff; 
		icm42688_get_accel(icm_42688, (rt_int16_t *)&ax, (rt_int16_t *)&ay,
				(rt_int16_t *)&az, (rt_int16_t *)&gx,
				(rt_int16_t *)&gy, (rt_int16_t *)&gz);
		uax = (uint32_t)ax;
		uay = (uint32_t)ay;
		uaz = (uint32_t)az;
		ugx = (uint32_t)gx;
		ugy = (uint32_t)gy;
		ugz = (uint32_t)gz;
		
		buf[25]  = (uax >> 24) & 0xff; 
		buf[26]  = (uax >> 16) & 0xff; 
		buf[27]  = (uax >>  8) & 0xff; 
		buf[28]  = (uax >>  0) & 0xff; 
		
		buf[29]  = (uay >> 24) & 0xff; 
		buf[30]  = (uay >> 16) & 0xff; 
		buf[31]  = (uay >>  8) & 0xff; 
		buf[32]  = (uay >>  0) & 0xff; 
		
		buf[33]  = (uaz >> 24) & 0xff; 
		buf[34] = (uaz >> 16) & 0xff; 
		buf[35] = (uaz >>  8) & 0xff; 
		buf[36] = (uaz >>  0) & 0xff; 
		
		buf[37] = (ugx >> 24) & 0xff; 
		buf[38] = (ugx >> 16) & 0xff; 
		buf[39] = (ugx >>  8) & 0xff; 
		buf[40] = (ugx >>  0) & 0xff; 
		
		buf[41] = (ugy >> 24) & 0xff; 
		buf[42] = (ugy >> 16) & 0xff; 
		buf[43] = (ugy >>  8) & 0xff; 
		buf[44] = (ugy >>  0) & 0xff; 
		
		buf[45] = (ugz >> 24) & 0xff; 
		buf[46] = (ugz >> 16) & 0xff; 
		buf[47] = (ugz >>  8) & 0xff; 
		buf[48] = (ugz >>  0) & 0xff; 
		if (hid_ready) {
			if (rt_device_write(hid_device, 0x02, buf+1, 63) != 63)
				LOG_D("hid write failed %d\r\n", errno);
			else {
				if (rt_sem_take(hid_tx_comp, 300) != RT_EOK) {
					hid_ready = RT_FALSE;
					LOG_D("waiting hid out timeout\r\n");
				}
			}
		}
	}
}
#endif


void dump_host_cmd(uint8_t *tag, uint8_t *cmd, uint32_t len)
{
	uint32_t i;

	rt_kprintf("=====================================> %s[%d]: \r\n",
			tag, len);
	for (i=0; i<len; i++) {
		rt_kprintf("%02x ", cmd[i]);
		if ((i+1) % 16 == 0 && i != 0)
			rt_kprintf("\r\n");
	}
	rt_kprintf("\r\n\r\n");
}

void uart_rsp_out(uint8_t *data, uint16_t len)
{
	if (data == RT_NULL || len == 0)
		return;

	rt_device_write(uart_device, 0, data, len);
	if (RT_EOK != rt_sem_take(uart_tx_comp, 100))
		LOG_E("wait for uart tx comp failed\r\n");

	notify_event(EVENT_ST2OV);
}
static void uart_thread_entry(void *parameter)
{
	int len;
	uint8_t uart_buf[64];

	rt_device_t device = (rt_device_t)parameter;

	uart_rx_ind = rt_sem_create("uart_rx_ind", 0, RT_IPC_FLAG_FIFO);
	uart_tx_comp = rt_sem_create("uart_tx_out", 0, RT_IPC_FLAG_FIFO);

	rt_device_set_rx_indicate(device, uart_data_ind);
	rt_device_set_tx_complete(device, uart_out_finish);
	protocol_init();
	LOG_D("Uart Ready.\n");

	while (1)
	{
		rt_sem_take(uart_rx_ind, RT_WAITING_FOREVER);
		/* handle host command */
		len = 0;
		do {
			len += rt_device_read(device, 0, uart_buf+len, 64-len);
			if (len > 0) {
				if (len == 64) {
					if (!insert_mem(TYPE_H2D, uart_buf, len))
						LOG_E("queue host cmd failed\r\n");
					notify_event(EVENT_OV2ST);
					break;
				}
			} else {
				break;
			}
		} while (1);
	}
}


static int mcu_cmd_init(void)
{
	int err = 0;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	uart_device = rt_device_find("uart6");

	RT_ASSERT(uart_device != RT_NULL);

	err = rt_device_open(uart_device, RT_DEVICE_FLAG_RDWR |
			RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX);

	if (err != RT_EOK)
	{
		LOG_D("open dev failed!\n");
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
#ifdef RT_USING_USB_DEVICE
void HID_Report_Received(hid_report_t report)
{
	hid_ready = RT_TRUE;
	LOG_D("app started\r\n");
}
static int generic_hid_init(void)
{
	int err = 0;
	hid_device = rt_device_find("hidd");
	RT_ASSERT(hid_device != RT_NULL);

	err = rt_device_open(hid_device, RT_DEVICE_FLAG_RDWR);

	if (err != RT_EOK)
	{
		LOG_D("open dev failed!\n");
		return -1;
	}
	hid_tx_comp = rt_sem_create("hid", 0, RT_IPC_FLAG_FIFO);
	rt_device_set_tx_complete(hid_device, event_hid_finish);
	rt_device_set_rx_indicate(hid_device, usb_sof_ind);
	return 0;
}
#endif
int main(void)
{
	int count = 1;

	if (!param_init())
		LOG_D("can't startup system\r\n");

#ifdef RT_USING_USB_DEVICE
	rt_thread_t tid = rt_thread_create("icm", icm_thread_entry, RT_NULL,
			2048, 28, 20);
	rt_thread_startup(tid);
#endif
	/* set LED2 pin mode to output */
	rt_memlist_init();	
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);
	timestamp_init();
	mcu_cmd_init();
#ifdef RT_USING_USB_DEVICE
	generic_hid_init();
#endif
	while (count++)
	{
		rt_pin_write(LED1_PIN, PIN_HIGH);
		rt_thread_mdelay(200);
		rt_pin_write(LED2_PIN, PIN_HIGH);
		rt_thread_mdelay(200);
		rt_pin_write(LED3_PIN, PIN_HIGH);
		rt_thread_mdelay(200);
		rt_pin_write(LED1_PIN, PIN_LOW);
		rt_thread_mdelay(200);
		rt_pin_write(LED2_PIN, PIN_LOW);
		rt_thread_mdelay(200);
		rt_pin_write(LED3_PIN, PIN_LOW);
		rt_thread_mdelay(200);
	}

	return RT_EOK;
}
INIT_COMPONENT_EXPORT(fal_init);

#ifdef FINSH_USING_MSH
int dump_len(void)
{
	uint8_t *data;
	uint32_t i;
	uint16_t len, all_len;
	do {
		remove_mem(TYPE_H2D, &data, &len);
		if (data == RT_NULL)
			break;
		all_len += len;
		LOG_D("============>[%04d]: ", all_len);
		for (i=0; i<len; i++) {
			LOG_D("%c", data[i]);
		}
	} while (1);
	return 0;
}
MSH_CMD_EXPORT(dump_len, dump len);
#endif /* FINSH_USING_MSH */
