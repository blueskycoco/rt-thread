/*
 *  tslib
 *
 *  Copyright (C) 2003 Chris Larson.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tslib.h"

extern int dev_touchscreen_read(struct ts_sample *samp, int nr);
/**
 *@brief:      ts_input_read
 *@details:    ����������������
 *@param[in]                          
 *@param[out]  ��
 *@retval:     static
 */
static int ts_input_read(struct ts_sample *samp, int nr)
{
	int ret = nr;
	
	ret = dev_touchscreen_read(samp, nr);

	return ret;
}
/*-------------------------------------------------------------------------------*/
struct tslib_linear 
{
	int	swap_xy;

// Linear scaling and offset parameters for pressure
	int	p_offset;
	int	p_mult;
	int	p_div;

// Linear scaling and offset parameters for x,y (can include rotation)
	int	a[7];
};
struct tslib_linear TslibLinear;

/**
 *@brief:      linear_read
 *@details:    У׼
 *@param[in]   struct tslib_module_info *info  
               struct ts_sample *samp          
               int nr                          
 *@param[out]  ��
 *@retval:     static
 */
static int linear_read(struct ts_sample *samp, int nr)
{
	struct tslib_linear *lin = &TslibLinear;
	
	int ret;
	int xtemp,ytemp;

	// ����һ���ȡ����
	ret =  ts_input_read(samp, nr);
	//����ret������
	if (ret >= 0) 
	{
		int nr;//��������һ��nr����?�б�ҪҲ��nr���������

		for (nr = 0; nr < ret; nr++, samp++) 
		{
			xtemp = samp->x; ytemp = samp->y;

			samp->x = 	( lin->a[2] +
					lin->a[0]*xtemp + 
					lin->a[1]*ytemp ) / lin->a[6];
			
			samp->y =	( lin->a[5] +
					lin->a[3]*xtemp +
					lin->a[4]*ytemp ) / lin->a[6];
			
			samp->pressure = ((samp->pressure + lin->p_offset)
						 * lin->p_mult) / lin->p_div;

			/*XY��Ե�*/
			if (lin->swap_xy) 
			{
				int tmp = samp->x;
				samp->x = samp->y;
				samp->y = tmp;
			}
		}
	}

	return ret;
}

calibration TsCalSet;

static int mod_linear_init(void)
{

	struct tslib_linear *lin;
	
	rt_kprintf("mod_linear_init\r\n");

	lin = &TslibLinear;

	/*  �����ĸ����ݲ����޸ģ�  */
	lin->p_offset = 0;
	lin->p_mult   = 1;
	lin->p_div    = 1;
	lin->swap_xy  = 0;

	/*
	 �������ݾ��ǻ�ȡϵͳ��У׼����
	 */
	lin->a[0] = TsCalSet.a[0];
	lin->a[1] = TsCalSet.a[1];
	lin->a[2] = TsCalSet.a[2];
	lin->a[3] = TsCalSet.a[3];
	lin->a[4] = TsCalSet.a[4];
	lin->a[5] = TsCalSet.a[5];
	lin->a[6] = TsCalSet.a[6];
		
	rt_kprintf("mod linear init ok\r\n");
	
	return 1;
}

/*-------------------------------------------------------------------------------*/
#define VAR_PENDOWN 		0x00000001
#define VAR_LASTVALID		0x00000002
#define VAR_NOISEVALID		0x00000004
#define VAR_SUBMITNOISE 	0x00000008

struct tslib_variance 
{
	int delta;
    struct ts_sample last;	//��һ������
    struct ts_sample noise;//���������ɣ�
	unsigned int flags;
};

struct tslib_variance TsVariance;

static int sqr (int x)
{
	return x * x;
}
/**
 *@brief:      variance_read
 *@details:    �˲��㷨���
 				����ɵ�
 *@param[in]   struct tslib_module_info *info  
               struct ts_sample *samp          
               int nr                          
 *@param[out]  ��
 *@retval:     static
 */
