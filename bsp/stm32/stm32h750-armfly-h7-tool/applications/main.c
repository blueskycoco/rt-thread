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
#include <fal.h>
#include "stm32h7xx_hal.h"
#include "w25qxx.h"
#include "md5.h"
/* defined the LED0 pin: PI8 */
#define LED0_PIN    GET_PIN(I, 8)
#define LED1_PIN    GET_PIN(C, 15)
#define VECT_TAB_OFFSET      0x00000000UL
#define UBOOT_ADDRESS  (uint32_t)0x90000000
#define KERNEL_ADDRESS  (uint32_t)0x90080000
static rt_bool_t ota_kernel = RT_FALSE;
static rt_uint32_t ofs = 0;
static rt_uint32_t file_len = 0;
static fal_partition_t qspi_part = RT_NULL;
rt_uint32_t down_addr = UBOOT_ADDRESS;
typedef void (*pFunction)(void);
static rt_err_t rym_download_file(rt_device_t idev);
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
extern void release_resource();
void jump(uint32_t addr)
{
	if (addr == UBOOT_ADDRESS) {
    	if (qspi_part == RT_NULL) {
    	W25QXX_ExitQPIMode();
    	W25QXX_Reset();
    	W25QXX_Init();
    	W25Q_Memory_Mapped_Enable();
		}
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
  		
  		//HAL_SDRAM_MspDeInit(RT_NULL);
	} else {
		//rt_kprintf("to release resource\r\n");
		release_resource();
		//rt_thread_mdelay(100);
	}
    	rt_hw_interrupt_disable();
#if 0
	if (((*(__IO uint32_t*)(addr + 4)) & 0xFF000000 ) == addr)
		rt_kprintf("addr verify ok\r\n");
	else	
		rt_kprintf("addr verify failed %x %x\r\n", addr, ((*(__IO uint32_t*)(addr + 4)) & 0xFF000000 ));

	uint32_t ret = ((*(__IO uint32_t*)addr) & 0x2FFE0000 );
	if (ret == 0x24000000)
		rt_kprintf("instr verify ok\r\n");
	else
		rt_kprintf("instr verify failed, %x\r\n", ret);
#endif
   	//SCB_InvalidateICache();
   	HAL_MPU_Disable();
   	SCB_DisableICache();
    SCB_DisableDCache();
    SysTick->CTRL = 0;
    //__set_CONTROL(0);
    __set_MSP(*(__IO uint32_t *)addr);
    JumpToApplication = (pFunction)(*(__IO uint32_t *)(addr + 4));
    SCB->VTOR = addr;
#if 0
    __set_PSP(*(volatile unsigned int*) addr);
    __set_CONTROL(0);
    __set_MSP(*(volatile unsigned int*) addr);
    //__set_FAULTMASK(1);
#endif
    rt_kprintf("before jump, %x\r\n", addr);
    JumpToApplication();
}
int main(void)
{
	int count = 1;
	/* set LED0 pin mode to output */
    	rt_kprintf("BOOT\r\n"); 
	rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
	//vcom_init();
	//jump(UBOOT_ADDRESS);
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
static void md5_sdram(uint8_t argc, char **argv)
{
	MD5_CTX md5;
    int i;
	uint8_t decrypt[16] = {0};
	MD5Init(&md5);
	MD5Update(&md5, down_addr, atoi(argv[1]));
	MD5Final(&md5, decrypt);
	for (i=0; i<16; i++)
		rt_kprintf("%02x", decrypt[i]);
	rt_kprintf("\r\n");
}
FINSH_FUNCTION_EXPORT_ALIAS(md5_sdram, __cmd_md5, md5 sdram);
static void boot(uint8_t argc, char **argv)
{
    jump(UBOOT_ADDRESS);
}
FINSH_FUNCTION_EXPORT_ALIAS(boot, __cmd_boot, Jump to App);
static void go(uint8_t argc, char **argv)
{
    jump(down_addr);
    //jump(0x24000000);
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
    
    file_len = atoi(1 + (const char *)buf + rt_strnlen((const char *)buf, len - 1));
    if (file_len == 0)
        file_len = -1;
	rt_kprintf("file name %s, len %d\r\n", cctx->fpath, file_len);
	ofs = 0;
	if (ota_kernel)
	qspi_part = fal_partition_find("linux");
	else
	qspi_part = fal_partition_find("u-boot");
	if (qspi_part == RT_NULL)
		rt_kprintf("can't find qspi part\r\n");
	else {
		if ((fal_partition_erase(qspi_part, 0, file_len) < 0))
			rt_kprintf("erase qspi part failed\r\n");
		else
			rt_kprintf("erase qspi flash %d bytes done\r\n", file_len);
	}
	return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_data(
    struct rym_ctx *ctx,
    rt_uint8_t *buf,
    rt_size_t len)
{
	uint8_t verify[1024] = {0};
	struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    //rt_memcpy((__IO uint8_t *)(down_addr + ofs), (uint8_t *)buf, len);
    if (fal_partition_write(qspi_part, ofs, buf, len) < 0) {
		rt_kprintf("write qspi flash %d failed\r\n", ofs);
	}
    if (fal_partition_read(qspi_part, ofs, verify, len) < 0) {
		rt_kprintf("read qspi flash %d failed\r\n", ofs);
	}
	if (memcmp(verify, buf, len) != 0)
		rt_kprintf("write to %d, read back different\r\n", ofs);
    ofs += len;
    return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_end(
    struct rym_ctx *ctx,
    rt_uint8_t *buf,
    rt_size_t len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;
	uint8_t decrypt[16] = {0};
	MD5_CTX md5;
    int i;
    cctx->fd = -1;
	rt_kprintf("rcv file finish\r\nmd5: ");
#if 1
    W25QXX_ExitQPIMode();
    W25QXX_Reset();
    W25QXX_Init();
    W25Q_Memory_Mapped_Enable();
#endif
	MD5Init(&md5);
	MD5Update(&md5, down_addr, file_len);
	MD5Final(&md5, decrypt);
	for (i=0; i<16; i++)
		rt_kprintf("%02x", decrypt[i]);
	rt_kprintf("\r\n");
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

    if (argc > 1 && strcmp(argv[1], "linux") == 0)
    {
    	    ota_kernel = RT_TRUE;
    	    down_addr = KERNEL_ADDRESS;
    } else {
    	    ota_kernel = RT_FALSE;
		down_addr = UBOOT_ADDRESS;
    }
    //dev = rt_console_get_device();
    dev = rt_device_find("vcom");
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
