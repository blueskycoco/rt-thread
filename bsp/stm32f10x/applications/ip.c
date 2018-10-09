#include <stdio.h>
#include <unistd.h> 
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <ctype.h>
#include "led.h"
#include <string.h>
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"

#include "socket.h"

#include "w5500.h"
//#include "util.h"
#include "dhcp.h"
uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
uint8 ch_status[MAX_SOCK_NUM] = {0};
extern uint8 dhcp_state;

#define CONFIG_MSG_LEN        sizeof(CONFIG_MSG) - 4 // the 4 bytes OP will not save to EEPROM
#define SOCK_DHCP             0
#define TX_RX_MAX_BUF_SIZE	2048
uint8 TX_BUF[TX_RX_MAX_BUF_SIZE];
extern uint8 RX_BUF[TX_RX_MAX_BUF_SIZE];
uint8 I_STATUS[MAX_SOCK_NUM];
struct rt_mutex iplock;
extern uint8*  SRC_MAC_ADDR     ;    // Local MAC address
extern uint8*   GET_SN_MASK    ;     // Subnet mask received from the DHCP server
extern uint8*   GET_GW_IP      ;     // Gateway ip address received from the DHCP server
extern uint8*   GET_DNS_IP     ;    // DNS server ip address received from the DHCP server
extern uint8*   DHCP_HOST_NAME;   // HOST NAME
extern uint8*   GET_SIP       ; 
CONFIG_MSG_DHCP  ConfigMsg;

