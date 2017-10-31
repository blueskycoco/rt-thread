/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup.
 */
	 
#include <rtthread.h>
#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <spi_flash.h>
#include <spi_flash_sfud.h>
#include "drv_qspi.h"
#include "drv_sdio.h"
#include "drv_afec.h"
#endif 
static struct rt_semaphore rx_sem_uart2;
static struct rt_semaphore tx_sem_uart2;
rt_device_t dev_uart2 = RT_NULL;

static rt_err_t rx_ind_uart2(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem_uart2);
    return RT_EOK;
}
static rt_err_t tx_ind_uart2(rt_device_t dev, void *buffer)
{
    rt_sem_release(&tx_sem_uart2);
    return RT_EOK;
}
static void uart2_rx_int_normal_tx(void* parameter)
{
	int len = 0;
	rt_uint8_t buf[256] = {0};
	rt_kprintf("uart2_rx_int_normal_tx start\r\n");
	while (1)
	{	
		if (rt_sem_take(&rx_sem_uart2, RT_WAITING_FOREVER) != RT_EOK) 
		{
			rt_kprintf("no message from uart2\n");
			continue;
		}
		len=rt_device_read(dev_uart2, 0, buf, 256);
		rt_device_write(dev_uart2,0,buf,len);
	}
}
static void uart2_rx_int_dma_tx(void* parameter)
{
	rt_kprintf("uart2_rx_int_dma_tx start\r\n");
	int len = 0;
	rt_uint8_t buf[256] = {0};
	while (1)
	{	
		if (rt_sem_take(&rx_sem_uart2, RT_WAITING_FOREVER) != RT_EOK) 
		{
			rt_kprintf("no message from uart2\n");
			continue;
		}
		
		len=rt_device_read(dev_uart2, 0, buf, 256);
		rt_device_write(dev_uart2,0,buf,len);
		rt_sem_take(&tx_sem_uart2, RT_WAITING_FOREVER);
	}
}
static void uart2_rx_dma_dma_tx(void* parameter)
{
	rt_kprintf("uart2_rx_dma_dma_tx start\r\n");
	int i=0;
	__attribute__((__aligned__(32))) rt_uint8_t buf[256] = {0};
	while (1)
	{	
		rt_memset(buf,0,256);
		rt_device_read(dev_uart2, 0, buf, 256);
		rt_sem_take(&rx_sem_uart2, RT_WAITING_FOREVER);
		i=0;
		while(buf[i]!=0)
			i++;
		rt_device_write(dev_uart2,0,buf,i-1);
		rt_sem_take(&tx_sem_uart2, RT_WAITING_FOREVER);
	}
}
static void uart2_rx_dma_normal_tx(void* parameter)
{
	rt_kprintf("uart2_rx_dma_normal_tx start\r\n");
	int i=0;
	__attribute__((__aligned__(32))) rt_uint8_t buf[256] = {0};
	while (1)
	{	
		rt_memset(buf,0,256);
		rt_device_read(dev_uart2, 0, buf, 256);
		rt_sem_take(&rx_sem_uart2, RT_WAITING_FOREVER);
		i=0;
		while(buf[i]!=0)
			i++;
		rt_device_write(dev_uart2,0,buf,i-1);
	}
}