static int variance_read(struct ts_sample *samp, int nr)
{
	struct tslib_variance *var = &TsVariance;
	
	struct ts_sample cur;
	int count = 0, dist;

	while (count < nr) 
	{
		/*
			���N+1��N+2�������N��ľ��볬����ֵ��������ǿ����ƶ���
			��ʱ�ὫVAR_SUBMITNOISE��ʶ����
			�������ʶ��Ϊ����һ��ѭ���ж�N+1��N+2֮���Ƿ�Ҳ������ֵ��
		*/
		if (var->flags & VAR_SUBMITNOISE) 
		{
			cur = var->noise;////�����»�ȡ�����㣬���ǽ�noise�е�������Ϊ��ǰ����
			var->flags &= ~VAR_SUBMITNOISE;
		} 
		else 
		{
			/*  
			��ȡһ��������
			*/
			if (linear_read(&cur, 1) < 1)
				return count;
		}

		if (cur.pressure == 0)//����ѹ��ֵΪ0, 
		{
			/* Flush the queue immediately when the pen is just
			 * released, otherwise the previous layer will
			 * get the pen up notification too late. This 
			 * will happen if info->next->ops->read() blocks.
			 */
			if (var->flags & VAR_PENDOWN) 
			{
				/*��һ��pressure��Ϊ0���ὫVAR_PENDOWN ��ʶ����
				���ѹ��Ϊ0��˵����������ʣ����ܶ��ѣ�Ҳ�����Ǹ���������
				�ȱ��浽noise*/
				var->flags |= VAR_SUBMITNOISE;
				var->noise = cur;//�Ƚ����㱣�浽noise
			}
			/* Reset the state machine on pen up events. */
			var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);
			goto acceptsample;
		} else
			var->flags |= VAR_PENDOWN;

		if (!(var->flags & VAR_LASTVALID)) 
		{
			var->last = cur;
			var->flags |= VAR_LASTVALID;
			continue;
		}

		if (var->flags & VAR_PENDOWN) {
			/* Compute the distance between last sample and current */
			dist = sqr (cur.x - var->last.x) +
			       sqr (cur.y - var->last.y);

			if (dist > var->delta) {
				//uart_rt_kprintf("%d-",dist);
				/* 
				Do we suspect the previous sample was a noise? 
				
				��һ���������Ҳ������ֵ���ͻὫVAR_NOISEVALID��λ��
				��ε������ֳ�����ֵ����ô�Ϳ�����Ϊ�ǿ����ƶ�
				*/
				if (var->flags & VAR_NOISEVALID) {
					//uart_rt_kprintf("q-");	
					/* Two "noises": it's just a quick pen movement */
					samp [count++] = var->last = var->noise;
					
					var->flags = (var->flags & ~VAR_NOISEVALID) |
									VAR_SUBMITNOISE;
				} else{
					/*��һ�γ�����ֵ����λ��־*/
					var->flags |= VAR_NOISEVALID;
				}
				/* The pen jumped too far, maybe it's a noise ... */
				var->noise = cur;
				continue;
			} else{
				//uart_rt_kprintf("g ");
				var->flags &= ~VAR_NOISEVALID;
			}
		}

acceptsample:
		samp [count++] = var->last;
		var->last = cur;
	}
	
	return count;
}


static int mod_variance_init(void)
{
	struct tslib_variance *var;

	rt_kprintf("mod_variance_init\r\n");

	var = &TsVariance;

	if (var == NULL)
		return NULL;

	var->delta = 10;
	var->flags = 0;

    var->delta = sqr (var->delta);

	return 1;
}

/*-------------------------------------------------------------------------------*/

/**
 * This filter works as follows: we keep track of latest N samples,
 * and average them with certain weights. The oldest samples have the
 * least weight and the most recent samples have the most weight.
 * This helps remove the jitter and at the same time doesn't influence
 * responsivity because for each input sample we generate one output
 * sample; pen movement becomes just somehow more smooth.
 */

#define NR_SAMPHISTLEN	4

/* To keep things simple (avoiding division) we ensure that
 * SUM(weight) = power-of-two. Also we must know how to approximate
 * measurements when we have less than NR_SAMPHISTLEN samples.
 */
