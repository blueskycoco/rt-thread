#ifndef _PCIE_
#define _PCIE_
#define PCIE_1_IP		0x01
#define PCIE_2_IP		0x02
#define PCIE_1_EC20		0x04
#define PCIE_2_EC20		0x08
#define PCIE_1_M26		0x10
#define PCIE_2_M26		0x20
#define PCIE_1_NBIOT	0x40
#define PCIE_2_NBIOT	0x80
struct rt_data_queue *g_data_queue;
rt_uint8_t g_type0 = 0;
rt_uint8_t g_type1 = 0;
#endif
