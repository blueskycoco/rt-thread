#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define PEN_PIN    GET_PIN(C, 1)
#define CS_PIN    GET_PIN(C, 13)
#define MOSI_PIN    GET_PIN(C, 3)
#define MISO_PIN    GET_PIN(C, 2)
#define SCLK_PIN    GET_PIN(C, 0)

static rt_sem_t touch_sem = RT_NULL;

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

static void touch_isr(void *parameter)
{
	rt_sem_release(touch_sem);
}

static void ads7843_handler()
{
	uint32_t x, y;

	touch_sem = rt_sem_create("touch", 0, RT_IPC_FLAG_FIFO);
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
	rt_pin_irq_enable(PEN_PIN, RT_TRUE);
	rt_kprintf("touch inited\r\n");

	while (1) {
		rt_sem_take(touch_sem, RT_WAITING_FOREVER);
		while (rt_pin_read(PEN_PIN) == PIN_LOW) {
			y = spi_gpio_rw_ext(0x90);
			x = spi_gpio_rw_ext(0xd0);
			rt_kprintf("touch %03x, %03x %d\r\n",
					(x<<1)>>4, (y<<1)>>4,
					rt_pin_read(PEN_PIN));
		}
	}
}

void ads7843_init()
{
	rt_thread_t tid = rt_thread_create("touch", ads7843_handler, RT_NULL,
			2048, 28, 20);
	rt_thread_startup(tid);
}
