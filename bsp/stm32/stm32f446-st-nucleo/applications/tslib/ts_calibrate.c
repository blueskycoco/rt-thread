/*
 *  tslib/tests/ts_calibrate.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_calibrate.c,v 1.8 2004/10/19 22:01:27 dlowder Exp $
 *
 * Basic test program for touchscreen library.

	原来的校验程序是基于linux LCD显示驱动的，
	在STM32上，没有LCD的显示架构，
	因此本校验程序修改较大。

 */
#include <stdio.h>
#include "tslib.h"
#include <stdarg.h>
#include "ili9325.h"
#include "tslib.h"
/* -1 538 -1026664 716 -6 -1157373 65536 */
extern void put_cross(int x, int y);

extern int getxy(struct tsdev *ts, int *x, int *y);
/*

	进行校准计算，这个函数才是核心算法

	1.4源码里面的算法有问题，从网上找了另外一个算法，
	参考《Tslib中触摸屏校准原理及其实现.pdf》

*/
int perform_calibration(calibration *cal) 
{
    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, det1, det2, det3;
    float scaling = 65536.0;
    
    n = x = y = x2 = y2 = xy = 0;
    for (j = 0; j < 5; j++)
    {
        n += 1.0;
        x += (float)cal->x[j];
        y += (float)cal->y[j];
        x2 += (float)(cal->x[j] * cal->x[j]);
        y2 += (float)(cal->y[j] * cal->y[j]);
        xy += (float)(cal->x[j] * cal->y[j]);
    }
    
    det = n * (x2*y2 - xy*xy) + x * (xy*y - x*y2) + y * (x*xy - y*x2);
    if (det < 0.1 && det > -0.1)
    {
    	rt_kprintf("ts_calibrate: determinant is too small -- %f\n",det);
        return -1;
    }
    
    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal->xfb[j];
        zx += (float)(cal->xfb[j] * cal->x[j]);
        zy += (float)(cal->xfb[j] * cal->y[j]);
    }
    
    det1 = n * (zx*y2 - xy*zy) + z * (xy*y - x*y2) + y * (x*zy - y*zx);
    det2 = n * (x2*zy - zx*xy) + x * (zx*y - x*zy) + z * (x*xy - y*x2);
    det3 = z * (x2*y2 - xy*xy) + x * (xy*zy - zx*y2) + y * (zx*xy - zy*x2);
    cal->a[0] = (int)((det1 / det) * scaling);
    cal->a[1] = (int)((det2 / det) * scaling);
    cal->a[2] = (int)((det3 / det) * scaling);

    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal->yfb[j];
        zx += (float)(cal->yfb[j] * cal->x[j]);
        zy += (float)(cal->yfb[j] * cal->y[j]);
    }
    
    det1 = n * (zx*y2 - xy*zy) + z * (xy*y - x*y2) + y * (x*zy - y*zx);
    det2 = n * (x2*zy - zx*xy) + x * (zx*y - x*zy) + z * (x*xy - y*x2);
    det3 = z * (x2*y2 - xy*xy) + x * (xy*zy - zx*y2) + y * (zx*xy - zy*x2);
    cal->a[3] = (int)((det1 / det) * scaling);
    cal->a[4] = (int)((det2 / det) * scaling);
    cal->a[5] = (int)((det3 / det) * scaling);
    cal->a[6] = (int)scaling;

	rt_kprintf("calibration ok!\r\n");
    return 0;
}

static void get_sample (struct tsdev *ts, calibration *cal,
			int index, int x, int y, char *name)
{
	//static int last_x = -1, last_y;

	getxy (ts, &cal->x [index], &cal->y [index]);

	//last_x = 
	//last_y = 
	cal->xfb [index] = x;
	cal->yfb [index] = y;
	
	rt_kprintf("\r\n %s : X = %4d Y = %4d\r\n", name, cal->x [index], cal->y [index]);
}
/*

	校准过程，与LCD显示相关
	使用竖屏校准？

*/
/*
	测试，直接定义一个cal用于保存校准数据
	实际项目中最好是将cal保存在文件中
*/	
calibration cal;

struct tsdev *ts_open_module(void)
{
	struct tsdev *ts;
	char *tsdevice = NULL;
	unsigned int i;

	rt_kprintf("env Calibration constants: ");
	for (i = 0; i < 7; i++) 
		rt_kprintf( "%d ", cal.a [i]);
	rt_kprintf("\n");
	/*先配置校准参数，再调ts_open*/
	ts_set_cal(&cal);

