#ifndef _TSLIB_H_
#define _TSLIB_H_
/*
 *  tslib/src/tslib.h
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.
 *
 * $Id: tslib.h,v 1.4 2005/02/26 01:47:23 kergoth Exp $
 *
 * Touch screen library interface definitions.
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdarg.h>



#ifndef ts_error
#define ts_error printf
#endif


struct tsdev 
{
	int fd;	
};

struct ts_sample //������һ������
{
	int		x;
	int		y;
	unsigned int	pressure;
	//struct timeval	tv;//ʱ�䣬��ֲ��STM32ƽ̨��Ӧ�ò���Ҫ
};

/*
	ʹ�����У׼������5��LCD��������������ݴ��룬�����7��У׼����
	ԭ������У׼�ļ��ģ���ֲ��ȫ������tslibģ��
*/
typedef struct {
	int x[5], xfb[5]; //x,y�Ǵ�������xfb��yfb�Ƕ�Ӧ��LCD����ֵ
	int y[5], yfb[5];
	unsigned int a[7];	//У׼�õ���7������
} calibration;

/*
 * Close the touchscreen device, free all resources.
 */
extern int ts_close(struct tsdev *);

/*
 * Configure the touchscreen device.
 */
extern int ts_config(struct tsdev *);

/*
 * Open the touchscreen device.
 */
extern struct tsdev *ts_open(const char *dev_name, int nonblock);

/*
 * Return a scaled touchscreen sample.
 */
extern int ts_read(struct tsdev *, struct ts_sample *, int);

/*
 * Returns a raw, unscaled sample from the touchscreen.
 */
extern int ts_read_raw(struct tsdev *, struct ts_sample *, int);

extern struct tsdev *ts_open_module(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _TSLIB_H_ */


