#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "ec20.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#define EC20_EVENT_0 				(1<<0)
#define EC20_STATE_INIT				0
#define EC20_STATE_CHECK_CPIN		1
#define EC20_STATE_SET_QINDI			2
#define EC20_STATE_SET_QIMUX			3
#define EC20_STATE_CHECK_CGREG		4
#define EC20_STATE_CHECK_QISTAT		5
#define EC20_STATE_SET_QICSGP		6
#define EC20_STATE_SET_QIREGAPP		7
#define EC20_STATE_SET_QISRVC		8
#define EC20_STATE_SET_QIACT			9
#define EC20_STATE_CHECK_CIMI		10
#define EC20_STATE_SET_QIFGCNT		11
#define EC20_STATE_SET_QIOPEN 		12
#define EC20_STATE_SET_QIDEACT  		13
#define EC20_STATE_DATA_PROCESSING 	14
#define EC20_STATE_DATA_READ			15
#define EC20_STATE_SET_QICLOSE		16
#define EC20_STATE_DATA_WRITE		17
#define EC20_STATE_DATA_ACK			18
#define EC20_STATE_DATA_PRE_WRITE	19
#define EC20_STATE_ATE0 				21
#define EC20_STATE_CSQ				22
#define EC20_STATE_LAC				23
#define EC20_STATE_ICCID			24
#define EC20_STATE_IMEI				25
#define EC20_STATE_LACR				26

#define STR_RDY						"RDY"
#define STR_CPIN					"+CPIN:"
#define STR_CPIN_READY				"+CPIN: READY"
#define STR_CGREG					"+CGREG: 0,"
#define STR_CGREG_READY				"+CGREG: 0,1"
#define STR_CGREG_READY1			"+CGREG: 0,5"
#define STR_STAT_INIT				"IP INITIAL"
#define STR_STAT_IND				"IP IND"
#define STR_STAT_CLOSE				"IP CLOSE"
#define STR_STAT_STATUS				"IP STATUS"
#define STR_STAT_DEACT				"PDP DEACT"
#define STR_QIMUX_0					"+QIMUX: 0"
#define STR_OK						"OK"
#define STR_QIRD					"+QIRD:"
#define STR_QIURC					"+QIURC: \"recv"
#define STR_TCP						"TCP,"
#define STR_CLOSED					"CLOSED"
#define STR_BEGIN_WRITE				">"
#define STR_ERROR					"ERROR"
#define STR_4600					"4600"
#define STR_46000					"46000"
#define STR_46001					"46001"
#define STR_46002					"46002"
#define STR_46003					"46003"
#define STR_46004					"46004"
#define STR_46006					"46006"
#define STR_STAT_DEACT_OK 			"DEACT OK"
#define STR_CLOSE_OK				"CLOSE OK"
#define STR_SEND_OK					"SEND OK"
#define STR_QIRDI					"+QIRDI:"
#define STR_QISACK					"+QISACK"
#define STR_SOCKET_BUSSY			"SOCKET BUSY"
#define STR_CONNECT_OK			"+QIOPEN: 0,0"
#define STR_CONNECT_FAIL		"+QIOPEN: 0,"
#define STR_CSQ						"+CSQ:"
#define STR_CREG					"+CREG:"
#define STR_QCCID				"+QCCID: "
#define DEFAULT_SERVER				"101.132.177.116"
#define DEFAULT_PORT				"2011"
#define cregs "AT+CREG=2\r\n"
#define cregr "AT+CREG?\r\n"
#define at_csq "AT+CSQ\r\n"
#define at_qccid "AT+QCCID\r\n"
#define gsn "AT+GSN\r\n"

#define qistat 		"AT+QISTATE=0,1\r\n"
#define qiclose "AT+QICLOSE\r\n"
#define qilocip  "AT+QILOCIP\r\n"
#define qilport  "AT+QILPORT?\r\n"
#define e0  "ATE0\r\n"
#define cgreg  "AT+CGREG?\r\n"
#define cimi  "AT+CIMI\r\n"
#define cpin  "AT+CPIN?\r\n"
#define qifgcnt "AT+QIFGCNT=0\r\n"
#define qisrvc  "AT+QISRVC=1\r\n"
#define qimux  "AT+QIMUX=0\r\n"
#define ask_qimux  "AT+QIMUX?\r\n"
#define qideact  "AT+QIDEACT=1\r\n"