static const unsigned char weight [NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
{
	/* The last element is pow2(SUM(0..3)) */
	{ 5, 3, 0, 0, 3 },	/* When we have 2 samples ... */
	{ 8, 5, 3, 0, 4 },	/* When we have 3 samples ... */
	{ 6, 4, 3, 3, 4 },	/* When we have 4 samples ... */
};

struct ts_hist {
	int x;
	int y;
	unsigned int p;
};

struct tslib_dejitter 
{
	int delta;
	int x;
	int y;
	int down;
	int nr;
	int head;
	struct ts_hist hist[NR_SAMPHISTLEN];
};

struct tslib_dejitter TsDejitter;

static void average (struct tslib_dejitter *djt, struct ts_sample *samp)
{
	const unsigned char *w;
	int sn = djt->head;
	int i, x = 0, y = 0;
	unsigned int p = 0;

    w = weight [djt->nr - 2];

	for (i = 0; i < djt->nr; i++) {
		x += djt->hist [sn].x * w [i];
		y += djt->hist [sn].y * w [i];
		p += djt->hist [sn].p * w [i];
		sn = (sn - 1) & (NR_SAMPHISTLEN - 1);
	}

	samp->x = x >> w [NR_SAMPHISTLEN];
	samp->y = y >> w [NR_SAMPHISTLEN];
	samp->pressure = p >> w [NR_SAMPHISTLEN];
	
}
/**
 *@brief:      dejitter_read
 *@details:    ������ȥ���㷨���
 			   ����ֵƽ���˲���
 *@param[in]   struct tslib_module_info *info  
               struct ts_sample *samp          
               int nr                          
 *@param[out]  ��
 *@retval:     static
 */
static int dejitter_read(struct ts_sample *samp, int nr)
{
    struct tslib_dejitter *djt = &TsDejitter;
	struct ts_sample *s;
	int count = 0, ret;

	ret = variance_read(samp, nr);
	
	for (s = samp; ret > 0; s++, ret--) 
	{
		if (s->pressure == 0) 
		{
			/*
			 * Pen was released. Reset the state and
			 * forget all history events.
			 */
			djt->nr = 0;
			samp [count++] = *s;
                        continue;
		}

        /* If the pen moves too fast, reset the backlog. */
		if (djt->nr) 
		{
			int prev = (djt->head - 1) & (NR_SAMPHISTLEN - 1);
			if (sqr (s->x - djt->hist [prev].x) +
			    sqr (s->y - djt->hist [prev].y) > djt->delta) 
			{
                djt->nr = 0;
			}
		}

		djt->hist[djt->head].x = s->x;
		djt->hist[djt->head].y = s->y;
		djt->hist[djt->head].p = s->pressure;
		
		if (djt->nr < NR_SAMPHISTLEN)
			djt->nr++;

		/* We'll pass through the very first sample since
		 * we can't average it (no history yet).
		 */
		if (djt->nr == 1)
			samp [count] = *s;
		else 
		{
			average (djt, samp + count);
			//samp [count].tv = s->tv;
		}
		count++;
		/*
		����Ĵ����λ�������ѭ�����⣬����ֻ�е����������2��N�η��ǲſ���������
		���ַ����ĸ���ԭ����ͨ������������λ��
		*/
		djt->head = (djt->head + 1) & (NR_SAMPHISTLEN - 1);
	}

	return count;
}


static int mod_dejitter_init(void)
{
	struct tslib_dejitter *djt;

	rt_kprintf("mod_dejitter_init\r\n");

	djt = &TsDejitter;

	memset(djt, 0, sizeof(struct tslib_dejitter));
	
	djt->delta = 100;
    djt->head = 0;

	djt->delta = sqr(djt->delta);

	return 1;
}

/*---------------------------------����ӿ�-----------------------------------------------*/
/**
 *@brief:      ts_open
 *@details:    ��һ��TS�豸
 *@param[in]   const char *name  
               int nonblock      
 *@param[out]  ��
 *@retval:     struct

 */
struct tsdev *ts_open(const char *name, int nonblock)
{
	mod_dejitter_init();
	mod_variance_init();
	mod_linear_init();
	return (struct tsdev *)1;
}

int ts_read(struct tsdev *ts, struct ts_sample *samp, int nr)
{
	int result;

	result = dejitter_read(samp, nr);

	return result;

}

int ts_config(struct tsdev *ts)
{
	return 0;	
}

/*
	ֱ�Ӷ�����ӿڣ����㲻����TSLIB����
*/
int ts_read_raw(struct tsdev *ts, struct ts_sample *samp, int nr)
{
	int result = ts_input_read(samp, nr);

	return result;
}
/*
	����У׼����
*/
int ts_set_cal(calibration *CalSet)
{
	memcpy(&TsCalSet, CalSet, sizeof(calibration));		
}

