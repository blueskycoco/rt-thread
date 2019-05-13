#include <stdint.h>
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f10x.h>
#include <string.h>
#include "wizchip_conf.h"
#include "dhcp.h"
#include "socket.h"
#include "led.h"
#include <string.h>
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"
#include <ctype.h>
extern uint8_t ip_assigned;
extern int g_index;
extern rt_uint8_t g_type1;
extern rt_uint8_t g_net_state;
extern rt_uint32_t g_server_addr;
extern rt_uint32_t g_server_addr_bak;
extern rt_uint16_t g_server_port;
extern rt_uint16_t g_server_port_bak;
extern rt_uint8_t g_ip_index;
extern rt_uint8_t ftp_cfg_step;
extern rt_uint8_t ftp_addr[32];
extern rt_uint8_t ftp_user[32];
extern rt_uint8_t ftp_passwd[32];
extern rt_uint8_t g_heart_cnt;
extern rt_uint8_t entering_ftp_mode;
extern rt_uint8_t *tmp_stm32_bin;
extern rt_size_t	tmp_stm32_len;
extern int stm32_fd;
extern int stm32_len;
extern int cur_stm32_len;
extern int down_fd;
extern uint8_t		  qiftp_read_file[32];//		"AT+QFREAD=\"RAM:stm32.bin\",0\r\n"
extern uint8_t			qiftp_ip_close_file[32];
extern rt_uint8_t ftp_rty;
extern rt_uint16_t g_app_v;
extern struct rt_event g_info_event;
extern rt_mp_t server_mp;
extern rt_uint8_t 	cur_status;
extern rt_uint16_t g_crc;
extern rt_uint8_t *g_ftp;
extern rt_uint8_t g_module_type;
extern rt_uint8_t in_qiact;
extern rt_uint32_t qiact_times;
extern rt_uint8_t m26_restart_flag;
extern void begin_yunduo();
extern rt_uint8_t upgrade_type;
uint8_t 	  *server_buf_ip 			= RT_NULL;
void 		  *send_data_ptr_ip 		= RT_NULL;
rt_size_t 	  send_size_ip;

wiz_NetInfo gWIZNETINFO = {
	.mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
	.ip = {192, 168, 1, 2},
	.sn = {255, 255, 255, 0},
	.gw = {192, 168, 1, 1},
	.dns = {0, 0, 0, 0},
	.dhcp = NETINFO_DHCP 
};
#define SOCK_DHCP       	6
#define SOCK_TASK			1
#define DATA_BUF_SIZE   2048
uint8_t gDATABUF[DATA_BUF_SIZE];
uint8_t cmd[] = {0xad,0xac,0x00,0x26,0x01,0x00,0x01,0x00,0x00,0xa1,0x18,0x08,0x10,0x00,0x09,0x13,0x51,0x56,0x42,0x48,0x42,0x4c,0x08,0x66,0x85,0x60,0x31,0x97,0x56,0x81,0x28,0x37,0x41,0x33,0x19,0x01,0xed,0x60};
void EXTI2_IRQHandler(void)
{
    intr_kind source;
    uint8_t sn_intr;
	rt_interrupt_enter();
    if(EXTI_GetITStatus(EXTI_Line2))
    {
        ctlwizchip(CW_GET_INTERRUPT,&source);
        ctlwizchip(CW_CLR_INTERRUPT,&source);
        rt_kprintf("w5500 intr %x\r\n", source);
        ctlsocket(SOCK_DHCP, CS_GET_INTERRUPT, &sn_intr);
        //ctlsocket(SOCK_DHCP, CS_CLR_INTERRUPT, &sn_intr);
        rt_kprintf("dhcp ir %x\r\n", sn_intr);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
	rt_interrupt_leave();
}

void ip_spi_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOD
			|RCC_APB2Periph_AFIO	, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

	/* w5500 reset, power */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 /*| GPIO_Pin_6*/; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_4);
	//GPIO_ResetBits(GPIOD, GPIO_Pin_6);

	/* spi cs,mosi,miso,clk */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_3);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI3, &SPI_InitStructure);
	SPI_Cmd(SPI3, ENABLE);

	/* w5500 int */
#if 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif
}

void  wizchip_select(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_3); 
}

void  wizchip_deselect(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_3); 
}

uint8_t wizchip_read()
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI3, 0xff);
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI3);
}

