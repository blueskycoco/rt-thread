#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

static rt_sem_t rx_sem = RT_NULL;
rt_device_t uart_device;

static rt_err_t rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(rx_sem);
}

static void wifi_handler(void *arg)
{
    rt_uint8_t val[256] = {0};
    int i;

    while (1) {
	rt_sem_take(rx_sem, RT_WAITING_FOREVER);
	int len = rt_device_read(uart_device, 0, val, 256);
	if (len > 0) {
		for (i=0; i<len; i++)
		    rt_kprintf("%c", val[i]);
	}
    }
}

int wifi_init()
{
    int err = 0;

    uart_device = rt_device_find("uart1");

    RT_ASSERT(uart_device != RT_NULL);

    err = rt_device_open(uart_device, RT_DEVICE_FLAG_INT_RX);
    if (err != RT_EOK) {
	rt_kprintf("open uart1 failed\n");
	return -1;
    }

    rx_sem = rt_sem_create("uart_sem", 0, RT_IPC_FLAG_FIFO);
    rt_device_set_rx_indicate(uart_device, rx_ind);
    rt_thread_t tid = rt_thread_create("wifi", wifi_handler, RT_NULL,
	    2048, 28, 20);
    rt_thread_startup(tid);
}
#ifdef FINSH_USING_MSH
static int wifi(int argc, char **argv)
{
    const char switch_to_at[1] = "a";
    const char enter[1] = "\n";
    int i;
    rt_kprintf("wifi cmd[%d]: %s\r\n", strlen(argv[1]), argv[1]);
    if (strcmp(argv[1], "+++") == 0) {
    	rt_device_write(uart_device, 0, argv[1], 3);
    	rt_thread_delay(100);
    	rt_device_write(uart_device, 0, switch_to_at, 1);
    } else {
    	for (i=0; i<strlen(argv[1]); i++)
    	    	if (argv[1][i] >= 'a' && argv[1][i] <= 'z')    
    	    	argv[1][i] = argv[1][i] - 32;

    	rt_kprintf("wifi new cmd[%d]: %s\r\n", strlen(argv[1]), argv[1]);
    	rt_device_write(uart_device, 0, argv[1], strlen(argv[1]));
    	rt_device_write(uart_device, 0, enter, 1);
    }
    return 0;
}
MSH_CMD_EXPORT(wifi, wifi cmd);
#endif /* FINSH_USING_MSH */
