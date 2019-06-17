#include <stdio.h>
#include <unistd.h> 
#include <board.h>
#include <rtthread.h>
#include <ctype.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "led.h"
#include <string.h>
#include "ec20.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"
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
#define EC20_STATE_CHECK_SENT		27
#define EC20_STATE_CFG_FTP		28
#define EC20_STATE_OPEN_FTP		29
#define EC20_STATE_GET_FILE		30
#define EC20_STATE_READ_FILE	31
#define EC20_STATE_LOGOUT_FTP	32
#define EC20_STATE_V			33
#define EC20_STATE_FTP_PATH		34
#define STR_POWER_DOWN				"POWERED DOWN"
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
#define STR_QIACT					"+QIACT:"
#define STR_OK						"OK"
#define STR_QIRD					"+QIRD:"
#define STR_QICSGP					"+QICSGP:"
#define STR_QIURC					"+QIURC: \"recv"
#define STR_TCP						"TCP,"
#define STR_CLOSED					"closed"
#define STR_BEGIN_WRITE				">"
#define STR_ERROR					"ERROR"
#define STR_4600					"4600"
#define STR_46000					"46000"
#define STR_CFUN					"+CFUN:"
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
#define STR_CREG1					"+CREG: 1"
#define STR_CREG5					"+CREG: 5"
#define STR_QCCID				"+QCCID: "
#define STR_PDP_DEACT	"pdpdeact"
#define DEFAULT_SERVER				"101.132.177.116"
#define DEFAULT_PORT				"2011"
#define STR_NO_DATA					"+QIRD: 0"
#define STR_FTPPATH					"+QFTPCWD: 0,0"
#define cregs "AT+CREG=2\r\n"
#define cregr "AT+CREG?\r\n"
#define at_csq "AT+CSQ\r\n"
#define at_qccid "AT+QCCID\r\n"
#define gsn "AT+GSN\r\n"
#define STR_QISEND "+QISEND:"
rt_uint8_t ftp_addr[32] = {0};//"u.110lw.com";
rt_uint8_t ftp_user[32] = {0};//"minfei";
rt_uint8_t ftp_passwd[32] = {0};//"minfei123";
extern rt_mp_t server_mp;
extern rt_uint8_t upgrade_type;

rt_uint8_t ftp_cfg_step = 0;

