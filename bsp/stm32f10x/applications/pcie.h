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
typedef struct _pcie_param {
	rt_device_t dev;
	struct rt_event event;
	struct rt_mutex lock;
	struct rt_semaphore sem;
	rt_uint8_t csq;
	rt_uint32_t lac_ci;
	rt_uint32_t cpin_cnt;
	rt_uint8_t qccid[10];
	rt_uint8_t imei[8];	
}pcie_param,*ppcie_param;
ppcie_param g_pcie[2];
#define GPRS_EVENT_0 (1<<0)
rt_uint8_t pcie_init(rt_uint8_t type0, rt_uint8_t type1);
rt_uint8_t pcie_switch(rt_uint8_t type);
rt_uint8_t check_pcie(rt_uint8_t num);
#endif
