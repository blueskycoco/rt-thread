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

#define PROTL_V				0x0000
#define CMD_LOGIN			0x0001
#define CMD_HEART			0x0002
#define CMD_EXIT			0x0003
#define CMD_ALARM			0x0004
#define CMD_MAIN_EVENT		0x0006
#define CMD_SUB_EVENT		0x0005
#define CMD_ASK_ADDR		0x0007
#define CMD_ASK_SUB_ACK		0x8711
#define CMD_ASK_MAIN_ACK	0x8712

#define NET_STATE_UNKNOWN  	0
#define NET_STATE_INIT  	1
#define NET_STATE_LOGIN		2
#define NET_STATE_LOGED		3

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