#define at_qisend	"AT+QISEND=0,0\r\n"
#define qistat 		"AT+QISTATE=0,1\r\n"
#define qiclose "AT+QICLOSE=0\r\n"
#define qilocip  "AT+QILOCIP\r\n"
#define qilport  "AT+QILPORT?\r\n"
#define e0  "ATE0\r\n"
#define cgreg  "AT+CGREG?\r\n"
#define cimi  "AT+CIMI\r\n"
#define cpin  "AT+CPIN?\r\n"
#define cfun  "AT+CFUN=1,1\r\n"
#define qifgcnt "AT+QIFGCNT=0\r\n"
#define qisrvc  "AT+QISRVC=1\r\n"
#define qimux  "AT+QIMUX=0\r\n"
#define ask_qimux  "AT+QIMUX?\r\n"
#define qideact  "AT+QIDEACT=1\r\n"
#define qflds	 "AT+QFLDS=\"RAM\"\r\n"
#define qindi  "AT+QINDI=1\r\n"
#define qird  "AT+QIRD=0,1500\r\r\n"
#define qisack  "AT+QISACK\r\n"
#define qiat  "AT\r\n"
#define ati	"ATI\r\n"
#define atdbg "AT+QURCCFG=\"urcport\",\"uart1\"\r\n"
#define atdbg1 "AT+QCFG=\"DBGCTL\",0\r\n"
uint8_t qftpcwd[64]	= {0};//"AT+QFTPCWD="
uint8_t 	  qicsgp_ec20[32]			= {0};
uint8_t 	  qiopen_ec20[64]			= {0};
uint8_t 	  qisend_ec20[32] 			= {0};
uint8_t 	  qiftp_ec20[64] = {0};
uint8_t		  qiftp_read_file[32] = {0};//		"AT+QFREAD=\"RAM:stm32.bin\",0\r\n"
uint8_t			qiftp_close_file[32] = {0};
uint8_t		  qiftp_get_ec20[128]			= {0};
#define qiregapp  "AT+QIREGAPP\r\n"
#define qiact  "AT+QIACT=1\r\n"
rt_bool_t g_data_in_ec20 = RT_FALSE;
extern int g_index;
extern rt_uint8_t 	cur_status;
rt_uint8_t ftp_rty=0;
rt_uint8_t entering_ftp_mode=0;
rt_device_t g_dev_ec20;
uint8_t g_ec20_state 				= EC20_STATE_INIT;
uint32_t server_len_ec20 = 0;
uint8_t *server_buf_ec20 = RT_NULL;
void *send_data_ptr_ec20 = RT_NULL;
rt_size_t send_size_ec20;
extern rt_uint8_t g_net_state;
extern rt_uint8_t g_heart_cnt;
int stm32_fd=0;
int stm32_len=0;
int cur_stm32_len=0;
int down_fd=0;
extern rt_uint8_t *g_ftp;
rt_uint32_t g_server_addr;
rt_uint32_t g_server_addr_bak;
rt_uint16_t g_server_port;
rt_uint16_t g_server_port_bak;
rt_uint8_t g_ip_index=0;
rt_uint8_t *tmp_stm32_bin = RT_NULL;
rt_size_t	tmp_stm32_len = 0;
rt_uint8_t g_module_type = 0;
extern rt_uint16_t g_app_v;
extern struct rt_event g_info_event;
rt_uint8_t need_read = 0;
rt_uint32_t bak_server_len_ec20=0;
void handle_ec20_server_in(const void *last_data_ptr,rt_size_t len)
{
		static rt_bool_t flag = RT_FALSE;
		int i;
		int ofs;
		if (match_bin((rt_uint8_t *)last_data_ptr, len, STR_NO_DATA,rt_strlen(STR_NO_DATA))!=-1) {
			g_ec20_state = EC20_STATE_DATA_PROCESSING;
			gprs_at_cmd(g_dev_ec20,at_csq);		
			return;
		}
		if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIURC,rt_strlen(STR_QIURC))!=-1) {
					rt_kprintf("got another qirdi\r\n");
					gprs_at_cmd(g_dev_ec20,qird);
					server_len_ec20 = 0;
					g_data_in_ec20 = RT_TRUE;
					flag=RT_FALSE;
					return ;
			}
		if (match_bin((rt_uint8_t *)last_data_ptr,len, STR_OK,rt_strlen(STR_OK)) != -1 && 
			(match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)) == -1)&& 
			!flag) {
			//for (i=0;i<len;i++)
			//	rt_kprintf("%c",((rt_uint8_t *)last_data_ptr)[i]);
			//rt_kprintf("m26 read again\r\n");
			if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_CSQ,rt_strlen(STR_CSQ))==-1) {
				gprs_at_cmd(g_dev_ec20,qird);
				server_len_ec20 = 0;
				g_data_in_ec20 = RT_TRUE;
			}
			return ;
			}
		rt_kprintf("????????????????????????????????????????????????\r\n");
		for (i=0;i<len;i++)
		{
			if (isascii(((char *)last_data_ptr)[i]))
				rt_kprintf("%c",((char *)last_data_ptr)[i]);
			else
				break;
		}
		rt_kprintf("\r\n");
    	rt_kprintf("????????????????????????????????????????????????\r\n");
		if ((ofs = match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)))!=-1)
		{	
			need_read = 0;
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
				//server_len_ec20 = get_len(pos+i,len-i);
				//rt_kprintf("server len %d\r\n", server_len_ec20);
		//		server_buf_ec20 = (uint8_t *)rt_malloc(server_len_ec20 * sizeof(uint8_t));
				server_buf_ec20 = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
				if (server_buf_ec20 == RT_NULL)
				{
					show_memory_info();
					rt_kprintf("malloc buf ec20 failed %d\r\n",server_len_ec20);
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_ec20,at_csq);		
					return ;
				}
				rt_memset(server_buf_ec20,0,server_len_ec20);
				//server_len_ec20 = 0;
				i+=2;
				if (i+server_len_ec20 < len)
					rt_memcpy(server_buf_ec20,pos+i,server_len_ec20);
				else
				{
					
					bak_server_len_ec20 = server_len_ec20;
					server_len_ec20 = i+server_len_ec20-len;
					rt_memcpy(server_buf_ec20,pos+i,server_len_ec20);
				}
				/*while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
				{
	
					server_buf_ec20[server_len_ec20++] = pos[i++];
					rt_kprintf("<%02x>\r\n", server_buf_ec20[server_len_ec20-1]);
				}*/
				if (match_bin((rt_uint8_t *)pos, len, "OK",rt_strlen("OK"))!=-1)
				{
					rt_data_queue_push(&g_data_queue[3], server_buf_ec20, server_len_ec20, RT_WAITING_FOREVER);
					if (server_len_ec20 == 1500) {
						g_ec20_state = EC20_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_ec20,qistat);
					} else {
						g_ec20_state = EC20_STATE_DATA_PROCESSING;
						gprs_at_cmd(g_dev_ec20,at_csq);
					}
					if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI))==-1)
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
				if (server_len_ec20 == bak_server_len_ec20)
					break;
			}
	
			if (match_bin((rt_uint8_t *)pos, len,"OK",2)!=-1)
			{
				rt_data_queue_push(&g_data_queue[3], server_buf_ec20, server_len_ec20, RT_WAITING_FOREVER);
				if (server_len_ec20 == 1500) {
					g_ec20_state = EC20_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_ec20,qistat);
				} else {
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_ec20,at_csq);		
				}
				if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIURC,rt_strlen(STR_QIURC))==-1)
					g_data_in_ec20 = RT_FALSE;
				flag = RT_FALSE;
			}
		}
	}