void w5500_isr(void)
{
	rt_uint8_t IR_val = 0, SIR_val = 0;
	rt_uint8_t tmp,s;
	rt_kprintf("in w5500 isr1\r\n");
	IINCHIP_WRITE(IMR, 0x00);
	rt_kprintf("in w5500 isr2\r\n");
	IINCHIP_WRITE(SIMR, 0x00);
	rt_kprintf("in w5500 isr3\r\n");
	IINCHIP_ISR_DISABLE();
	rt_kprintf("in w5500 isr4\r\n");

	IR_val = IINCHIP_READ(IR);
	SIR_val = IINCHIP_READ(SIR);

	rt_kprintf("\r\nIR_val : %02x", IR_val);
	rt_kprintf("\r\nSIR_val : %02x\r\n", SIR_val);
	rt_kprintf("\r\nSn_MR(0): %02x", IINCHIP_READ(Sn_MR(0)));
	rt_kprintf("\r\nSn_SR(0): %02x\r\n", IINCHIP_READ(Sn_SR(0)));

	if (IR_val > 0) {
		if (IR_val & IR_CONFLICT) {
			rt_kprintf("IP conflict : %.2x\r\n", IR_val);
		}
		if (IR_val & IR_MAGIC) {
			rt_kprintf("Magic packet: %.2x\r\n",IR_val);
		}
		if (IR_val & IR_PPPoE) {
			rt_kprintf("PPPoE closed: %.2x\r\n",IR_val);
		}
		if (IR_val & IR_UNREACH) {
			rt_kprintf("Destination unreached: %.2x\r\n",IR_val);
		}
		IINCHIP_WRITE(IR, IR_val);
	}
	for(s = 0;s < 8;s ++) {
		tmp = 0;
		if (SIR_val & IR_SOCK(s)) {
			tmp = IINCHIP_READ(Sn_IR(s));
			I_STATUS[s] |= tmp;
			tmp &= 0x0F; 
			//I_STATUS_FLAG[0]++;
			IINCHIP_WRITE(Sn_IR(s), tmp);	
			rt_kprintf("Sn_IR(%d): %.2x\r\n",s, tmp);
		}
	}
	IINCHIP_ISR_ENABLE();
	IINCHIP_WRITE(IMR, 0xF0);
	IINCHIP_WRITE(SIMR, 0xFF); 
	if (I_STATUS[0] != 0 || I_STATUS[1] != 0)
		rt_sem_release(&(g_pcie[1]->sem));
	//rt_kprintf("IR2: %02x, IMR2: %02x, Sn_IR(0): %02x, Sn_IMR(0): %02x\r\n",
	//	IINCHIP_READ(IR2), IINCHIP_READ(IMR2), IINCHIP_READ(Sn_IR(0)), IINCHIP_READ(Sn_IMR(0)));
}
void Set_network(void)
{
	rt_uint8_t tmp_array[6];
	rt_uint8_t i;
	rt_uint8_t mac[6]={0x00,0x08,0xdc,0x11,0x11,0x15};

	for (i = 0 ; i < 6; i++) ConfigMsg.mac[i] = mac[i];

	for (i = 0 ; i < 4; i++) ConfigMsg.lip[i] = GET_SIP[i];

	for (i = 0 ; i < 4; i++) ConfigMsg.gw[i] = GET_GW_IP[i];

	for (i = 0 ; i < 4; i++) ConfigMsg.sub[i] = GET_SN_MASK[i];

	setSHAR(ConfigMsg.mac);
	setSUBR(ConfigMsg.sub);
	setGAR(ConfigMsg.sub);
	setSIPR(ConfigMsg.lip);

	sysinit(txsize, rxsize);

	getSHAR(tmp_array);
	rt_kprintf("\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X",
	tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3],tmp_array[4],tmp_array[5]);

	getSIPR (tmp_array);
	rt_kprintf("\r\nIP : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);

	getSUBR(tmp_array);
	rt_kprintf("\r\nSN : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);

	getGAR(tmp_array);
	rt_kprintf("\r\nGW : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
}
void SOCK_DISCON(SOCKET s)
{
	disconnect(s);
	ch_status[s] = 0;
	I_STATUS[s] &= ~(0x02);
}

void loopback_tcpc(SOCKET s, rt_uint16_t port)
{
	uint16 len; 						
	rt_uint8_t * data_buf = (u8*) TX_BUF;
	rt_uint8_t	tmp = 0;
	rt_uint8_t destip[4] = {106,14,116,201};
	rt_uint16_t rport = 1704;
	switch (I_STATUS[s]) {
		case SOCK_CLOSED:
			if(!ch_status[s]) {
				rt_kprintf("\r\n%d : Loop-Back TCP Client Started. port: %d", s, port);
				ch_status[s] = 1;
			}
			if(socket(s, Sn_MR_TCP, port, 0x00) == 0) {
				rt_kprintf("\a%d : Fail to create socket.",s);
				ch_status[s] = 0;
			}
			else	
				connect(s, destip, rport);
			break;
		case Sn_IR_CON:
			ch_status[s] = 2;
			I_STATUS[s] &= ~(0x01);
			rt_kprintf("connect to server ok\r\n");
			break;
		case Sn_IR_DISCON:
			if ((len = getSn_RX_RSR(s)) > 0) {
				if (len > TX_RX_MAX_BUF_SIZE) len = TX_RX_MAX_BUF_SIZE;
				len = recv(s, data_buf, len);
			}
			SOCK_DISCON(s);
			rt_kprintf("server disconnect\r\n");
			break;
		case Sn_IR_RECV: 
			IINCHIP_WRITE(IMR, 0x00);
			IINCHIP_WRITE(SIMR, 0x00);
			IINCHIP_ISR_DISABLE();
			tmp = I_STATUS[s];
			I_STATUS[s] &= ~(0x04);
			IINCHIP_ISR_ENABLE();
			IINCHIP_WRITE(IR, 0xF0);
			IINCHIP_WRITE(SIR, 0xFF);
			if (tmp & Sn_IR_RECV) {
				if((len = getSn_RX_RSR(s)) > 0) 	{
					if (len > TX_RX_MAX_BUF_SIZE) 
						len = TX_RX_MAX_BUF_SIZE;
					rt_mutex_take(&iplock,RT_WAITING_FOREVER);
					len = recv(s, data_buf, len);
					rt_mutex_release(&iplock);
					rt_kprintf("recv_data_len : %d, getSn_RX_RSR(%d) : %d\r\n", len, s, getSn_RX_RSR(s));	
				}
			}
			break;
		case Sn_IR_SEND_OK:
			I_STATUS[s] &= ~(0x10);
			break;
	}
}
uint16 socket_send(const uint8 * buf, uint16 len)
{
	uint16 res = 0;
	
	rt_mutex_take(&iplock,RT_WAITING_FOREVER);
	len = send(1, buf, len);
	rt_mutex_release(&iplock);

	return res;
}
void WIZ_SPI_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	rt_mutex_init(&iplock,	"iplock",	RT_IPC_FLAG_FIFO);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOA
											  |RCC_APB2Periph_AFIO	, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_3);
		  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI3, &SPI_InitStructure);
	SPI_Cmd(SPI3, ENABLE);
	
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
	rt_kprintf("ip module init\r\n");
}

