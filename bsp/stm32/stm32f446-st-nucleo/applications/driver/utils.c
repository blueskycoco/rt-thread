#include <rtthread.h>
#include "utils.h"

int timestamp_init()
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
	
	return 0;
}

static rt_uint32_t read_ts()
{
	rt_hwtimerval_t val,val1;
	rt_device_read(ts_device, 0, &val, sizeof(val));
	return (val.sec*1000000+val.usec);
}

void read_ts_64(rt_uint8_t *buf)
{
	static uint32_t old_sensor_ts = 0;
	static uint32_t up_ts = 0;
	rt_uint32_t ts = read_ts();
	
	if (ts < old_sensor_ts) {
		up_ts++;
	}
	
	old_sensor_ts = ts;
	buf[0] = ts & 0xff;
	buf[1] = (ts >> 8) & 0xff;
	buf[2] = (ts >> 16) & 0xff;
	buf[3] = (ts >> 24) & 0xff;
	buf[4] = up_ts & 0xff;
	buf[5] = (up_ts >> 8) & 0xff;
	buf[6] = (up_ts >> 16) & 0xff;
	buf[7] = (up_ts >> 24) & 0xff;
}