void test_uart2_rx_dma_normal_tx(void)
{
	dev_uart2 = rt_device_find("uart1");
	if (dev_uart2 == RT_NULL) {
		rt_kprintf("can not find uart2 \n");
		return ;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
		config.baud_rate=115200;
		config.parity=PARITY_NONE;
		config.bufsz = 0;
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CONFIG, &config);
	if (rt_device_open(dev_uart2, 
		RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX 
		) == RT_EOK)
	{
		rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CLR_INT, (void *)RT_DEVICE_FLAG_INT_RX);
		rt_sem_init(&rx_sem_uart2, "uart2_sem", 0, 0);
		rt_device_set_rx_indicate(dev_uart2, rx_ind_uart2);
		rt_thread_startup(rt_thread_create("uart2_rx",
			uart2_rx_dma_normal_tx, RT_NULL,2048, 20, 10));
	}
}
void test_uart2_rx_dma_dma_tx(void)
{
	dev_uart2 = rt_device_find("uart1");
	if (dev_uart2 == RT_NULL) {
		rt_kprintf("can not find uart2 \n");
		return ;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
		config.baud_rate=115200;
		config.parity=PARITY_NONE;
		config.bufsz = 0;
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CONFIG, &config);
	if (rt_device_open(dev_uart2, 
		RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX 
		) == RT_EOK)
	{
		rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CLR_INT, (void *)RT_DEVICE_FLAG_INT_RX);
		rt_sem_init(&rx_sem_uart2, "uart2_sem", 0, 0);
		rt_device_set_rx_indicate(dev_uart2, rx_ind_uart2);
		rt_sem_init(&tx_sem_uart2, "uart2_tsem", 0, 0);
		rt_device_set_tx_complete(dev_uart2, tx_ind_uart2);
		rt_thread_startup(rt_thread_create("uart2_rx",
			uart2_rx_dma_dma_tx, RT_NULL,2048, 20, 10));
	}
}
void test_uart2_rx_int_dma_tx(void)
{
	dev_uart2 = rt_device_find("uart1");
	if (dev_uart2 == RT_NULL) {
		rt_kprintf("can not find uart2 \n");
		return ;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
	config.baud_rate=115200;
	config.parity=PARITY_NONE;
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CONFIG, &config);
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_RX);
	if (rt_device_open(dev_uart2, 
		RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX 
		) == RT_EOK)
	{		
		rt_sem_init(&rx_sem_uart2, "uart2_sem", 0, 0);
		rt_device_set_rx_indicate(dev_uart2, rx_ind_uart2);
		rt_sem_init(&tx_sem_uart2, "uart2_tsem", 0, 0);
		rt_device_set_tx_complete(dev_uart2, tx_ind_uart2);
		rt_thread_startup(rt_thread_create("uart2_rx",
			uart2_rx_int_dma_tx, RT_NULL,2048, 20, 10));
	}
}
void test_uart2_rx_int_normal_tx(void)
{
	dev_uart2 = rt_device_find("uart1");
	if (dev_uart2 == RT_NULL) {
		rt_kprintf("can not find uart2 \n");
		return ;
	}
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
	config.baud_rate=115200;
	config.parity=PARITY_NONE;
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_CONFIG, &config);
	rt_device_control(RT_DEVICE(dev_uart2), RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_RX);
	if (rt_device_open(dev_uart2, 
		RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX  
		) == RT_EOK)
	{		
		rt_sem_init(&rx_sem_uart2, "uart2_sem", 0, 0);
		rt_device_set_rx_indicate(dev_uart2, rx_ind_uart2);
		rt_thread_startup(rt_thread_create("uart2_rx",
			uart2_rx_int_normal_tx, RT_NULL,2048, 20, 10));
	}
}
void mnt_init(void)
{

	if (RT_EOK != rt_hw_sdio_init())
		return ;
	//rt_thread_delay(RT_TICK_PER_SECOND * 1);

	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/sd", "elm", 0, 0) == 0)
	{
		rt_kprintf("SD File System initialized!\n");
	}
	else
	{
		rt_kprintf("SD File System initialzation failed!\n");
	}
	/*int fd;

	  fd = open("/1.txt", O_RDWR | O_APPEND | O_CREAT, 0);
	  if (fd >= 0)
	  {
	  write (fd, "1234", 4);
	  close(fd);
	  }
	  else
	  {
	  rt_kprintf("open file:/1.txt failed!\n");
	  }*/
}
extern char yfile[256];
int low_level_init(void)
{
#ifdef RT_USING_DFS
	rt_hw_spi_init();	
	rt_sfud_flash_probe("flash", "spi10");	
	if (dfs_mount("flash", "/", "elm", 0, 0) == 0)
	{
		DIR *dir = RT_NULL;
		rt_kprintf("root file system initialized!\n");
		if ((dir = opendir("/sd"))==RT_NULL)
			mkdir("/sd",0);
		else
			closedir(dir);
	}
	else
	{
		rt_kprintf("root file system failed %d!\n", rt_get_errno());
	}
//	mnt_init();
	

#endif

	return 0;
}

int main(void)
{
#if 0
	/* put user application code here */
	dev_usart1 = rt_device_find("uart1");

	if (dev_usart1 == RT_NULL) {
		rt_kprintf("can not find usart1 \n");
		return 0;
	}
	//struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
	//config.bufsz = 0;
	//rt_device_control(RT_DEVICE(dev_usart1), RT_DEVICE_CTRL_CONFIG, &config);
	if (rt_device_open(dev_usart1, 
				RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX
				) == RT_EOK)
	{
		rt_sem_init(&rx_sem, "usart1_rsem", 0, 0);
		//rt_sem_init(&tx_sem, "usart1_tsem", 0, 0);
		rt_device_set_rx_indicate(dev_usart1, rx_ind);
		//rt_device_set_tx_complete(dev_usart1,tx_ind);
		rt_thread_startup(rt_thread_create("usart1_rx",
					usart1_rx, RT_NULL,2048, 20, 10));
	}
#endif
	//test_uart2_rx_int_normal_tx();
	//test_uart2_rx_int_dma_tx();
	//test_uart2_rx_dma_dma_tx();
	//test_uart2_rx_dma_normal_tx();
	return 0;
}
INIT_ENV_EXPORT(low_level_init);
int download_file(void)
{

	rt_device_t dev = rt_device_find("uart1");
	if (!dev)
	{
		rt_kprintf("could not find device:uart1\n");
		return -RT_ERROR;
	}

	rym_write_to_file(dev);
	msh_exec(yfile,strlen(yfile));
	return 0;
}
//INIT_APP_EXPORT(download_file);
#ifdef RT_USING_FINSH
#ifdef FINSH_USING_MSH
#include <finsh.h>

#ifdef DFS_USING_WORKDIR
int cmd_exec(int argc, char **argv)
{
	if (argc == 2)
	{
		msh_exec(argv[1],strlen(argv[1]));
	}

	return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_exec, __cmd_exec, exec a app module);
#endif
#endif
#endif
