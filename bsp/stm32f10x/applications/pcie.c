#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "ec20.h"
#include "nb_iot.h"
#include "ip_module.h"
#include "pcie.h"
#include "bsp_misc.h"

rt_uint8_t pcie_init(rt_uint8_t type);
rt_uint8_t pcie_switch(rt_uint8_t type);

rt_uint8_t pcie_init(rt_uint8_t type)
{
	rt_uint8_t index;
	
	rt_device_t dev_pcie2=rt_device_find("uart2"); //PCIE2
	rt_device_t dev_pcie1=rt_device_find("uart3"); //PCIE1	
	rt_device_open(dev_pcie2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
	rt_device_open(dev_pcie1, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
	switch (type) 
	{
		case PCIE_1_IP:
			ip_module_init(dev_pcie1);
			break;
		case PCIE_2_IP:
			ip_module_init(dev_pcie2);
			break;
		case PCIE_1_M26:
			m26_init(dev_pcie1);
			break;
		case PCIE_2_M26:
			m26_init(dev_pcie2);
			break;
		case PCIE_1_EC20:
			ec20_init(dev_pcie1);
			break;
		case PCIE_2_EC20:
			ec20_init(dev_pcie2);
			break;
		case PCIE_1_NBIOT:
			nb_iot_init(dev_pcie1);
			break;
		case PCIE_2_NBIOT:
			nb_iot_init(dev_pcie2);
			break;
		default:
			rt_kprintf("unknown module\r\n");
			return 0;
	}
}
