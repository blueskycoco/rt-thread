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
#include "lcd.h"
/* defined the LED0 pin: PC13 */
#define LED0_PIN    GET_PIN(B, 14)
#define LED1_PIN    GET_PIN(B, 15)
#define SET_PIN    	GET_PIN(A, 1)
#define RST_PIN		GET_PIN(A, 0)
static rt_device_t dev;
static struct rt_semaphore sem;

static rt_err_t air_input(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&sem);
	return RT_EOK;
}

unsigned char FucCheckSum(unsigned char *i,unsigned char ln)
{
	unsigned char j;
	unsigned char tempq=0;
	i+=1;
	for(j=0;j<(ln-2);j++)
	{
		tempq+=*i;
		i++;
	}
	tempq=(~tempq)+1;
	return(tempq);
}
static void air_thread(void *parm)
{
	char ch;
	char buf[32] = {0};
	char str[64] = {0};
	int len = 0;
	int i = 0, j = 0;
	rt_uint8_t started = 0;
	rt_uint8_t cmd[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
	rt_kprintf("startup air thread\n");
	//rt_device_write(dev, 0, cmd, sizeof(cmd));
    rt_pin_write(SET_PIN, PIN_HIGH);
    rt_pin_write(RST_PIN, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(RST_PIN, PIN_HIGH);
    rt_thread_mdelay(100);
	while (1)
	{
		while((len = rt_device_read(dev, -1, &ch, 1)) != 1) {
			rt_sem_take(&sem, RT_WAITING_FOREVER);
		}
		if (len == 1) {
			buf[i++] = ch;
			if (ch != 0x42)
				rt_kprintf(" %02x", ch);
			else
				rt_kprintf("\r\n%02x", ch);
			if (ch == 0x4d && buf[i-2] == 0x42) {
				started = 1;
				j=i;
			}
			if (started && (i-j)==15) {
					rt_sprintf(str, "pm1.0 %03d       pm2.5 %03d       pm10  %03d",
							buf[i-7]<<8|buf[i-6],
							buf[i-5]<<8|buf[i-4],
							buf[i-3]<<8|buf[i-2]);
					rt_kprintf("\r\n%s\r\n", str);	
					ST7585_Write_String(0, 3, str);
				i=0;
				j=0;
				started = 0;
			}
			/*
			if (i == 24) {
				for (int j=0; j<24; j++)
					rt_kprintf("%02x ", buf[j]);
				rt_kprintf("\n");
				if (buf[0] == 0x42 && buf[1] == 0x4d) {
					rt_sprintf(str, "pm1.0 %03d       pm2.5 %03d       pm10  %03d",
							buf[10]<<8|buf[11],
							buf[12]<<8|buf[13],
							buf[14]<<8|buf[15]);
					
					ST7585_Write_String(0, 3, str);
				}
				i=0;
			}*/
			/*
			if (ch == 0x42) {
				if (i > 1 && buf[0] == 0x42 && buf[1] == 0x4d) {
					unsigned char crc = FucCheckSum(buf, i);
					rt_kprintf("\r\ncrc is %04x %02x", crc, ch);
					i=0;
					buf[i++] = ch;
					
				} else
					rt_kprintf("\r\n%02x", ch);
			}
			else
				rt_kprintf(" %02x", ch);*/
		}
	}
}
int main(void)
{
    int count = 1;
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SET_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(RST_PIN, PIN_MODE_OUTPUT);
	dev = rt_device_find("uart2");
	if (!dev){
		rt_kprintf("can not find uart2\n");
	} else {
		rt_kprintf("find uart3\n");
		rt_sem_init(&sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
		rt_device_open(dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
    	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
		config.baud_rate = BAUD_RATE_9600; 
    	rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &config);
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
