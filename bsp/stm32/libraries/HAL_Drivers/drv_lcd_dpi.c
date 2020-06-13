/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-08     zylx         first version
 */

#include <board.h>

#ifdef BSP_USING_LCD_DPI
#include <string.h>
#include <lcd_port.h>
//#define DRV_DEBUG
#define LOG_TAG             "drv.lcd"
#include <drv_log.h>

#define CS_PIN    GET_PIN(C, 9)
#define RS_PIN    GET_PIN(C, 8)
#define WR_PIN    GET_PIN(C, 7)
#define RD_PIN    GET_PIN(C, 6)

#define FB_0	GET_PIN(B, 0)
#define FB_1	GET_PIN(B, 1)
#define FB_2	GET_PIN(B, 2)
#define FB_3	GET_PIN(B, 3)
#define FB_4	GET_PIN(B, 4)
#define FB_5	GET_PIN(B, 5)
#define FB_6	GET_PIN(B, 6)
#define FB_7	GET_PIN(B, 7)
#define FB_8	GET_PIN(B, 8)
#define FB_9	GET_PIN(B, 9)
#define FB_10	GET_PIN(B, 10)
#define FB_11	GET_PIN(B, 11)
#define FB_12	GET_PIN(B, 12)
#define FB_13	GET_PIN(B, 13)
#define FB_14	GET_PIN(B, 14)
#define FB_15	GET_PIN(B, 15)

#define LCD_DEVICE(dev)     (struct drv_lcd_device*)(dev)

struct drv_lcd_device
{
	struct rt_device parent;

	struct rt_device_graphic_info lcd_info;
};

struct drv_lcd_device _lcd;

static rt_err_t drv_lcd_init(struct rt_device *device)
{
	struct drv_lcd_device *lcd = LCD_DEVICE(device);
	/* nothing, right now */
	lcd = lcd;
	return RT_EOK;
}

