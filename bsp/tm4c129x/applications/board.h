/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 * 2010-02-04     Magicoe      add board.h to LPC176x bsp
 * 2014-07-18     ArdaFu       port it to TM4C129X bsp
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "tm4c1294ncpdt.h"
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>

extern uint32_t SysClock;
// <RDTConfigurator URL="http://www.rt-thread.com/eclipse">

// <bool name="RT_USING_UART0" description="Using UART0" default="true" />
#define RT_USING_UART0
// <bool name="RT_USING_UART1" description="Using UART1" default="true" />
//#define RT_USING_UART1
// <bool name="RT_USING_UART2" description="Using UART2" default="true" />
#define RT_USING_UART2
#define RT_USING_UART3
#define RT_USING_UART4
#define RT_USING_UART6

#define COMMAND_FAIL "Command crc fail"
#define COMMAND_OK "Command exec OK"
void usb_config(rt_uint8_t *data,int ipv6_len,int dev);

// </RDTConfigurator>

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN  ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN  (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN  ((void *)&__bss_end)
#endif
#define HEAP_END    (0x20000000 + 256*1024)

#define FINSH_DEVICE_NAME   RT_CONSOLE_DEVICE_NAME
void rt_hw_board_init(void);

#endif
