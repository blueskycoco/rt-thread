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
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32f4xx_eth.h"
#endif
#include "aes128.h"
#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif
void aes_test()
{
	uint32_t key[AES_KEY_SIZE]={0x40414243,0x44454647,0x48494a4b,0x4c4d4e4f};
	/* Expanded key */
	uint32_t exp_key[AES_EXPKEY_SIZE];
	/* Two plaitext blosks */
	uint32_t p1[AES_BLOCK_SIZE]={0x20212223,0x24252627,0x28292a2b,0x2c2d2e2f};
	uint32_t p2[AES_BLOCK_SIZE]={0x12345678,0x9abcdef0,0xfedcba98, 0x76543210};
	uint32_t p3[AES_BLOCK_SIZE] = {0};
	uint32_t p4[AES_BLOCK_SIZE] = {0};
	/* Two ciphertext blocks */
	uint32_t c1[AES_BLOCK_SIZE];
	uint32_t c2[AES_BLOCK_SIZE];

	/* Encryption key scheduling, to be done once */
	AES_keyschedule_enc((uint32_t*)key,(uint32_t*)exp_key);

	/* First block encryption */
	AES_encrypt((uint32_t*)p1,(uint32_t*)c1,(uint32_t*)exp_key); 
	/* Second block encryption */
	AES_encrypt((uint32_t*)p2,(uint32_t*)c2,(uint32_t*)exp_key); 


	/*******************************************************/
	/*                   AES ECB DECRYPTION                */
	/*******************************************************/

	/* Decryption key scheduling, to be done once */
	AES_keyschedule_dec((uint32_t*)key,(uint32_t*)exp_key);

	/* First block decryption */
	AES_decrypt((uint32_t*)c1,(uint32_t*)p3,(uint32_t*)exp_key);
	/* Second block decryption */
	AES_decrypt((uint32_t*)c2,(uint32_t*)p4,(uint32_t*)exp_key);

	for (int i=0; i<4; i++)
	{
		rt_kprintf("%08x -> %08x -> %08x\r\n", p1[i],c1[i],p3[i]);
		rt_kprintf("%08x -> %08x -> %08x\r\n", p2[i],c2[i],p4[i]);
		if (p3[i] != p1[i])
			rt_kprintf("p3 is not same to p1 at %d\r\n", i);
		if (p2[i] != p4[i])
			rt_kprintf("p2 is not same to p4 at %d\r\n", i);
	}

}
void rt_init_thread_entry(void* parameter)
{
	/* GDB STUB */
#ifdef RT_USING_GDB
	gdb_set_device("uart6");
	gdb_start();
#endif

	/* LwIP Initialization */
#ifdef RT_USING_LWIP
	{
		extern void lwip_sys_init(void);

		/* register ethernetif device */
		eth_system_device_init();

		rt_hw_stm32_eth_init();

		/* init lwip system */
		lwip_sys_init();
		rt_kprintf("TCP/IP initialized!\n");
	}
#endif
	rt_thread_delay(1000);
	aes_test();
}

int rt_application_init()
{
	rt_thread_t tid;

	tid = rt_thread_create("init",
			rt_init_thread_entry, RT_NULL,
			2048, RT_THREAD_PRIORITY_MAX/3, 20);

	if (tid != RT_NULL)
		rt_thread_startup(tid);

	return 0;
}

/*@}*/