static rt_err_t drv_lcd_control(struct rt_device *device, int cmd, void *args)
{
	struct drv_lcd_device *lcd = LCD_DEVICE(device);

	switch (cmd)
	{
		case RTGRAPHIC_CTRL_GET_INFO:
			{
				struct rt_device_graphic_info *info = (struct rt_device_graphic_info *)args;

				RT_ASSERT(info != RT_NULL);
				info->pixel_format  = lcd->lcd_info.pixel_format;
				info->bits_per_pixel = 16;
				info->width         = lcd->lcd_info.width;
				info->height        = lcd->lcd_info.height;
				info->framebuffer   = lcd->lcd_info.framebuffer;
			}
			break;
	}

	return RT_EOK;
}
void dpi_w(rt_uint16_t cmd, rt_uint16_t data)
{
	rt_pin_write(RS_PIN, PIN_LOW);
	rt_pin_write(RD_PIN, PIN_HIGH);
	GPIOB->ODR = cmd;
	rt_pin_write(WR_PIN, PIN_LOW);
	rt_pin_write(WR_PIN, PIN_HIGH);

	rt_pin_write(RS_PIN, PIN_HIGH);
	rt_pin_write(RD_PIN, PIN_HIGH);
	GPIOB->ODR = data;
	rt_pin_write(WR_PIN, PIN_LOW);
	rt_pin_write(WR_PIN, PIN_HIGH);
}
rt_uint16_t dpi_r(rt_uint16_t cmd)
{
	rt_uint16_t color;
	rt_uint32_t mode = GPIOB->MODER;

	rt_pin_write(RS_PIN, PIN_LOW);
	rt_pin_write(RD_PIN, PIN_HIGH);
	GPIOB->ODR = cmd;
	rt_pin_write(WR_PIN, PIN_LOW);
	rt_pin_write(WR_PIN, PIN_HIGH);
	rt_pin_write(RS_PIN, PIN_HIGH);

	/*rt_pin_mode(FB_0, PIN_MODE_INPUT);
	rt_pin_mode(FB_1, PIN_MODE_INPUT);
	rt_pin_mode(FB_2, PIN_MODE_INPUT);
	rt_pin_mode(FB_3, PIN_MODE_INPUT);
	rt_pin_mode(FB_4, PIN_MODE_INPUT);
	rt_pin_mode(FB_5, PIN_MODE_INPUT);
	rt_pin_mode(FB_6, PIN_MODE_INPUT);
	rt_pin_mode(FB_7, PIN_MODE_INPUT);
	rt_pin_mode(FB_8, PIN_MODE_INPUT);
	rt_pin_mode(FB_9, PIN_MODE_INPUT);
	rt_pin_mode(FB_10, PIN_MODE_INPUT);
	rt_pin_mode(FB_11, PIN_MODE_INPUT);
	rt_pin_mode(FB_12, PIN_MODE_INPUT);
	rt_pin_mode(FB_13, PIN_MODE_INPUT);
	rt_pin_mode(FB_14, PIN_MODE_INPUT);
	rt_pin_mode(FB_15, PIN_MODE_INPUT);
	*/
	GPIOB->MODER = 0x00;

	rt_pin_write(RD_PIN, PIN_LOW);
	color = GPIOB->IDR & 0xffff;
	rt_pin_write(RD_PIN, PIN_HIGH);
	
	GPIOB->MODER = mode;
	/*rt_pin_mode(FB_0, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_1, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_2, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_3, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_4, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_5, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_6, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_7, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_8, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_9, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_10, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_11, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_12, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_13, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_14, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_15, PIN_MODE_OUTPUT);*/
}
rt_err_t stm32_lcd_init(struct drv_lcd_device *lcd)
{
	rt_pin_mode(CS_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RS_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(WR_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RD_PIN, PIN_MODE_OUTPUT);

	rt_pin_mode(FB_0, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_1, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_2, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_3, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_4, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_5, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_6, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_7, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_8, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_9, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_10, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_11, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_12, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_13, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_14, PIN_MODE_OUTPUT);
	rt_pin_mode(FB_15, PIN_MODE_OUTPUT);

	rt_pin_write(CS_PIN, PIN_LOW);

	dpi_w(0x0001,0x0100);	  /* Driver Output Contral Register */ 
	dpi_w(0x0002,0x0700);      /* LCD Driving Waveform Contral */
	dpi_w(0x0003,0x1030);	  /* Entry Mode设置 */

	dpi_w(0x0004,0x0000);	  /* Scalling Control register */
	dpi_w(0x0008,0x0207);	  /* Display Control 2 */
	dpi_w(0x0009,0x0000);	  /* Display Control 3 */
	dpi_w(0x000A,0x0000);	  /* Frame Cycle Control */
	dpi_w(0x000C,0x0000);	  /* External Display Interface Control 1 */
	dpi_w(0x000D,0x0000);      /* Frame Maker Position */
	dpi_w(0x000F,0x0000);	  /* External Display Interface Control 2 */
	rt_thread_mdelay(50);
	dpi_w(0x0007,0x0101);	  /* Display Control */
	rt_thread_mdelay(50);
	dpi_w(0x0010,0x16B0);      /* Power Control 1 */
	dpi_w(0x0011,0x0001);      /* Power Control 2 */
	dpi_w(0x0017,0x0001);      /* Power Control 3 */
	dpi_w(0x0012,0x0138);      /* Power Control 4 */
	dpi_w(0x0013,0x0800);      /* Power Control 5 */
	dpi_w(0x0029,0x0009);	  /* NVM read data 2 */
	dpi_w(0x002a,0x0009);	  /* NVM read data 3 */
	dpi_w(0x00a4,0x0000);

	dpi_w(0x0050,0x0000);	  /* 设置操作窗口的X轴开始列 */
	dpi_w(0x0051,0x00EF);	  /* 设置操作窗口的X轴结束列 */
	dpi_w(0x0052,0x0000);	  /* 设置操作窗口的Y轴开始行 */
	dpi_w(0x0053,0x013F);	  /* 设置操作窗口的Y轴结束行 */

	/* 设置屏幕的点数以及扫描的起始行
	 * */	   
	dpi_w(0x0060,0xA700);	  /* Driver Output Control 需要竖立屏幕改为2700 */  								  
	dpi_w(0x0061,0x0003);	  /* Driver Output Control */
	dpi_w(0x006A,0x0000);	  /* Vertical Scroll Control */

	dpi_w(0x0080,0x0000);	  /* Display Position – Partial Display 1 */
	dpi_w(0x0081,0x0000);	  /* RAM Address Start – Partial Display 1 */
	dpi_w(0x0082,0x0000);	  /* RAM address End - Partial Display 1 */
	dpi_w(0x0083,0x0000);	  /* Display Position – Partial Display 2 */
	dpi_w(0x0084,0x0000);	  /* RAM Address Start – Partial Display 2 */
	dpi_w(0x0085,0x0000);	  /* RAM address End – Partail Display2 */
	dpi_w(0x0090,0x0013);	  /* Frame Cycle Control */
	dpi_w(0x0092,0x0000); 	  /* Panel Interface Control 2 */
	dpi_w(0x0093,0x0003);	  /* Panel Interface control 3 */
	dpi_w(0x0095,0x0110);	  /* Frame Cycle Control */
	dpi_w(0x0007,0x0173); 						
	rt_thread_mdelay(50);   /* delay 50 ms */
}

static void set_pixel(const char *pixel, int x, int y)
{
	dpi_w(0x0020, x); 						
	dpi_w(0x0021, y); 						
	dpi_w(0x0022, *(rt_uint16_t *)pixel); 						
}

static void draw_hline(const char *pixel, int x1, int x2, int y)
{
	int i;
	for (i = x1; i <= x2; i++)
		set_pixel(pixel, i, y);
}

static void draw_vline(const char *pixel, int x, int y1, int y2)
{
	int i;
	for (i = y1; i <= y2; i++)
		set_pixel(pixel, x, i);
}

static void get_pixel(char *pixel, int x, int y)
{
	rt_uint16_t *color = (rt_uint16_t *)pixel;

	dpi_w(0x0020, x); 						
	dpi_w(0x0021, y); 						
	*color = dpi_r(0x0022);
}

void fb_clr(uint16_t color)
{
	rt_uint32_t i;

	rt_pin_write(RS_PIN, PIN_LOW);
	rt_pin_write(RD_PIN, PIN_HIGH);
	GPIOB->ODR = 0x0022;
	rt_pin_write(WR_PIN, PIN_LOW);
	rt_pin_write(WR_PIN, PIN_HIGH);
	
	for( i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++ )
	{
		rt_pin_write(RS_PIN, PIN_HIGH);
		rt_pin_write(RD_PIN, PIN_HIGH);
		GPIOB->ODR = color;
		rt_pin_write(WR_PIN, PIN_LOW);
		rt_pin_write(WR_PIN, PIN_HIGH);
	}
}

#if defined(LCD_BACKLIGHT_USING_PWM)
void turn_on_lcd_backlight(void)
{
	struct rt_device_pwm *pwm_dev;

	/* turn on the LCD backlight */
	pwm_dev = (struct rt_device_pwm *)rt_device_find(LCD_PWM_DEV_NAME);
	/* pwm frequency:100K = 10000ns */
	rt_pwm_set(pwm_dev, LCD_PWM_DEV_CHANNEL, 10000, 10000);
	rt_pwm_enable(pwm_dev, LCD_PWM_DEV_CHANNEL);
}
#elif defined(LCD_BACKLIGHT_USING_GPIO)
void turn_on_lcd_backlight(void)
{
	rt_pin_mode(LCD_BL_GPIO_NUM, PIN_MODE_OUTPUT);
	rt_pin_mode(LCD_DISP_GPIO_NUM, PIN_MODE_OUTPUT);

	rt_pin_write(LCD_DISP_GPIO_NUM, PIN_HIGH);
	rt_pin_write(LCD_BL_GPIO_NUM, PIN_HIGH);
}
#else
void turn_on_lcd_backlight(void)
{

}
#endif

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops lcd_ops =
{
	drv_lcd_init,
	RT_NULL,
	RT_NULL,
	RT_NULL,
	RT_NULL,
	drv_lcd_control
};
#endif

struct rt_device_graphic_ops dpi_lcd_ops =
{
	set_pixel,
	get_pixel,
	draw_hline,
	draw_vline,
	RT_NULL,
};

int drv_lcd_hw_init(void)
{
	rt_err_t result = RT_EOK;
	struct rt_device *device = &_lcd.parent;

	/* memset _lcd to zero */
	memset(&_lcd, 0x00, sizeof(_lcd));

	/* config LCD dev info */
	_lcd.lcd_info.height = LCD_HEIGHT;
	_lcd.lcd_info.width = LCD_WIDTH;
	_lcd.lcd_info.bits_per_pixel = LCD_BITS_PER_PIXEL;
	_lcd.lcd_info.pixel_format = LCD_PIXEL_FORMAT;

	device->type    = RT_Device_Class_Graphic;
#ifdef RT_USING_DEVICE_OPS
	device->ops     = &lcd_ops;
#else
	device->init    = drv_lcd_init;
	device->control = drv_lcd_control;
#endif
	device->user_data = &dpi_lcd_ops;

	/* register lcd device */
	rt_device_register(device, "lcd", RT_DEVICE_FLAG_RDWR);

	/* init stm32 LTDC */
	if (stm32_lcd_init(&_lcd) != RT_EOK)
	{
		result = -RT_ERROR;
		goto __exit;
	}
	else
	{
		rt_uint8_t ch[2] = {0xff, 0x00};
		rt_uint8_t ch2[2] = {0x00};
		fb_clr(0x0000);
		draw_hline(ch, 78, 220, 99);
		ch[0] = 0x67; ch[1] = 0x32;
		draw_vline(ch, 78, 0, 320);
		ch[0] = 0x09; ch[1] = 0xAB;
		set_pixel(ch, 99, 300);
		get_pixel(ch2, 99, 300);
		rt_kprintf("ch2 is 0x%x, 0x%x\n", ch2[0], ch2[1]);
		turn_on_lcd_backlight();
	}

__exit:
	if (result != RT_EOK)
	{
	}
	return result;
}
INIT_DEVICE_EXPORT(drv_lcd_hw_init);

#endif /* BSP_USING_LCD */
