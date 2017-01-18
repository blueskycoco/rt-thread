#include <rtthread.h>
#include "board.h"

#include "drv_sdram.h"

struct rt_memheap system_heap;

bool sdram_init(void)
{
	BOARD_ConfigureSdram();
	if (RT_EOK != rt_memheap_init(&system_heap,
						"system",
						(void *)SDRAM_CS_ADDR,
						BOARD_SDRAM_SIZE))
		return false;
	
	return true;
}
void *sdram_malloc(unsigned long size)
{
    return rt_memheap_alloc(&system_heap, size);
}
RTM_EXPORT(sdram_malloc);

void sdram_free(void *ptr)
{
    rt_memheap_free(ptr);
}
RTM_EXPORT(sdram_free);

void *sdram_realloc(void *ptr, unsigned long size)
{
    return rt_memheap_realloc(&system_heap, ptr, size);
}
RTM_EXPORT(sdram_realloc);

#ifdef RT_USING_FINSH
#include <finsh.h>
static void sdram_init_t(void)
{
	bool result = sdram_init();
	if (result == true)
		rt_kprintf("sdram_init ok\n");
	else		
		rt_kprintf("sdram_init failed\n");
}
static void sdram_test(void)
{
	bool result = BOARD_SdramValidation(SDRAM_CS_ADDR, BOARD_SDRAM_SIZE);
	if (result)
		rt_kprintf("sdram_test ok\n");
	else		
		rt_kprintf("sdram_test failed\n");
}

FINSH_FUNCTION_EXPORT(sdram_init_t, test sdram_init);
FINSH_FUNCTION_EXPORT(sdram_test, test sdram);
#endif