#define qindi  "AT+QINDI=1\r\n"
#define qird  "AT+QIRD=0,1500\r\r\n"
#define qisack  "AT+QISACK\r\n"
#define qiat  "AT\r\n"
uint8_t 	  qicsgp_ec20[32]			= {0};
uint8_t 	  qiopen_ec20[64]			= {0};
uint8_t 	  qisend_ec20[32] 			= {0};
#define qiregapp  "AT+QIREGAPP\r\n"
#define qiact  "AT+QIACT=1\r\n"
rt_bool_t g_data_in_ec20 = RT_FALSE;
extern int g_index;

rt_device_t g_dev_ec20;
uint8_t g_ec20_state 				= EC20_STATE_INIT;
uint32_t server_len_ec20 = 0;
uint8_t *server_buf_ec20 = RT_NULL;
void *send_data_ptr_ec20 = RT_NULL;
rt_size_t send_size_ec20;
extern rt_uint8_t g_net_state;
rt_uint32_t g_server_addr;
rt_uint32_t g_server_addr_bak;
rt_uint16_t g_server_port;
rt_uint16_t g_server_port_bak;

rt_uint8_t g_ip_index=0;
void handle_ec20_server_in(const void *last_data_ptr,rt_size_t len)
{
		static rt_bool_t flag = RT_FALSE;
		int i;
		int ofs;
		if (match_bin(last_data_ptr,len, STR_OK,rt_strlen(STR_OK)) != -1 && 
			(match_bin(last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)) == -1)&& 
			!flag)
			return ;
		//for (i=0;i<len;i++)
			//rt_kprintf("%02x ",((char *)last_data_ptr)[i]);
		if ((ofs = match_bin(last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)))!=-1)
		{	
			uint8_t *pos = (uint8_t *)last_data_ptr;
			//rt_kprintf("ofs is %d\r\n",ofs);
			//if (pos != RT_NULL) {
				int i = 7+ofs;
			//	rt_kprintf("\r\n<>%x%x%x%x%x%x%x%x%x%x%x%x<>\r\n",
			//		pos[ofs],pos[ofs+1],pos[ofs+2],pos[ofs+3],pos[ofs+4],pos[ofs+5],pos[ofs+6],pos[ofs+7],
			//		pos[ofs+8],pos[ofs+9],pos[ofs+10],pos[ofs+11]);
				while (pos[i] != '\r' && pos[i+1] != '\n' && i<len)
				{
					server_len_ec20 = server_len_ec20*10 + pos[i] - '0';
					i++;
				}
				//rt_kprintf("server len %d\r\n", server_len_ec20);
				server_buf_ec20 = (uint8_t *)rt_malloc(server_len_ec20 * sizeof(uint8_t));
				rt_memset(server_buf_ec20,0,server_len_ec20);
				//server_len_ec20 = 0;
				i+=2;
				if (i+server_len_ec20 < len)
					rt_memcpy(server_buf_ec20,pos+i,server_len_ec20);
				else
				{
					server_len_ec20 = i+server_len_ec20-len;
					rt_memcpy(server_buf_ec20,pos+i,server_len_ec20);
				}
				/*while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
				{
	
					server_buf_ec20[server_len_ec20++] = pos[i++];
					rt_kprintf("<%02x>\r\n", server_buf_ec20[server_len_ec20-1]);
				}*/
				if (match_bin(pos, len, "OK",rt_strlen("OK"))!=-1)
				{
					rt_data_queue_push(&g_data_queue[3], server_buf_ec20, server_len_ec20, RT_WAITING_FOREVER);
					if (server_len_ec20 == 1500) {
						g_ec20_state = EC20_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_ec20,qistat);
					} else {
						g_ec20_state = EC20_STATE_DATA_PROCESSING;
						gprs_at_cmd(g_dev_ec20,at_csq);
					}
					if (match_bin(last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI))==-1)
						g_data_in_ec20 = RT_FALSE;
	
					flag = RT_FALSE;
				}
				else
					flag = RT_TRUE;
				/*handle server request
				  put to another dataqueue
				  */
			//}
		}
	
		else if (flag){ 
			int i=0;
			uint8_t *pos = (uint8_t *)last_data_ptr;
			while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{
				server_buf_ec20[server_len_ec20++] = pos[i++];
			}
	
			if (match_bin(pos, len,"OK",2)!=RT_NULL)
			{
				rt_data_queue_push(&g_data_queue[3], server_buf_ec20, server_len_ec20, RT_WAITING_FOREVER);
				if (server_len_ec20 == 1500) {
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				} else {
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_ec20,at_csq);		
				}
				if (match_bin(last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI)))
					g_data_in_ec20 = RT_FALSE;
				flag = RT_FALSE;
			}
		}
	}


