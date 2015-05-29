/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */

/**
 * @addtogroup k64
 */
/*@{*/

#include <stdio.h>

#include <board.h>
#include <rtthread.h>

#include "led.h"

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32_eth.h"
#endif
#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
//#include <rtgui/calibration.h>
#endif
#ifdef RT_USING_DFS
#include <dfs_init.h>
#include <dfs_fs.h>

#ifdef RT_USING_DFS_UFFS
#include <dfs_uffs.h>
#include "psram_mtd.h"
#endif
#endif

void rt_init_thread_entry(void* parameter)
{
    rt_hw_led_init();

    while (1)
    {
		rt_hw_led_on();
		rt_thread_delay(RT_TICK_PER_SECOND/20);
		rt_hw_led_off();
		rt_thread_delay(RT_TICK_PER_SECOND/20);
    }

}
int rt_application_init()
{
    rt_thread_t init_thread;
    

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}

/*@}*/
