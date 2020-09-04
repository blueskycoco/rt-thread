#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "ili9325.h"
#include "tslib.h"

#define KEY1_PIN    GET_PIN(C, 5)
#define KEY2_PIN    GET_PIN(D, 2)

#define PEN_PIN    GET_PIN(C, 1)
#define CS_PIN    GET_PIN(C, 13)
#define MOSI_PIN    GET_PIN(C, 3)
#define MISO_PIN    GET_PIN(C, 2)
#define SCLK_PIN    GET_PIN(C, 0)
static uint16_t point[2][1024] = {0x00};
static rt_sem_t touch_sem = RT_NULL;
static rt_sem_t tslib_sem = RT_NULL;
uint8_t j = 0;

void put_char(int x, int y, int c, int colidx)
{
	int i,j,bits;
	u8* p;
	

	p = (u8*)font_vga_8x8.path;//need fix
	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits =	p[font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				drv_ILI9341_drawpoint(x + j, y + i, colidx);
			}
		}
	}
}

void put_string(int x, int y, char *s, unsigned colidx)
{
	int i;
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(x, y, *s, colidx);
}

static void stm32_udelay(rt_uint32_t us)
{
	rt_uint32_t ticks;
	rt_uint32_t told, tnow, tcnt = 0;
	rt_uint32_t reload = SysTick->LOAD;

	ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
	told = SysTick->VAL;
	while (1)
	{
		tnow = SysTick->VAL;
		if (tnow != told)
		{
			if (tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if (tcnt >= ticks)
			{
				break;
			}
		}
	}
}

static uint32_t spi_gpio_rw_ext(uint8_t cmd)
{
	int i;

	uint8_t tdata = cmd;
	uint32_t rdata = 0;

	rt_pin_write(CS_PIN, PIN_LOW);

	for (i = 0; i < 24; i++) {
		if (tdata & 0x80)
			rt_pin_write(MOSI_PIN, PIN_HIGH);
		else
			rt_pin_write(MOSI_PIN, PIN_LOW);
		rt_pin_write(SCLK_PIN, PIN_LOW);

		rdata = rdata << 1;
		if (rt_pin_read(MISO_PIN) == PIN_HIGH)
			rdata |= 0x01;

		rt_pin_write(SCLK_PIN, PIN_HIGH);
		tdata = tdata << 1;
	}

	rt_pin_write(CS_PIN, PIN_HIGH);

	return rdata;
}

static uint16_t spi_gpio_rw(uint8_t cmd)
{
	int i;

	uint8_t tdata = cmd;
	uint16_t rdata = 0;

	rt_pin_write(CS_PIN, PIN_LOW);
	stm32_udelay(10);

	for (i = 0; i < 8; i++) {
		if (tdata & 0x80)
			rt_pin_write(MOSI_PIN, PIN_HIGH);
		else
			rt_pin_write(MOSI_PIN, PIN_LOW);
		rt_pin_write(SCLK_PIN, PIN_LOW);
		stm32_udelay(10);
		rt_pin_write(SCLK_PIN, PIN_HIGH);
		stm32_udelay(10);
		tdata = tdata << 1;
	}

	for (i = 0; i < 16; i++) {
		rdata = rdata << 1;
		rt_pin_write(SCLK_PIN, PIN_LOW);
		stm32_udelay(10);
		if (rt_pin_read(MISO_PIN) == PIN_HIGH)
			rdata |= 0x01;
		rt_pin_write(SCLK_PIN, PIN_HIGH);
		stm32_udelay(10);
	}
	stm32_udelay(10);
	rt_pin_write(CS_PIN, PIN_HIGH);

	return rdata;
}
int dev_touchscreen_read(struct ts_sample *samp, int nr)
{
	int i;
    	g_need_bytes = nr;
	j = 0;
	rt_sem_take(tslib_sem, RT_WAITING_FOREVER);

	for (i=0; i<nr; i++) {
		samp->x = point[0][i];
		samp->y = point[1][i];
		samp->pressure = 200;
	}

	return nr;
}
static void touch_isr(void *parameter)
{
	rt_sem_release(touch_sem);
}

static void ads7843_handler()
{
	uint32_t x, y;
	uint8_t flag = 0;

	touch_sem = rt_sem_create("touch", 0, RT_IPC_FLAG_FIFO);
	tslib_sem = rt_sem_create("tslib", 0, RT_IPC_FLAG_FIFO);
	rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT);
	rt_pin_mode(KEY2_PIN, PIN_MODE_INPUT);
	rt_pin_mode(PEN_PIN, PIN_MODE_INPUT);
	rt_pin_mode(MISO_PIN, PIN_MODE_INPUT);
	rt_pin_mode(MOSI_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(SCLK_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(CS_PIN, PIN_MODE_OUTPUT);

	rt_pin_write(SCLK_PIN, PIN_LOW);
	rt_pin_write(CS_PIN, PIN_HIGH);
	rt_pin_write(MOSI_PIN, PIN_HIGH);
	rt_pin_write(SCLK_PIN, PIN_HIGH);

	rt_pin_attach_irq(PEN_PIN, PIN_IRQ_MODE_FALLING, touch_isr, RT_NULL);
	rt_pin_attach_irq(KEY1_PIN, PIN_IRQ_MODE_FALLING, touch_isr, RT_NULL);
	rt_pin_attach_irq(KEY2_PIN, PIN_IRQ_MODE_FALLING, touch_isr, RT_NULL);
	rt_pin_irq_enable(PEN_PIN, RT_TRUE);
	rt_pin_irq_enable(KEY1_PIN, RT_TRUE);
	rt_pin_irq_enable(KEY2_PIN, RT_TRUE);
	rt_kprintf("touch inited\r\n");

	while (1) {
		rt_sem_take(touch_sem, RT_WAITING_FOREVER);
		
		if (rt_pin_read(KEY1_PIN) == PIN_LOW)
			rt_kprintf("Key 1 pressed\r\n");
		
		if (rt_pin_read(KEY2_PIN) == PIN_LOW)
			rt_kprintf("Key 2 pressed\r\n");

		while (rt_pin_read(PEN_PIN) == PIN_LOW) {
			y = spi_gpio_rw_ext(0x90);
			x = spi_gpio_rw_ext(0xd0);
			if (j<1024) {
				point[0][j] = x;
				point[1][j] = y;
			}
			//rt_kprintf("touch %03x, %03x %d\r\n",
			//		(x<<1)>>4, (y<<1)>>4,
			//		rt_pin_read(PEN_PIN));
			j++;
			if (j > g_need_bytes)
				rt_sem_release(tslib_sem);
		}
#if 0
		if (j == 1) {
		rt_kprintf("AAA\r\n");
		if (flag == 0) {
			put_cross(50, 50);
			clr_cross(240 - 50, 50);
			clr_cross(240 - 50, 320 - 50);
			clr_cross(50,  320 - 50);
			clr_cross(240 / 2, 320 / 2);
			flag++;
		} else	if (flag == 1) {
			clr_cross(50, 50);
			put_cross(240 - 50, 50);
			clr_cross(240 - 50, 320 - 50);
			clr_cross(50,  320 - 50);
			clr_cross(240 / 2, 320 / 2);
			flag++;
		} else	if (flag == 2) {
			clr_cross(50, 50);
			clr_cross(240 - 50, 50);
			put_cross(240 - 50, 320 - 50);
			clr_cross(50,  320 - 50);
			clr_cross(240 / 2, 320 / 2);
			flag++;
		} else	if (flag == 3) {
			clr_cross(50, 50);
			clr_cross(240 - 50, 50);
			clr_cross(240 - 50, 320 - 50);
			put_cross(50,  320 - 50);
			clr_cross(240 / 2, 320 / 2);
			flag++;
		} else	if (flag == 4) {
			clr_cross(50, 50);
			clr_cross(240 - 50, 50);
			clr_cross(240 - 50, 320 - 50);
			clr_cross(50,  320 - 50);
			put_cross(240 / 2, 320 / 2);
			flag = 0;
		}
		rt_kprintf("BBB\r\n");
#endif
		}
	}
}

void ads7843_init()
{
	rt_thread_t tid = rt_thread_create("touch", ads7843_handler, RT_NULL,
			2048, 28, 20);
	rt_thread_startup(tid);
}