void ec20_start(int index)
{
	rt_uint32_t power_rcc,pwr_key_rcc;
	rt_uint16_t power_pin,pwr_key_pin;
	GPIO_TypeDef* GPIO_power,*GPIO_pwr;
	GPIO_InitTypeDef GPIO_InitStructure;
	g_index = index;
	g_dev_ec20 = g_pcie[index]->dev;
	if (index) {
		power_rcc = RCC_APB2Periph_GPIOC;
		pwr_key_rcc = RCC_APB2Periph_GPIOC;
		power_pin = GPIO_Pin_8;
		pwr_key_pin = GPIO_Pin_8;
		GPIO_power = GPIOC;
		GPIO_pwr = GPIOC;
	} else {
		power_rcc = RCC_APB2Periph_GPIOC;
		pwr_key_rcc = RCC_APB2Periph_GPIOC;
		power_pin = GPIO_Pin_8;
		pwr_key_pin = GPIO_Pin_8;
		GPIO_power = GPIOC;
		GPIO_pwr = GPIOC;
	}
	RCC_APB2PeriphClockCmd(power_rcc|pwr_key_rcc,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = power_pin;
	GPIO_Init(GPIO_power, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = pwr_key_pin;
	GPIO_Init(GPIO_pwr, &GPIO_InitStructure);
	/*open pciex power*/
	GPIO_SetBits(GPIO_pwr, pwr_key_pin);
	//GPIO_ResetBits(GPIO_power, power_pin);
	rt_thread_delay(RT_TICK_PER_SECOND);
	GPIO_SetBits(GPIO_power, power_pin);
	GPIO_ResetBits(GPIO_pwr, pwr_key_pin);
	rt_thread_delay(RT_TICK_PER_SECOND);
	GPIO_SetBits(GPIO_pwr, pwr_key_pin);
}

void ec20_proc(void *last_data_ptr, rt_size_t data_size)
{
	int i=0;
	rt_uint8_t *tmp = (rt_uint8_t *)last_data_ptr;
	if (data_size != 6 && strstr(last_data_ptr, STR_QIRD)==NULL&& 
		!have_str(last_data_ptr,STR_CSQ))
		rt_kprintf("\r\n(EC20<= %d %d %s)\r\n",g_ec20_state, data_size,last_data_ptr);
	if (data_size >= 2) {
		switch (g_ec20_state) {
			case EC20_STATE_INIT:
				if (have_str(last_data_ptr,STR_RDY)) {
				g_ec20_state = EC20_STATE_ATE0;
				gprs_at_cmd(g_dev_ec20,e0);			
				}
				break;
			case EC20_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					g_ec20_state = EC20_STATE_CHECK_CPIN;
					gprs_at_cmd(g_dev_ec20,cpin);
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_ec20,e0);
				}
				break;
			case EC20_STATE_CHECK_CPIN:
				if (have_str(last_data_ptr,STR_CPIN_READY)) {
					g_ec20_state = EC20_STATE_CHECK_CGREG;
					gprs_at_cmd(g_dev_ec20,cgreg);
				} else/* if (have_str(last_data_ptr, STR_CPIN))*/
				{					
					g_pcie[g_index]->cpin_cnt++;
					if (g_pcie[g_index]->cpin_cnt>10)
					{/*power off this module , power on another module*/
						SetStateIco(3,1);
						if (g_index==0)
							SetErrorCode(0x08);
						else
							SetErrorCode(0x09);
					}
					rt_thread_delay(100);
					gprs_at_cmd(g_dev_ec20,cpin);
					
				}

				break;
			case EC20_STATE_CSQ:
				if (have_str(last_data_ptr,STR_CSQ)) {
					if (tmp[9] == 0x2c)
						g_pcie[g_index]->csq = tmp[8]-0x30;
					else
						g_pcie[g_index]->csq = (tmp[8]-0x30)*10+tmp[9]-0x30;
					rt_kprintf("csq is %x %x %d\r\n",tmp[8],tmp[9],g_pcie[g_index]->csq);
					show_signal(g_pcie[g_index]->csq);
					g_ec20_state = EC20_STATE_LAC;
					gprs_at_cmd(g_dev_ec20,cregs);
				} 
				break;
			case EC20_STATE_LAC:
				if (have_str(last_data_ptr,STR_OK)) {
					g_ec20_state = EC20_STATE_LACR;
					gprs_at_cmd(g_dev_ec20,cregr);
				} 
				break;
			case EC20_STATE_LACR:
				if (have_str(last_data_ptr,STR_CREG)) {
						i=0;
						while(tmp[i]!='"' && i<strlen(tmp))
							i++;
						if (((rt_uint8_t *)last_data_ptr)[i+1]>='A' && ((rt_uint8_t *)last_data_ptr)[i+1]<='F' )
							g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[i+1]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[i+1]-'0';
						
						if (((rt_uint8_t *)last_data_ptr)[i+2]>='A' && ((rt_uint8_t *)last_data_ptr)[i+2]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+2]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+2]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+3]>='A' && ((rt_uint8_t *)last_data_ptr)[i+3]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+3]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+3]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+4]>='A' && ((rt_uint8_t *)last_data_ptr)[i+4]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+4]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+4]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+8]>='A' && ((rt_uint8_t *)last_data_ptr)[i+8]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+8]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+8]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+9]>='A' && ((rt_uint8_t *)last_data_ptr)[i+9]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+9]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+9]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+10]>='A' && ((rt_uint8_t *)last_data_ptr)[i+10]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+10]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+10]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+11]>='A' && ((rt_uint8_t *)last_data_ptr)[i+11]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+11]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+11]-'0';
						rt_kprintf("ORI %c%c%c%c%c%c%c%c LAC_CI %08x\r\n", 
							((rt_uint8_t *)last_data_ptr)[i+1],((rt_uint8_t *)last_data_ptr)[i+2],
							((rt_uint8_t *)last_data_ptr)[i+3],((rt_uint8_t *)last_data_ptr)[i+4],
							((rt_uint8_t *)last_data_ptr)[i+8],((rt_uint8_t *)last_data_ptr)[i+9],
							((rt_uint8_t *)last_data_ptr)[i+10],((rt_uint8_t *)last_data_ptr)[i+11],
							g_pcie[g_index]->lac_ci);
				} 
				g_ec20_state = EC20_STATE_ICCID;
				gprs_at_cmd(g_dev_ec20,at_qccid);
				break;
			case EC20_STATE_ICCID:
				if (have_str(last_data_ptr,STR_QCCID)) {
					rt_uint8_t *tmp1 = strstr(last_data_ptr,STR_QCCID);
					i=8;
					while(tmp1[i]!='\r')
					{
						if (tmp1[i]>='0' && tmp1[i]<='9')
							g_pcie[g_index]->qccid[i/2-4] = (tmp1[i]-0x30)*16;
						else if (tmp1[i]>='a' && tmp1[i]<='f')
							g_pcie[g_index]->qccid[i/2-4] = (tmp1[i]-'a'+10)*16;
						else if (tmp1[i]>='A' && tmp1[i]<='F')
							g_pcie[g_index]->qccid[i/2-4] = (tmp1[i]-'A'+10)*16;
			
						if (tmp1[i+1]>='0' && tmp1[i+1]<='9')
							g_pcie[g_index]->qccid[i/2-4] += (tmp1[i+1]-0x30);
						else if (tmp1[i+1]>='a' && tmp1[i+1]<='f')
							g_pcie[g_index]->qccid[i/2-4] += (tmp1[i+1]-'a'+10);
						else if (tmp1[i+1]>='A' && tmp1[i+1]<='F')
							g_pcie[g_index]->qccid[i/2-4] += (tmp1[i+1]-'A'+10);
						rt_kprintf("qccid[%d] = %02X\r\n",i/2-4,g_pcie[g_index]->qccid[i/2-4]);
						i+=2;						
					}					
					
					g_ec20_state = EC20_STATE_IMEI;
					gprs_at_cmd(g_dev_ec20,gsn);
				}
					break;
			case EC20_STATE_IMEI:
				if (have_str(last_data_ptr,"86")) {
				g_pcie[g_index]->imei[0]=0x0;
				i=0;
				i=2;//866159032379171
				while(tmp[i]!='\r' && i<17)
				{
					if (tmp[i]>='0' && tmp[i]<='9')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i]-0x30);
					else if (tmp[i]>='a' && tmp[i]<='f')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i]-'a'+10);
					else if (tmp[i]>='A' && tmp[i]<='F')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i]-'A'+10);
					rt_kprintf("imei[%d] = %02X\r\n",i/2,g_pcie[g_index]->imei[i/2-1]);
					//i+=2;
					if (tmp[i+1]>='0' && tmp[i+1]<='9')
						g_pcie[g_index]->imei[i/2] = (tmp[i+1]-0x30)*16;
					else if (tmp[i+1]>='a' && tmp[i+1]<='f')
						g_pcie[g_index]->imei[i/2] = (tmp[i+1]-'a'+10)*16;
					else if (tmp[i+1]>='A' && tmp[i+1]<='F')
						g_pcie[g_index]->imei[i/2] = (tmp[i+1]-'A'+10)*16;
				
					i+=2;						
				}					
				
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				}
					break;
			
			case EC20_STATE_CHECK_CGREG:
				if (have_str(last_data_ptr, STR_CGREG_READY) ||
						have_str(last_data_ptr, STR_CGREG_READY1)) {
					//g_ec20_state = EC20_STATE_CHECK_QISTAT;
					g_ec20_state = EC20_STATE_CSQ;
					gprs_at_cmd(g_dev_ec20,at_csq);
				} else/* if(have_str(last_data_ptr, STR_CGREG))*/ {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_ec20,cgreg);
				}
				break;
			case EC20_STATE_CHECK_QISTAT:
				if(have_str(last_data_ptr, STR_OK)) { //e20
					g_ec20_state = EC20_STATE_CHECK_CIMI;
					gprs_at_cmd(g_dev_ec20,cimi);
				}
				g_net_state = NET_STATE_UNKNOWN;
				break;
			case EC20_STATE_SET_QICLOSE:
				if (have_str(last_data_ptr, STR_CLOSE_OK)) {
					g_ec20_state = EC20_STATE_SET_QIOPEN;
					rt_memset(qiopen_ec20, 0, 64);
					rt_sprintf(qiopen_ec20, "AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",
							mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
							mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
							mp.socketAddress[g_ip_index].port);
					gprs_at_cmd(g_dev_ec20,qiopen_ec20);
				}
				else
				{
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				}
				break;
			case EC20_STATE_SET_QINDI:
				if (have_str(last_data_ptr, STR_OK)) {
					g_ec20_state = EC20_STATE_CHECK_CIMI;
					gprs_at_cmd(g_dev_ec20,cimi);
				}
				break;								
			case EC20_STATE_SET_QIMUX:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_ec20_state = EC20_STATE_SET_QINDI;
					gprs_at_cmd(g_dev_ec20,qindi);
				}
				break;		
			case EC20_STATE_CHECK_CIMI:
				rt_memset(qicsgp_ec20, 0, 32);
				if (have_str(last_data_ptr, STR_4600)) {
					if (have_str(last_data_ptr, STR_46000) ||
							have_str(last_data_ptr, STR_46002) ||
							have_str(last_data_ptr, STR_46004)) {
						rt_sprintf(qicsgp_ec20, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CMNET");
					} else if (have_str(last_data_ptr, STR_46001) ||
							have_str(last_data_ptr, STR_46006)){						
						rt_sprintf(qicsgp_ec20, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "UNINET");
					} else if (have_str(last_data_ptr, STR_46003)){						
						rt_sprintf(qicsgp_ec20, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CTNET");
					} else {						
						rt_sprintf(qicsgp_ec20, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CMNET");
					}
					g_ec20_state = EC20_STATE_SET_QICSGP;
					gprs_at_cmd(g_dev_ec20,qicsgp_ec20);
				}
				else
					gprs_at_cmd(g_dev_ec20,cimi);
				break;						
			case EC20_STATE_SET_QICSGP:
				if (have_str(last_data_ptr, STR_OK)) {					
					g_ec20_state = EC20_STATE_SET_QIACT;
					gprs_at_cmd(g_dev_ec20,qiact);			
				}
				break;		
			case EC20_STATE_SET_QIREGAPP:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_ec20_state = EC20_STATE_SET_QISRVC;
					gprs_at_cmd(g_dev_ec20,qisrvc);
				}
				break;
			case EC20_STATE_SET_QISRVC:
				if (have_str(last_data_ptr, STR_OK)) {
					g_ec20_state = EC20_STATE_SET_QIACT;
					gprs_at_cmd(g_dev_ec20,qiact);
				}
				break;		
			case EC20_STATE_SET_QIACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_ec20_state = EC20_STATE_SET_QIOPEN;
					rt_memset(qiopen_ec20, 0, 64);					
					rt_sprintf(qiopen_ec20, "AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",
							mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
							mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
							mp.socketAddress[g_ip_index].port);
					gprs_at_cmd(g_dev_ec20,qiopen_ec20);
				} else {
					/*check error condition*/
					g_ec20_state = EC20_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_ec20,qideact);
				}
				break;
			case EC20_STATE_SET_QIDEACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				} else if (have_str(last_data_ptr, STR_ERROR)){
					g_ec20_state = EC20_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_ec20,qideact);
				}
				break;						
			case EC20_STATE_SET_QIOPEN:
				if (have_str(last_data_ptr, STR_CONNECT_OK)) {
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					/*send data here */					
					g_server_addr = (mp.socketAddress[g_ip_index].IP[0] << 24)|
									(mp.socketAddress[g_ip_index].IP[1] << 16)|
									(mp.socketAddress[g_ip_index].IP[2] <<  8)|
									(mp.socketAddress[g_ip_index].IP[3] <<  0);
					g_server_port = mp.socketAddress[g_ip_index].port;
					g_server_addr_bak = g_server_addr;
					g_server_port_bak = g_server_port;
					rt_kprintf("connect to server ok\r\n");
					g_net_state = NET_STATE_INIT;
					rt_event_send(&(g_pcie[g_index]->event), EC20_EVENT_0);
					gprs_at_cmd(g_dev_ec20,at_csq);
					rt_hw_led_on(NET_LED);
				} else if (have_str(last_data_ptr, STR_ERROR) ||
					have_str(last_data_ptr, STR_CONNECT_FAIL)){
					g_ec20_state = EC20_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_ec20,qideact);
					if (g_ip_index+1<MAX_IP_LIST && !(mp.socketAddress[g_ip_index+1].IP[0] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[1] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[2] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[3] !=0 &&
						mp.socketAddress[g_ip_index+1].port !=0))
						g_ip_index++;
				}
				break;
			case EC20_STATE_DATA_PROCESSING:
				if (have_str(last_data_ptr, STR_QIURC)) {
					/*server data in */
					g_ec20_state = EC20_STATE_DATA_READ;
					gprs_at_cmd(g_dev_ec20,qird);
					server_len_ec20 = 0;
					g_data_in_ec20 = RT_TRUE;
				} else if (have_str(last_data_ptr,STR_CSQ)) {
					if (tmp[9] == 0x2c)
						g_pcie[g_index]->csq = tmp[8]-0x30;
					else
						g_pcie[g_index]->csq = (tmp[8]-0x30)*10+tmp[9]-0x30;
					//rt_kprintf("csq is %x %x %d\r\n",tmp[8],tmp[9],g_pcie[g_index]->csq);
					show_signal(g_pcie[g_index]->csq);
					//rt_kprintf("server addr %08x:%04x, bak %08x:04x\r\n", g_server_addr,g_server_port,g_server_addr_bak,g_server_port_bak);
					if (g_server_addr != g_server_addr_bak || g_server_port != g_server_port_bak) {
						g_ip_index=0;
						g_ec20_state = EC20_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_ec20,qistat);
					}
					/*check have data to send */
					if (rt_data_queue_peak(&g_data_queue[2],(const void **)&send_data_ptr_ec20,&send_size_ec20) == RT_EOK)
					{	
						rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_ec20, &send_size_ec20, RT_WAITING_FOREVER);
						rt_hw_led_off(NET_LED);
						rt_kprintf("should send data %d\r\n", send_size_ec20);
						rt_sprintf(qisend_ec20, "AT+QISEND=0,%d\r\n", send_size_ec20);
						gprs_at_cmd(g_dev_ec20,qisend_ec20);
						g_ec20_state = EC20_STATE_DATA_PRE_WRITE;
					}else if (have_str(last_data_ptr,STR_CREG)) {
						i=0;
						while(tmp[i]!='"' && i<strlen(tmp))
							i++;
						if (((rt_uint8_t *)last_data_ptr)[i+1]>='A' && ((rt_uint8_t *)last_data_ptr)[i+1]<='F' )
							g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[i+1]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[i+1]-'0';
						
						if (((rt_uint8_t *)last_data_ptr)[i+2]>='A' && ((rt_uint8_t *)last_data_ptr)[i+2]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+2]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+2]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+3]>='A' && ((rt_uint8_t *)last_data_ptr)[i+3]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+3]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+3]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+4]>='A' && ((rt_uint8_t *)last_data_ptr)[i+4]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+4]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+4]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+8]>='A' && ((rt_uint8_t *)last_data_ptr)[i+8]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+8]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+8]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+9]>='A' && ((rt_uint8_t *)last_data_ptr)[i+9]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+9]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+9]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+10]>='A' && ((rt_uint8_t *)last_data_ptr)[i+10]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+10]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+10]-'0';
						if (((rt_uint8_t *)last_data_ptr)[i+11]>='A' && ((rt_uint8_t *)last_data_ptr)[i+11]<='F' )
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+11]-'A'+10;
						else
							g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[i+11]-'0';
						rt_kprintf("ORI %c%c%c%c%c%c%c%c LAC_CI %08x\r\n", 
							((rt_uint8_t *)last_data_ptr)[i+1],((rt_uint8_t *)last_data_ptr)[i+2],
							((rt_uint8_t *)last_data_ptr)[i+3],((rt_uint8_t *)last_data_ptr)[i+4],
							((rt_uint8_t *)last_data_ptr)[i+8],((rt_uint8_t *)last_data_ptr)[i+9],
							((rt_uint8_t *)last_data_ptr)[i+10],((rt_uint8_t *)last_data_ptr)[i+11],
							g_pcie[g_index]->lac_ci);
							gprs_at_cmd(g_dev_ec20,at_csq);
					} 
					else {								
						rt_thread_delay(100);
						gprs_at_cmd(g_dev_ec20,at_csq);
					}

				} else {

					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				}

				break;
			case EC20_STATE_DATA_READ:
				handle_ec20_server_in(last_data_ptr,data_size);
				break;
			case EC20_STATE_DATA_PRE_WRITE:
				if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
					g_ec20_state = EC20_STATE_DATA_WRITE;
					rt_device_write(g_pcie[g_index]->dev, 0, send_data_ptr_ec20, send_size_ec20);
					rt_event_send(&(g_pcie[g_index]->event), EC20_EVENT_0);
				}
				else if(have_str(last_data_ptr, STR_ERROR))
				{
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				}
				if (have_str(last_data_ptr, STR_QIURC))
					g_data_in_ec20 = RT_TRUE;
				break;
			case EC20_STATE_DATA_WRITE:
				if (have_str(last_data_ptr, STR_SEND_OK)) {
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					rt_hw_led_on(NET_LED);
					gprs_at_cmd(g_dev_ec20,at_csq);
				} else {
					if (!have_str(last_data_ptr, STR_QIURC)) {
						g_ec20_state = EC20_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_ec20,qistat);
					}
				}
				if (have_str(last_data_ptr, STR_QIURC))
					g_data_in_ec20 = RT_TRUE;
				break;			
		}
	}
}
