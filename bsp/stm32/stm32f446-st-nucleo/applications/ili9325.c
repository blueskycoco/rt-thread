#include <board.h>
#include "ili9325.h"
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

rt_err_t stm32_lcd_init()
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

	dpi_w(0x0001,0x0100); 
	dpi_w(0x0002,0x0700);
	dpi_w(0x0003,0x1030);

	dpi_w(0x0004,0x0000);
	dpi_w(0x0008,0x0207);
	dpi_w(0x0009,0x0000);
	dpi_w(0x000A,0x0000);
	dpi_w(0x000C,0x0000);
	dpi_w(0x000D,0x0000);
	dpi_w(0x000F,0x0000);
	rt_thread_mdelay(50);
	dpi_w(0x0007,0x0101);
	rt_thread_mdelay(50);
	dpi_w(0x0010,0x16B0);
	dpi_w(0x0011,0x0001);
	dpi_w(0x0017,0x0001);
	dpi_w(0x0012,0x0138);
	dpi_w(0x0013,0x0800);
	dpi_w(0x0029,0x0009);
	dpi_w(0x002a,0x0009);
	dpi_w(0x00a4,0x0000);

	dpi_w(0x0050,0x0000);
	dpi_w(0x0051,0x00EF);
	dpi_w(0x0052,0x0000);
	dpi_w(0x0053,0x013F);


	dpi_w(0x0060,0xA700);  								  
	dpi_w(0x0061,0x0003);
	dpi_w(0x006A,0x0000);

	dpi_w(0x0080,0x0000);
	dpi_w(0x0081,0x0000);
	dpi_w(0x0082,0x0000);
	dpi_w(0x0083,0x0000);
	dpi_w(0x0084,0x0000);
	dpi_w(0x0085,0x0000);
	dpi_w(0x0090,0x0013);
	dpi_w(0x0092,0x0000);
	dpi_w(0x0093,0x0003);
	dpi_w(0x0095,0x0110);
	dpi_w(0x0007,0x0173); 						
	rt_pin_write(CS_PIN, PIN_HIGH);
	rt_thread_mdelay(50);
}

static void set_pixel(const rt_uint16_t pixel, int x, int y)
{
	rt_pin_write(CS_PIN, PIN_LOW);
	dpi_w(0x0020, x); 						
	dpi_w(0x0021, y); 						
	dpi_w(0x0022, pixel); 						
	rt_pin_write(CS_PIN, PIN_HIGH);
}

void draw_hline(const rt_uint16_t pixel, int x1, int x2, int y)
{
	int i;
	for (i = x1; i <= x2; i++)
		set_pixel(pixel, i, y);
}

void draw_vline(const rt_uint16_t pixel, int x, int y1, int y2)
{
	int i;
	for (i = y1; i <= y2; i++)
		set_pixel(pixel, x, i);
}

void fb_clr(uint16_t color)
{
	rt_uint32_t i;

	rt_pin_write(CS_PIN, PIN_LOW);
	rt_pin_write(RS_PIN, PIN_LOW);
	rt_pin_write(RD_PIN, PIN_HIGH);
	GPIOB->ODR = 0x0022;
	rt_pin_write(WR_PIN, PIN_LOW);
	rt_pin_write(WR_PIN, PIN_HIGH);

	for( i = 0; i < 240 * 320; i++ )
	{
		rt_pin_write(RS_PIN, PIN_HIGH);
		rt_pin_write(RD_PIN, PIN_HIGH);
		GPIOB->ODR = color;
		rt_pin_write(WR_PIN, PIN_LOW);
		rt_pin_write(WR_PIN, PIN_HIGH);
	}
	rt_pin_write(CS_PIN, PIN_HIGH);
}

void clr_cross(int x, int y)
{
	draw_hline(BLACK, x - 10, x - 4, y);
	draw_hline(BLACK, x + 4, x + 10, y);
	draw_vline(BLACK, x, y - 10, y - 4);
	draw_vline(BLACK, x, y + 4, y + 10);

	draw_hline(BLACK, x - 9, x - 6, y - 9);
	draw_vline(BLACK, x - 9, y - 8, y - 6);
	draw_vline(BLACK, x - 9, y + 6, y + 9);
	draw_hline(BLACK, x - 8, x - 6, y + 9);
	draw_hline(BLACK, x + 6, x + 9, y + 9);
	draw_vline(BLACK, x + 9, y + 6, y + 8);
	draw_vline(BLACK, x + 9, y - 9, y - 6);
	draw_hline(BLACK, x + 6, x + 8, y - 9);
}
void put_cross(int x, int y)
{
	draw_hline(WHITE, x - 10, x - 4, y);
	draw_hline(WHITE, x + 4, x + 10, y);
	draw_vline(WHITE, x, y - 10, y - 4);
	draw_vline(WHITE, x, y + 4, y + 10);

	draw_hline(WHITE, x - 9, x - 6, y - 9);
	draw_vline(WHITE, x - 9, y - 8, y - 6);
	draw_vline(WHITE, x - 9, y + 6, y + 9);
	draw_hline(WHITE, x - 8, x - 6, y + 9);
	draw_hline(WHITE, x + 6, x + 9, y + 9);
	draw_vline(WHITE, x + 9, y + 6, y + 8);
	draw_vline(WHITE, x + 9, y - 9, y - 6);
	draw_hline(WHITE, x + 6, x + 8, y - 9);
}