void ec20_start(int index)
{
	rt_uint32_t power_rcc,pwr_key_rcc,status_rcc;
	rt_uint16_t power_pin,pwr_key_pin,status_pin;
	GPIO_TypeDef* GPIO_power,*GPIO_pwr,*GPIO_status;
	GPIO_InitTypeDef GPIO_InitStructure;
	g_index = index;
	g_dev_ec20 = g_pcie[index]->dev;
	g_ec20_state = EC20_STATE_INIT;

	if (index) {
		power_rcc = RCC_APB2Periph_GPIOE;
		pwr_key_rcc = RCC_APB2Periph_GPIOB;
		status_rcc = RCC_APB2Periph_GPIOE;
		status_pin = GPIO_Pin_14; 
		power_pin = GPIO_Pin_13;
		pwr_key_pin = GPIO_Pin_3;
		GPIO_power = GPIOE;
		GPIO_pwr = GPIOB;
		GPIO_status = GPIOE;
	} else {
		power_rcc = RCC_APB2Periph_GPIOC;
		pwr_key_rcc = RCC_APB2Periph_GPIOC;
		status_rcc = RCC_APB2Periph_GPIOC;
		power_pin = GPIO_Pin_9;
		pwr_key_pin = GPIO_Pin_8;
		status_pin = GPIO_Pin_7;
		GPIO_power = GPIOC;
		GPIO_pwr = GPIOC;
		GPIO_status = GPIOC;
	}
	RCC_APB2PeriphClockCmd(status_rcc|power_rcc|pwr_key_rcc,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = power_pin;
	GPIO_Init(GPIO_power, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = pwr_key_pin;
	GPIO_Init(GPIO_pwr, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin   = status_pin;
	GPIO_Init(GPIO_status, &GPIO_InitStructure);

	if (GPIO_ReadInputDataBit(GPIO_status,status_pin) == Bit_RESET)
	{
		rt_kprintf("ec20 is running\r\n");	
		#if 0
		GPIO_ResetBits(GPIO_power, power_pin);
		GPIO_ResetBits(GPIO_pwr, pwr_key_pin);		
		rt_thread_delay(RT_TICK_PER_SECOND);
		
		GPIO_SetBits(GPIO_pwr, pwr_key_pin);
		rt_thread_delay(50);
		GPIO_ResetBits(GPIO_pwr, pwr_key_pin);
		while(GPIO_ReadInputDataBit(GPIO_status,status_pin) == Bit_RESET)
			rt_thread_delay(1);
		#else
		gprs_at_cmd(g_dev_ec20,cfun);
		rt_thread_delay(50);
		gprs_at_cmd(g_dev_ec20,cfun);
		rt_thread_delay(50);
		gprs_at_cmd(g_dev_ec20,cfun);
		return;
		#endif
		
	}
	GPIO_ResetBits(GPIO_power, power_pin);
	GPIO_ResetBits(GPIO_pwr, pwr_key_pin);		
	rt_thread_delay(RT_TICK_PER_SECOND);
	
	GPIO_SetBits(GPIO_pwr, pwr_key_pin);
	rt_thread_delay(50);
	GPIO_ResetBits(GPIO_pwr, pwr_key_pin);
	
	/*rt_thread_delay(300);
	GPIO_SetBits(GPIO_pwr, pwr_key_pin);
	rt_thread_delay(15);
	GPIO_ResetBits(GPIO_pwr, pwr_key_pin);
	

	GPIO_SetBits(GPIO_power, power_pin);
	rt_thread_delay(20);
	GPIO_ResetBits(GPIO_power, power_pin);
	*/

	rt_kprintf("ec20 init done\r\n");
}

void ec20_proc(void *last_data_ptr, rt_size_t data_size)
{
	int i=0;
	rt_uint8_t *tmp = (rt_uint8_t *)last_data_ptr;
	if (data_size != 6 && strstr(last_data_ptr, STR_QIRD)==NULL&& 
		!have_str(last_data_ptr,STR_CSQ))
		 if (!have_str(last_data_ptr,STR_CONNECT))
			rt_kprintf("\r\n(EC20<= %d %d %s)\r\n",g_ec20_state, data_size,last_data_ptr);
		 else
		 	rt_kprintf("\r\n(EC20<= %d %d)\r\n",g_ec20_state, data_size);
	if (data_size >= 2) {
		if (have_str(last_data_ptr,STR_RDY)||have_str(last_data_ptr,STR_CLOSED)
			|| have_str(last_data_ptr, STR_PDP_DEACT) || (have_str(last_data_ptr, STR_CREG) && 
			!have_str(last_data_ptr, STR_CREG1) && !have_str(last_data_ptr, 
															  STR_CREG5)
			&& g_net_state != NET_STATE_UNKNOWN))
		{
			g_ec20_state = EC20_STATE_INIT;
			if (have_str(last_data_ptr,STR_PDP_DEACT)) {
				rt_kprintf("MODULE lost1\r\n");
				pcie_switch(g_module_type);
			}
			if (have_str(last_data_ptr, STR_CREG) && !have_str(last_data_ptr, STR_CREG1) && !have_str(last_data_ptr,
						STR_CREG5)&& g_net_state != NET_STATE_UNKNOWN) {
				rt_kprintf("MODULE lost2\r\n");
				pcie_switch(g_module_type);
			}
			g_heart_cnt=0;
			g_net_state = NET_STATE_UNKNOWN;
		}
		
		switch (g_ec20_state) {
			case EC20_STATE_INIT:
				if (have_str(last_data_ptr,STR_RDY)||have_str(last_data_ptr,STR_CFUN)||have_str(last_data_ptr,STR_CLOSED)
						||have_str(last_data_ptr,STR_PDP_DEACT)) {
				g_ec20_state = EC20_STATE_ATE0;
				gprs_at_cmd(g_dev_ec20,e0);			
				}/* else if (have_str(last_data_ptr, STR_POWER_DOWN)){						
					pcie_switch(g_module_type);
				}*/
				break;
			case EC20_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					//g_ec20_state = EC20_STATE_CHECK_CPIN;
					//gprs_at_cmd(g_dev_ec20,cpin);					
					g_ec20_state = EC20_STATE_V;
					gprs_at_cmd(g_dev_ec20,atdbg);
					rt_thread_delay(100);
					gprs_at_cmd(g_dev_ec20,atdbg1);					
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_ec20,e0);
				}
				break;				
			case EC20_STATE_V:
				if (have_str(last_data_ptr,STR_OK)) {
					g_pcie[g_index]->cpin_cnt=0;
					g_ec20_state = EC20_STATE_CHECK_CPIN;
					gprs_at_cmd(g_dev_ec20,cpin);
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
				if (entering_ftp_mode) {
					g_ec20_state = EC20_STATE_CFG_FTP;
					ftp_cfg_step = 0;
					strcpy(ftp_addr,"u.110LW.com");
//					strcpy(ftp_addr,"47.93.48.167");
					strcpy(ftp_user,"minfei");
					strcpy(ftp_passwd,"minfei123");
					gprs_at_cmd(g_dev_ec20,qicfgftp_id);
				} else {
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
				}
				break;
			case EC20_STATE_CFG_FTP:
				{
					if (ftp_cfg_step == 3) {
						rt_sprintf(qiftp_ec20,"AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n", ftp_user,ftp_passwd);
						gprs_at_cmd(g_dev_ec20,qiftp_ec20);
					} else if (ftp_cfg_step == 0) {
						gprs_at_cmd(g_dev_ec20,qicfgftp_file);
					} else if (ftp_cfg_step == 1) {
						gprs_at_cmd(g_dev_ec20,qicfgftp_mode);
					} else if (ftp_cfg_step == 2) {
						gprs_at_cmd(g_dev_ec20,qicfgftp_timeout);					
					} else if (ftp_cfg_step == 4) {
						rt_sprintf(qiftp_ec20,"AT+QFTPOPEN=\"%s\",%d\r\n", ftp_addr,mp.updateDomainAddress.port);
						gprs_at_cmd(g_dev_ec20,qiftp_ec20);
						g_ec20_state = EC20_STATE_OPEN_FTP;
						ftp_rty=0;
					}
					ftp_cfg_step++;
				}
				break;
			case EC20_STATE_OPEN_FTP:
				{
					if (have_str(last_data_ptr, STR_FTP_OK)) {
						rt_kprintf("login ftp ok\r\n");
						g_ec20_state = EC20_STATE_FTP_PATH;
						sprintf(qftpcwd, "AT+QFTPCWD=\"%s\"\r\n", g_ftp);
						gprs_at_cmd(g_dev_ec20, qftpcwd);
					} else if (/*!have_str(last_data_ptr, STR_OK) || */have_str(last_data_ptr, STR_FTP_FAILED)){
						rt_thread_delay(100);
						gprs_at_cmd(g_dev_ec20,qiftp_ec20);
						ftp_rty++;
						if (ftp_rty>5)
						{
							entering_ftp_mode=0;
							g_ec20_state = EC20_STATE_CHECK_QISTAT;
							gprs_at_cmd(g_dev_ec20,qistat);
						}
					}
				}
				break;
			case EC20_STATE_FTP_PATH:
				{
					if (have_str(last_data_ptr, STR_FTPPATH)) {
						if (upgrade_type)
							sprintf(qiftp_get_ec20, "AT+QFTPGET=\"stm32.bin\",\"RAM:stm32.bin\",0\r\n");
						else
							sprintf(qiftp_get_ec20, "AT+QFTPGET=\"BootLoader.bin\",\"RAM:BootLoader.bin\",0\r\n");
						gprs_at_cmd(g_dev_ec20, qiftp_get_ec20);
						g_ec20_state = EC20_STATE_GET_FILE;				
						stm32_len=0;
					}
				}
				break;
			case EC20_STATE_GET_FILE:
				if (have_str(last_data_ptr, STR_QFTPGET_M26)) {
					if (have_str(last_data_ptr, STR_QFTPGET)) {
					stm32_len = get_len(strstr(last_data_ptr, STR_QFTPGET)+strlen(STR_QFTPGET),data_size-strlen(STR_QFTPGET));
					rt_kprintf("get stm32 len %d\r\n", stm32_len);
					if (stm32_len == 0) {
						g_ec20_state = EC20_STATE_LOGOUT_FTP;
						gprs_at_cmd(g_dev_ec20, at_csq);
					} else {
					if (upgrade_type)
					gprs_at_cmd(g_dev_ec20,qiftp_open_file);
					else
					gprs_at_cmd(g_dev_ec20,qiftp_open_file_boot);
					g_ec20_state = EC20_STATE_READ_FILE;
					stm32_fd=0;
					}
					} else {
						rt_kprintf("ftp download failed, exit\r\n");
						g_ec20_state = EC20_STATE_LOGOUT_FTP;
						gprs_at_cmd(g_dev_ec20, at_csq);
					}
				}
				break;
			case EC20_STATE_READ_FILE:
					if (stm32_fd == 0) {
						if (have_str(last_data_ptr, STR_QFOPEN)) {
							stm32_fd = get_len(strstr(last_data_ptr, STR_QFOPEN)+strlen(STR_QFOPEN),data_size-strlen(STR_QFOPEN));	
							if (stm32_len <= SINGLE_FILE_LEN)
							{
								sprintf(qiftp_read_file,"AT+QFREAD=%d,%d\r\n", stm32_fd,stm32_len);							
								//cur_stm32_len = stm32_len;
							} else {
								sprintf(qiftp_read_file,"AT+QFREAD=%d,1024\r\n", stm32_fd);
								//cur_stm32_len = SINGLE_FILE_LEN;
							}
							cur_stm32_len=0;
							if (upgrade_type)
								down_fd = open("/stm32.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);
							else
								down_fd = open("/BootLoader.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);			
							rt_kprintf("get stm32 fd %d\r\n", stm32_fd);
							gprs_at_cmd(g_dev_ec20,qiftp_read_file);	
							tmp_stm32_bin = (rt_uint8_t *)rt_malloc(1500*sizeof(rt_uint8_t));
							if (tmp_stm32_bin == RT_NULL)
							{
								show_memory_info();
								rt_kprintf("malloc tmp ec20 failed\r\n");
							}
							rt_memset(tmp_stm32_bin,0,1500);
							tmp_stm32_len=0;
						}		
					} else {
						rt_uint8_t *ptr = (rt_uint8_t *)last_data_ptr;					
						rt_uint16_t cnt=0;
						if (tmp_stm32_len+data_size>1500) 
							tmp_stm32_len = 0;
						else
							rt_memcpy(tmp_stm32_bin+tmp_stm32_len, (rt_uint8_t *)last_data_ptr, data_size);
						tmp_stm32_len += data_size;						
						rt_hw_led_off(NET_LED);
						rt_kprintf("tmp stm32 len %d \r\n",tmp_stm32_len);
						
						if (tmp_stm32_bin[tmp_stm32_len-1] == '\n'  
							&& tmp_stm32_bin[tmp_stm32_len-2] == '\r' 
							&& tmp_stm32_bin[tmp_stm32_len-3] == 'K'
							&& tmp_stm32_bin[tmp_stm32_len-4] == 'O'
							&& have_str(tmp_stm32_bin, STR_CONNECT)) {
							rt_uint16_t cur_len;
							cur_len = get_len(strstr(tmp_stm32_bin, STR_CONNECT)+strlen(STR_CONNECT),tmp_stm32_len-strlen(STR_CONNECT));
							rt_uint8_t *ch = strchr(strstr(tmp_stm32_bin, STR_CONNECT),'\n')+1;							
							rt_kprintf("%d cur Len %d\r\n", cur_stm32_len, cur_len);																
							if (cur_len != write(down_fd, ch, cur_len))
							{
								rt_kprintf("write data failed\n");
								close(down_fd);
								break;
							}
							cur_stm32_len +=cur_len;
							if (cur_stm32_len < stm32_len) {		
								gprs_at_cmd(g_dev_ec20,qiftp_read_file);									
								rt_hw_led_on(NET_LED);
							} else {
								close(down_fd);
								rt_free(tmp_stm32_bin);
								//cat_file("/stm32.bin");
								//CRC_check_file("/stm32.bin");
								sprintf(qiftp_close_file,"AT+QFCLOSE=%d\r\n",stm32_fd);
								gprs_at_cmd(g_dev_ec20,qiftp_close_file);
								g_ec20_state = EC20_STATE_LOGOUT_FTP;
								if (upgrade_type) {
								mp.firmCRC = CRC_check_file("/stm32.bin");
								if (g_app_v!=0)
									mp.firmVersion = g_app_v;
								mp.firmLength = stm32_len;
								} else {
								mp.firmCRC = CRC_check_file("/BootLoader.bin");
							rt_event_send(&(g_info_event), INFO_EVENT_SAVE_HWV);
							/* real update boot*/
							rt_kprintf("going to upgrade Boot\r\n");
							write_flash(0x08000000, "/BootLoader.bin", stm32_len);
								}
								rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
						if (!cur_status) {
						rt_thread_delay(500);
						rt_kprintf("programing app/boot done, reseting ...\r\n");
						NVIC_SystemReset();
						}
							}
							tmp_stm32_len=0;
							rt_memset(tmp_stm32_bin,0,1500);			
						} else
								gprs_at_cmd(g_dev_ec20,qiftp_read_file);									

					}				
				break;
			case EC20_STATE_LOGOUT_FTP:
				if (have_str(last_data_ptr, STR_OK))
				{
					gprs_at_cmd(g_dev_ec20,qiftp_close);
					g_ec20_state = EC20_STATE_SET_QIACT;
					entering_ftp_mode=0;
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
				if (have_str(last_data_ptr, STR_OK)|| have_str(last_data_ptr, STR_QFTPCLOSE)) {
					#if 0
					g_ec20_state = EC20_STATE_CFG_FTP;
					ftp_cfg_step = 0;
					strcpy(ftp_addr,"u.110LW.com");
//					strcpy(ftp_addr,"47.93.48.167");
					strcpy(ftp_user,"minfei");
					strcpy(ftp_passwd,"minfei123");
					gprs_at_cmd(g_dev_ec20,qicfgftp_id);
					#else				
					g_ec20_state = EC20_STATE_SET_QIOPEN;
					rt_memset(qiopen_ec20, 0, 64);					
					rt_sprintf(qiopen_ec20, "AT+QIOPEN=1,0,\"TCP\",\"%d.%d.%d.%d\",%d,0,0\r\n",
							mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
							mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
							mp.socketAddress[g_ip_index].port);
					gprs_at_cmd(g_dev_ec20,qiopen_ec20);
					#endif
				} else {
					/*check error condition*/
					g_ec20_state = EC20_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_ec20,qideact);
				}
				break;
			case EC20_STATE_SET_QIDEACT:
				if (have_str(last_data_ptr, STR_OK)) {
					//g_ec20_state = EC20_STATE_CHECK_QISTAT;
					//gprs_at_cmd(g_dev_ec20,qistat);
					g_ec20_state =EC20_STATE_SET_QICLOSE;
					gprs_at_cmd(g_dev_ec20,qiclose);
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
					g_heart_cnt=0;
					rt_event_send(&(g_pcie[g_index]->event), EC20_EVENT_0);
					gprs_at_cmd(g_dev_ec20,at_csq);
					rt_hw_led_on(NET_LED);
				} else if (have_str(last_data_ptr, STR_ERROR) ||
					have_str(last_data_ptr, STR_CONNECT_FAIL)){
					g_ec20_state = EC20_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_ec20,qideact);
					rt_kprintf("before , ip index %d\r\n", g_ip_index);
					if (g_ip_index+1<MAX_IP_LIST && (mp.socketAddress[g_ip_index+1].IP[0] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[1] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[2] !=0 &&
						mp.socketAddress[g_ip_index+1].IP[3] !=0 &&
						mp.socketAddress[g_ip_index+1].port !=0))
						g_ip_index++;
					else
						g_ip_index=0;
					rt_kprintf("after , ip index %d\r\n", g_ip_index);
					rt_thread_delay(100);
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
						g_ec20_state = EC20_STATE_SET_QICLOSE;
						gprs_at_cmd(g_dev_ec20,qiclose);
						break;
					}
					if (g_heart_cnt > 5 || entering_ftp_mode) {
						g_ec20_state = EC20_STATE_SET_QICLOSE;
						gprs_at_cmd(g_dev_ec20,qiclose);
						break;
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
						/*if (need_read) {
							g_ec20_state = EC20_STATE_DATA_READ;
							gprs_at_cmd(g_dev_ec20,qird);
							server_len_ec20 = 0;
							g_data_in_ec20 = RT_TRUE;
						} else*/
							gprs_at_cmd(g_dev_ec20,at_csq);
					}

				} else {/*
					rt_kprintf("data processing ,will reconnect %d %s\r\n",data_size,last_data_ptr);
					if (!have_str(last_data_ptr, STR_OK) && !have_str(last_data_ptr, STR_RDY)) {
						g_ec20_state = EC20_STATE_SET_QICLOSE;
						gprs_at_cmd(g_dev_ec20,qiclose);
					} else {
						gprs_at_cmd(g_dev_ec20,at_csq);
					}*/
					
					g_ec20_state = EC20_STATE_DATA_READ;
					gprs_at_cmd(g_dev_ec20,qird);
					server_len_ec20 = 0;
					g_data_in_ec20 = RT_TRUE;
				}

				break;
			case EC20_STATE_DATA_READ:
				handle_ec20_server_in(last_data_ptr,data_size);
				break;
			case EC20_STATE_DATA_PRE_WRITE:
				if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
					g_ec20_state = EC20_STATE_DATA_WRITE;
					//for (int ii=0;ii<send_size_ec20;ii++)
					//	rt_kprintf("sending %02x\r\n", ((rt_uint8_t *)send_data_ptr_ec20)[ii]);
					
					rt_device_write(g_pcie[g_index]->dev, 0, send_data_ptr_ec20, send_size_ec20);
					//rt_free(send_data_ptr_ec20);
				if (send_data_ptr_ec20) {
					if (send_size_ec20 <= 64) {
						rt_mp_free(send_data_ptr_ec20);
					} else {
						rt_free(send_data_ptr_ec20);
					}
					send_data_ptr_ec20 = RT_NULL;
				}
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
					//g_ec20_state = EC20_STATE_DATA_PROCESSING;
					//rt_hw_led_on(NET_LED);
					//gprs_at_cmd(g_dev_ec20,at_csq);
					g_ec20_state = EC20_STATE_CHECK_SENT;
					gprs_at_cmd(g_dev_ec20, at_qisend);
				} else {
					if (!have_str(last_data_ptr, STR_QIURC)) {
						//g_ec20_state = EC20_STATE_CHECK_QISTAT;
						//gprs_at_cmd(g_dev_ec20,qistat);
						g_ec20_state = EC20_STATE_SET_QICLOSE;
						gprs_at_cmd(g_dev_ec20,qiclose);
						break;
					}
				}
				if (have_str(last_data_ptr, STR_QIURC) || g_data_in_ec20)
				{
						g_ec20_state = EC20_STATE_DATA_READ;
						gprs_at_cmd(g_dev_ec20,qird);
						server_len_ec20 = 0;
				}
				break;	
			case EC20_STATE_CHECK_SENT:
				if (have_str(last_data_ptr, STR_QISEND)) {
					g_ec20_state = EC20_STATE_DATA_PROCESSING;
					rt_hw_led_on(NET_LED);
					gprs_at_cmd(g_dev_ec20,at_csq);
					rt_time_t cur_time = time(RT_NULL);
					rt_kprintf("send server ok ======> %s\r\n",ctime(&cur_time));
					
					rt_event_send(&(g_pcie[g_index]->event), EC20_EVENT_0);
				} else {
					gprs_at_cmd(g_dev_ec20, at_qisend);
				}
		}
	}
}