void  wizchip_write(uint8_t wb)
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI3, wb);
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI3);
}
void wizchip_reset(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_4);
	rt_thread_delay(16);
	GPIO_ResetBits(GPIOD, GPIO_Pin_4);
	rt_thread_delay(5);  
	GPIO_SetBits(GPIOD, GPIO_Pin_4);
	rt_thread_delay(16);
}

static uint8_t PHYStatus_Check(void)
{
	uint8_t tmp;

	ctlwizchip(CW_GET_PHYLINK, (void*) &tmp);
	//rt_kprintf("tmp %x\r\n", tmp);
	return (tmp == PHY_LINK_OFF) ? 0 : 1;
}
int32_t ip_send(uint8_t sn)
{
	int32_t ret = 0;
	if (rt_data_queue_peak(&g_data_queue[2],(const void **)&send_data_ptr_ip,&send_size_ip) == RT_EOK)
	{	
		rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_ip, &send_size_ip, RT_WAITING_FOREVER);
		rt_kprintf("should send data %d\r\n", send_size_ip);

		ret = send(sn, send_data_ptr_ip, send_size_ip);
		if (send_data_ptr_ip) {
			if (send_size_ip <= 64) {
				rt_mp_free(send_data_ptr_ip);
			} else {
				rt_free(send_data_ptr_ip);
			}
			send_data_ptr_ip = RT_NULL;
		}
		if (ret < 0) {
			ip_close(sn);
			return ret;
		}
	}
}
int32_t loopback_tcpc(uint8_t sn)//, uint8_t* buf, uint8_t* destip, uint16_t destport)
{
	int32_t ret;
	uint16_t size = 0, sentsize=0;
	uint8_t *buf = gDATABUF;
	static uint16_t any_port = 	50000;

	switch(getSn_SR(sn))
	{
		case SOCK_ESTABLISHED :
			if(getSn_IR(sn) & Sn_IR_CON)
			{
#ifdef _LOOPBACK_DEBUG_
				rt_kprintf("%d:Connected to - %d.%d.%d.%d : %d\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
				setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
			}

			if((size = getSn_RX_RSR(sn)) > 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
			{
				if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
				ret = recv(sn, buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)
				if(ret <= 0) return ret; // If the received data length <= 0, receive failed and process end
				size = (uint16_t) ret;
				sentsize = 0;
				for (int i=0; i<ret; i++)
					rt_kprintf("%02x ",buf[i]);
				rt_kprintf("\r\n");
				rt_uint8_t *server_buf_ip = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
				rt_memcpy(server_buf_ip, buf, ret);
				rt_data_queue_push(&g_data_queue[3], server_buf_ip, ret, RT_WAITING_FOREVER);
#if 0
				// Data sentsize control
				while(size != sentsize)
				{
					ret = send(sn, buf+sentsize, size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
					if(ret < 0) // Send Error occurred (sent data length < 0)
					{
						ip_close(sn); // socket ip_close
						return ret;
					}
					sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
				}
#endif
			} else {
				ret = ip_send(sn);//, cmd, sizeof(cmd));
				if (ret < 0) {
					return ret;
				}

			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			break;

		case SOCK_CLOSE_WAIT :
#ifdef _LOOPBACK_DEBUG_
			rt_kprintf("%d:CloseWait\r\n",sn);
#endif
			if((ret=disconnect(sn)) != SOCK_OK) return ret;
#ifdef _LOOPBACK_DEBUG_
			rt_kprintf("%d:Socket Closed\r\n", sn);
#endif
			break;

		case SOCK_INIT :
			rt_kprintf("%d:Try to connect to the %d.%d.%d.%d : %d\r\n", sn, mp.socketAddress[g_ip_index].IP[0], 
					mp.socketAddress[g_ip_index].IP[1], mp.socketAddress[g_ip_index].IP[2], 
					mp.socketAddress[g_ip_index].IP[3], mp.socketAddress[g_ip_index].port);
			if( (ret = connect(sn, mp.socketAddress[g_ip_index].IP, mp.socketAddress[g_ip_index].port)) != SOCK_OK) {
				g_ip_index++;
				if (g_ip_index == MAX_IP_LIST)
					g_ip_index = 0;
				return ret;	//	Try to TCP connect to the TCP server (destination)
			} else {
				rt_kprintf("connect to server ok %d\r\n",g_index);
				g_heart_cnt=0;
				g_net_state = NET_STATE_INIT;
				rt_event_send(&(g_pcie[g_index]->event), 1);
				rt_hw_led_on(NET_LED);
			}
			break;

		case SOCK_CLOSED:
			rt_kprintf("%d:Sock Closed\r\n", sn);
			ip_close(sn);
			g_heart_cnt=0;
			g_net_state = NET_STATE_UNKNOWN;
			if((ret=socket(sn, Sn_MR_TCP, any_port++, 0x00)) != sn){
				if(any_port == 0xffff) 
					any_port = 50000;
				return ret; // TCP socket open with 'any_port' port number
			} 
#ifdef _LOOPBACK_DEBUG_
			//rt_kprintf("%d:TCP client loopback start\r\n",sn);
			//rt_kprintf("%d:Socket opened\r\n",sn);
#endif
			break;
		default:
			rt_kprintf("default \r\n");
			break;
	}
	return 1;
}

void w5500_init()
{
	uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };
	uint8_t tmpstr[6] = {0};
	intr_kind intr_source = 0;
	uint8_t sn_intr = 0;
	rt_kprintf("w5500 1\r\n");
	wizchip_reset();
	rt_kprintf("w5500 2\r\n");
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	rt_kprintf("w5500 3\r\n");
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	rt_kprintf("w5500 4\r\n");
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {
		rt_kprintf("WIZCHIP Initialized fail.\r\n");
		while (1);
	}
	rt_kprintf("w5500 5\r\n");
	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
	rt_kprintf("w5500 6\r\n");
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	rt_kprintf("w5500 7\r\n");
	rt_kprintf("WIZnet %s EVB - DHCP client \r\n", tmpstr);

	DHCP_init(SOCK_DHCP, gDATABUF);
	rt_kprintf("w5500 8\r\n");
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
	rt_kprintf("w5500 9\r\n");
#if 1
	ctlwizchip(CW_GET_INTRMASK,&intr_source);
	rt_kprintf("intr %x\r\n", intr_source);
	intr_source |= IK_SOCK_ALL;
	ctlwizchip(CW_SET_INTRMASK,&intr_source);
	ctlwizchip(CW_GET_INTRMASK,&intr_source);
	rt_kprintf("new intr %x\r\n", intr_source);
	ctlsocket(SOCK_DHCP, CS_SET_INTMASK, &sn_intr);
	ctlsocket(SOCK_DHCP, CS_GET_INTMASK, &sn_intr);
	rt_kprintf("socket sn ir %x\r\n", sn_intr);
	sn_intr |= 0x1f;
	ctlsocket(SOCK_DHCP, CS_SET_INTMASK, &sn_intr);
	ctlsocket(SOCK_DHCP, CS_GET_INTMASK, &sn_intr);
	rt_kprintf("new socket sn ir %x\r\n", sn_intr);
#endif
}
void task()
{
	uint8_t  task_ip[] = {106,14,116,201};
	uint16_t task_port = 1704;
	uint8_t dest_ip[] = {47,93,48,167};
	uint16_t dest_port = 1721;
	uint8_t *path = "/A1/21/stm32.bin";
	uint8_t ftpstart = 0;
	rt_kprintf("task 0 \r\n");
	w5500_init();
	rt_kprintf("task 1 \r\n");
	while (1) {
		if (!PHYStatus_Check()) {
			rt_kprintf("Link is lost .. \r\n");
			rt_thread_delay(100);
			continue;
		}
		dhcp_handler();
		if (ip_assigned) {
			loopback_tcpc(SOCK_TASK);//, gDATABUF, task_ip, task_port);
#if 0
			rt_thread_delay(5000);
			if (ftpstart == 0) {
				rt_kprintf("to start ftp\r\n");
				ftpstart = 1;
				rt_kprintf("ftp result %d\r\n", ftpc_run(gDATABUF, dest_ip, dest_port, path));
			}
#endif
		}
	}
}

void ip_thread(void* param)
{
	rt_kprintf("ip_thread 1 \r\n");
	g_type1 = PCIE_2_IP;
	pcie_switch(PCIE_2_IP);
	rt_kprintf("ip_thread 2 \r\n");
	ip_spi_init();
	rt_kprintf("ip_thread 3 \r\n");
	task();
}

