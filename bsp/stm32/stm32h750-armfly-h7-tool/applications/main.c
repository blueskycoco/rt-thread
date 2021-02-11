/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-25     zylx         first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <finsh.h>
#include <drv_common.h>
#include <ymodem.h>
#include <sdram_port.h>
#include "stm32h7xx_hal.h"
#include "w25qxx.h"
/* defined the LED0 pin: PI8 */
#define LED0_PIN    GET_PIN(I, 8)
#define LED1_PIN    GET_PIN(C, 15)
#define VECT_TAB_OFFSET      0x00000000UL
#define APPLICATION_ADDRESS  (uint32_t)0x90000000
#define SDRAM_ADDRESS  (uint32_t)0x60000000
static rt_uint32_t ofs = 0;
typedef void (*pFunction)(void);
pFunction JumpToApplication;
int vcom_init(void)
{
	/* set console */
	rt_console_set_device("vcom");

#if defined(RT_USING_POSIX)    
	/* backup flag */
	dev_old_flag = ioctl(libc_stdio_get_console(), F_GETFL, (void *) RT_NULL);
	/* add non-block flag */
	ioctl(libc_stdio_get_console(), F_SETFL, (void *) (dev_old_flag | O_NONBLOCK));
	/* set tcp shell device for console */
	libc_stdio_set_console("vcom", O_RDWR);

	/* resume finsh thread, make sure it will unblock from
	 * last device receive */
	rt_thread_t tid = rt_thread_find(FINSH_THREAD_NAME);
	if (tid)
	{
		rt_thread_resume(tid);
		rt_schedule();
	}
#else
	/* set finsh device */
	finsh_set_device("vcom");
#endif /* RT_USING_POSIX */

	return 0;
}
void jump(uint32_t addr)
{
	if (addr == APPLICATION_ADDRESS) {
    	rt_kprintf("boot to qspi\r\n");
    	W25QXX_Init();
    	W25Q_Memory_Mapped_Enable();
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
  		//HAL_SDRAM_MspDeInit(RT_NULL);
    	//SCB_DisableICache();
    	//SCB_DisableDCache();
	} else {
		rt_kprintf("boot to sdram\r\n");
	}
	rt_hw_interrupt_disable();

    SysTick->CTRL = 0;

    JumpToApplication = (pFunction)(*(__IO uint32_t *)(addr + 4));
    __set_MSP(*(__IO uint32_t *)addr);

    JumpToApplication();
}
int main(void)
{
	int count = 1;
	/* set LED0 pin mode to output */
    
	rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	//vcom_init();
	//jump();
	while (count++)
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
#ifdef RT_USING_FINSH
#include <finsh.h>
static void boot(uint8_t argc, char **argv)
{
    jump(APPLICATION_ADDRESS);
}
FINSH_FUNCTION_EXPORT_ALIAS(boot, __cmd_boot, Jump to App);
static void go(uint8_t argc, char **argv)
{
    jump(SDRAM_ADDRESS);
}
FINSH_FUNCTION_EXPORT_ALIAS(go, __cmd_go, Jump to SDRAM);
struct custom_ctx
{
    struct rym_ctx parent;
    int fd;
    int flen;
    char fpath[256];
};

static enum rym_code _rym_recv_begin(
    struct rym_ctx *ctx,
    rt_uint8_t *buf,
    rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    cctx->fpath[0] = '/';
    rt_strncpy(&(cctx->fpath[1]), (const char *)buf, len - 1);
#if 0
	cctx->fd = open(cctx->fpath, O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (cctx->fd < 0)
    {
        rt_err_t err = rt_get_errno();
        rt_kprintf("error creating file: %d\n", err);
        return RYM_CODE_CAN;
    }
#endif
    cctx->flen = atoi(1 + (const char *)buf + rt_strnlen((const char *)buf, len - 1));
    if (cctx->flen == 0)
        cctx->flen = -1;
	rt_kprintf("file name %s, len %d\r\n", cctx->fpath, cctx->flen);
	ofs = 0;
	return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_data(
    struct rym_ctx *ctx,
    rt_uint8_t *buf,
    rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    //RT_ASSERT(cctx->fd >= 0);
    if (cctx->flen == -1)
    {
        //write(cctx->fd, buf, len);
    }
    else
    {
        int wlen = len > cctx->flen ? cctx->flen : len;
        //write(cctx->fd, buf, wlen);
        cctx->flen -= wlen;
    }
	
    rt_memcpy((__IO uint16_t *)(SDRAM_BANK_ADDR + ofs), (uint16_t *)buf, len/2);
    ofs += (len/2);
	//rt_kprintf("rcv len %d\r\n", len);
    return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_end(
    struct rym_ctx *ctx,
    rt_uint8_t *buf,
    rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    //RT_ASSERT(cctx->fd >= 0);
    //close(cctx->fd);
    cctx->fd = -1;
	rt_kprintf("rcv file finish\r\n");
    return RYM_CODE_ACK;
}
static rt_err_t rym_download_file(rt_device_t idev)
{
    rt_err_t res;
    struct custom_ctx *ctx = rt_calloc(1, sizeof(*ctx));

    if (!ctx)
    {
        rt_kprintf("rt_malloc failed\n");
        return RT_ENOMEM;
    }
    ctx->fd = -1;
    RT_ASSERT(idev);
    res = rym_recv_on_device(&ctx->parent, idev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                             _rym_recv_begin, _rym_recv_data, _rym_recv_end, 1000);
    rt_free(ctx);

    return res;
}
static rt_err_t ry(uint8_t argc, char **argv)
{
    rt_err_t res;
    rt_device_t dev;

    if (argc > 1)
        dev = rt_device_find(argv[1]);
    else
        dev = rt_console_get_device();
    if (!dev)
    {
        rt_kprintf("could not find device.\n");
        return -RT_ERROR;
    }
    res = rym_download_file(dev);

    return res;
}
MSH_CMD_EXPORT(ry, YMODEM Receive e.g: ry [uart0] default by console.);
#endif /* RT_USING_FINSH */
