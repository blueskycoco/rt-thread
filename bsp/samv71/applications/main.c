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
#include <spi_flash.h>
#include <spi_flash_sfud.h>
#include "drv_qspi.h"
#include "drv_sdio.h"
#endif 
void mnt_init(void)
{

    if (RT_EOK != rt_hw_sdio_init())
		return ;
    rt_thread_delay(RT_TICK_PER_SECOND * 1);

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
int main(void)
{
	/* put user application code here */
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
	mnt_init();
#endif
    return 0;
}