void WIZ_CS(rt_uint8_t val)
{
	if (val == 0) {
   		GPIO_ResetBits(GPIOD, GPIO_Pin_3); 
	} else if (val == 1){
   		GPIO_SetBits(GPIOD, GPIO_Pin_3); 
	}
}

void Reset_W5500(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_4);
	rt_thread_delay(5);  
	GPIO_SetBits(GPIOD, GPIO_Pin_4);
	rt_thread_delay(160);
}


uint8_t SPI2_SendByte(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI3, byte);
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI3);
}
void set_default(void)
{
  
  uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x15};
  uint8 lip[4]={192,168,1,111};
  uint8 sub[4]={255,255,255,0};
  uint8 gw[4]={192,168,1,1};
  uint8 dns[4]={8,8,8,8};
  memcpy(ConfigMsg.lip, lip, 4);
  memcpy(ConfigMsg.sub, sub, 4);
  memcpy(ConfigMsg.gw,  gw, 4);
  memcpy(ConfigMsg.mac, mac,6);
  memcpy(ConfigMsg.dns,dns,4);
  /*
  uint8 dhcp;
  uint8 debug;

  uint16 fw_len;
  uint8 state;
  */
  ConfigMsg.dhcp=0;
  ConfigMsg.debug=1;
  ConfigMsg.fw_len=0;
  
  ConfigMsg.state=0;
  ConfigMsg.sw_ver[0]=1;
  ConfigMsg.sw_ver[1]=0;
  
}

void ip_module_start()
{
	rt_uint8_t C_Flag = 0;
	//WIZ_SPI_Init();
	rt_kprintf("ip module start1\r\n");
	Reset_W5500();
	set_default();
	rt_kprintf("ip module start2\r\n");
	init_dhcp_client();
	rt_kprintf("ip module start2.2\r\n");
	setRTR(2000);
	setRCR(5);
	IINCHIP_WRITE(Sn_MR(7), 0x20);
	IINCHIP_WRITE(Sn_IMR(7), 0x0F);
	rt_kprintf("ip module start3\r\n");

	IINCHIP_WRITE(IMR, 0xF0);
	IINCHIP_WRITE(SIMR, 0xFE); 
	rt_kprintf("read IMR %x\r\n", IINCHIP_READ(IMR));
	rt_kprintf("read SIMR %x\r\n", IINCHIP_READ(SIMR));
	rt_kprintf("ip module start4\r\n");
	while (1) {
	uint8 dhcpret = check_DHCP_state(SOCK_DHCP);
	rt_kprintf("dhcp ret %x\r\n", dhcpret);
		switch(dhcpret) {
		  case DHCP_RET_NONE:
			break;
		  case DHCP_RET_TIMEOUT:
			break;
		  case DHCP_RET_UPDATE:
			Set_network();
			rt_kprintf("DHCP OK!\r\n");  
			C_Flag = 1;
			break;
		  case DHCP_RET_CONFLICT:
			C_Flag = 0;
			rt_kprintf("DHCP Fail!\r\n");
			dhcp_state = STATE_DHCP_READY;
			break; 
		  default:
			break;
		}
	if (C_Flag)
		break;
	rt_thread_delay(100);
	}
}

void w5500_proc(void *parameter)
{
	uint8 C_Flag = 0;
	rt_kprintf("w5500 proc\r\n");
	while(1) {			
		rt_sem_take(&(g_pcie[1]->sem), RT_WAITING_FOREVER);
		rt_kprintf("w5500 int\r\n");
		uint8 dhcpret = check_DHCP_state(SOCK_DHCP);
		switch(dhcpret) {
		  case DHCP_RET_NONE:
			break;
		  case DHCP_RET_TIMEOUT:
			break;
		  case DHCP_RET_UPDATE:
			Set_network();
			rt_kprintf("DHCP OK!\r\n");  
			C_Flag = 1;
			break;
		  case DHCP_RET_CONFLICT:
			C_Flag = 0;
			rt_kprintf("DHCP Fail!\r\n");
			dhcp_state = STATE_DHCP_READY;
			break; 
		  default:
			break;
		}

		if(C_Flag == 1) {
			loopback_tcpc(1, 1234);
		}
	}
}