	ts = ts_open(tsdevice,0);

	if (!ts) 
	{
		return NULL;
	}
	
	if (ts_config(ts)) 
	{
		return NULL;
	}
	
	return ts;
}
/*
	校验流程
*/
int ts_calibrate(void)
{
	struct tsdev *ts;
	char *tsdevice = NULL;
	int i;

	rt_kprintf(" main ts calibrate!\r\n");

	ts = ts_open(tsdevice,0);
	if (!ts) 
	{
		return -1;
	}

	if (ts_config(ts)) 
	{
		return -1;
	}

	/*下面开始进入校准*/
	/*刷LCD背景*/

	/*  显示提示语     */
	int xres = 240;
	int yres = 320;

	put_string(8, yres / 4,
			   "TSLIB calibration utility", RED);
	put_string(8, yres / 4 + 20,
			   "Touch crosshair to calibrate", RED);

	rt_kprintf("xres = %d, yres = %d\n", xres, yres);
	
	//----校准过程，获取5个点的数据-------
	put_cross(  50, 50);
	clr_cross(  xres - 50, 50);
	clr_cross(  xres - 50, yres - 50);
	clr_cross(50,  yres - 50);
	clr_cross(xres / 2, yres / 2);
	get_sample (ts, &cal, 0, 50,        50,        "Top left");
	rt_kprintf("-----------------------Top left finish\r\n");

	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	put_cross(  xres - 50, 50);
	clr_cross(  50, 50);
	clr_cross(  xres - 50, yres - 50);
	clr_cross(50,  yres - 50);
	clr_cross(xres / 2, yres / 2);
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	get_sample (ts, &cal, 1, xres - 50, 50,        "Top right");
	rt_kprintf("%s %d\r\n", __func__, __LINE__);
	rt_kprintf("-----------------------Top right finish\r\n");

	put_cross(  xres - 50, yres - 50);
	clr_cross(  50, 50);
	clr_cross(  xres - 50, 50);
	clr_cross(50,  yres - 50);
	clr_cross(xres / 2, yres / 2);
	get_sample (ts, &cal, 2, xres - 50, yres - 50, "Bot right");
	rt_kprintf("-----------------------Bot right finish\r\n");

	put_cross(50,  yres - 50);
	clr_cross(  50, 50);
	clr_cross(  xres - 50, 50);
	clr_cross(  xres - 50, yres - 50);
	clr_cross(xres / 2, yres / 2);
	get_sample (ts, &cal, 3, 50,        yres - 50, "Bot left");
	rt_kprintf("-----------------------Bot left finish\r\n");

	put_cross(xres / 2, yres / 2);
	clr_cross(  50, 50);
	clr_cross(  xres - 50, 50);
	clr_cross(  xres - 50, yres - 50);
	clr_cross(50,  yres - 50);
	get_sample (ts, &cal, 4, xres / 2,  yres / 2,  "Center");
	rt_kprintf("-----------------------Center\r\n");

	if (0 == perform_calibration (&cal)) 
	{
		//校准后得到的数据
		rt_kprintf("Calibration constants: ");
		for (i = 0; i < 7; i++) 
			rt_kprintf("%d ", cal.a [i]);
		rt_kprintf("\n");
		clr_cross(xres / 2, yres / 2);
		put_string(60, yres / 4 + 80,
			"Calibration Success", GREEN);

	} 
	else 
	{
		rt_kprintf("Calibration failed.\n");
		i = -1;
		put_string(60, yres / 4 + 80,
			"Calibration Failed", RED);
	}

	return i;
}


extern signed int dev_lcd_drawpoint(uint16_t x, uint16_t y, uint16_t color);

signed int ts_calibrate_test(void)
{
	struct tsdev *ts;

	ts = ts_open_module();
	
	while(1)
	{
		struct ts_sample samp;
		int ret;
		
		ret = ts_read(ts, &samp, 1);
		if(ret == 1)
		{
			//uart_rt_kprintf("pre:%d, x:%d, y:%d\r\n", samp.pressure, samp.x, samp.y);
			if(samp.pressure !=0 )
			{
				//uart_rt_kprintf("pre:%d, x:%d, y:%d\r\n", samp.pressure, samp.x, samp.y);
				set_pixel(0xf800, samp.x, samp.y);	
			}
		}
		
	}	
}


