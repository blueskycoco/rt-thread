#include <stdio.h>
#include <unistd.h> 
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"
#include <ctype.h>

#define M26_EVENT_0 				(1<<0)
#define M26_STATE_INIT				0
#define M26_STATE_CHECK_CPIN		1
#define M26_STATE_SET_QINDI			2
#define M26_STATE_SET_QIMUX			3
#define M26_STATE_CHECK_CGREG		4
#define M26_STATE_CHECK_QISTAT		5
#define M26_STATE_SET_QICSGP		6
#define M26_STATE_SET_QIREGAPP		7
#define M26_STATE_SET_QISRVC		8
#define M26_STATE_SET_QIACT			9
#define M26_STATE_CHECK_CIMI		10
#define M26_STATE_SET_QIFGCNT		11
#define M26_STATE_SET_QIOPEN 		12
#define M26_STATE_SET_QIDEACT  		13
#define M26_STATE_DATA_PROCESSING 	14
#define M26_STATE_DATA_READ			15
#define M26_STATE_SET_QICLOSE		16
#define M26_STATE_DATA_WRITE		17
#define M26_STATE_DATA_ACK			18
#define M26_STATE_DATA_PRE_WRITE	19
#define M26_STATE_ATE0 				21
#define M26_STATE_CSQ				22
#define M26_STATE_LAC				23
#define M26_STATE_ICCID				24
#define M26_STATE_IMEI				25
#define M26_STATE_LACR				26
#define M26_STATE_IFGCNT			27
#define M26_STATE_CFG_FTP		28
#define M26_STATE_OPEN_FTP		29
#define M26_STATE_GET_FILE		30
#define M26_STATE_READ_FILE	31
#define M26_STATE_LOGOUT_FTP	32
#define M26_STATE_SET_LOCATION	33
#define M26_STATE_V				34
#define M26_STATE_CLEAN_RAM		35
#define M26_STATE_CLOSE_FILE	36

#define STR_CFUN					"+CFUN:"
#define STR_RDY						"RDY"
#define STR_CPIN					"+CPIN:"
#define STR_CSQ						"+CSQ:"
#define STR_CREG					"+CREG:"
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
#define STR_QIURC					"+QIURC:"
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
#define STR_CONNECT_OK				"CONNECT OK"
#define STR_CLOSE_OK				"CLOSE OK"
#define STR_SEND_OK					"SEND OK"
#define STR_QIRDI					"+QIRDI: 0,1,0"
#define STR_QISACK					"+QISACK"
#define STR_SOCKET_BUSSY			"SOCKET BUSY"
#define STR_CONNECT_FAIL			"CONNECT FAIL"
#define STR_CONNECT_OK_EC20			"+QIOPEN: 0,0"
#define STR_CONNECT_FAIL_EC20		"+QIOPEN: 0,"
#define STR_QFTPCFG					"+QFTPCFG:0"
#define STR_FTP_FILE_SIZE			"+QFTPSIZE:"
#define DEFAULT_SERVER				"101.132.177.116"
#define DEFAULT_PORT				"2011"

#define qistat 				"AT+QISTAT\r\n"
#define qiclose "AT+QICLOSE\r\n"
#define qilocip "AT+QILOCIP\r\n"
#define qilport "AT+QILPORT?\r\n"
#define e0 "ATE0\r\n"
#define cgreg "AT+CGREG?\r\n"
#define cregs "AT+CREG=2\r\n"
#define cregr "AT+CREG?\r\n"
#define at_csq "AT+CSQ\r\n"
#define at_qccid "AT+QCCID\r\n"
#define gsn "AT+GSN\r\n"

#define cimi "AT+CIMI\r\n"
#define cpin "AT+CPIN?\r\n"
#define qifgcnt "AT+QIFGCNT=0\r\n"
#define qisrvc "AT+QISRVC=1\r\n"
#define qimux "AT+QIMUX=0\r\n"
#define ask_qimux "AT+QIMUX?\r\n"
#define qideact "AT+QIDEACT\r\n"
#define ati	"ATI\r\n"
#define qindi "AT+QINDI=1\r\n"
#define qird "AT+QIRD=0,1,0,1500\r\n"
#define qisack "AT+QISACK\r\n"
#define qiat "AT\r\n"
#define qiftp_set_path	"AT+QFTPPATH=\"/\"\r\n"
#define qiftp_file_size	"AT+QFTPSIZE=\"stm32_0.bin\"\r\n"
#define qiftp_clean_ram "AT+QFDEL=\"RAM:*\"\r\n"
uint8_t 	  qicsgp_m26[32]			= {0};
uint8_t 	  qiopen_m26[64]			= {0};
uint8_t 	  qisend_m26[32] 			= {0};
uint8_t 	  qiftp_set_resuming[32] 	= {0};
uint8_t 	  qiftp_m26_ram[32]			= {0};
uint8_t		  qiftp_get_m26[32]			= {0};
#define qiregapp "AT+QIREGAPP\r\n"
#define qiact "AT+QIACT\r\n"
rt_bool_t g_data_in_m26 = RT_FALSE;
extern int g_index;
rt_uint32_t ftp_ofs = 0;
rt_device_t g_dev_m26;
uint8_t g_m26_state 				= M26_STATE_INIT;
uint32_t server_len_m26 = 0;
uint8_t *server_buf_m26 = RT_NULL;
void *send_data_ptr_m26 = RT_NULL;
rt_size_t send_size_m26;
rt_uint8_t m26_cnt = 0;
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
uint8_t 	  qiftp_m26[64];
extern uint8_t		  qiftp_read_file[32];//		"AT+QFREAD=\"RAM:stm32.bin\",0\r\n"
extern uint8_t			qiftp_close_file[32];
extern rt_uint8_t ftp_rty;
rt_uint16_t g_app_v=0;
extern struct rt_event g_info_event;
rt_uint32_t bak_server_len_m26 = 0;
rt_uint8_t test_buf[128] = {0};
extern rt_mp_t server_mp;
extern rt_uint8_t 	cur_status;
void handle_m26_server_in(const void *last_data_ptr,rt_size_t len)
{
	static rt_bool_t flag = RT_FALSE;	
	int i;
	int ofs;
	static int cnt = 0;
	if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI))!=-1) {
		rt_kprintf("got another qirdi\r\n");
		gprs_at_cmd(g_dev_m26,qird);
		server_len_m26 = 0;
		g_data_in_m26 = RT_TRUE;
		flag=RT_FALSE;
		return ;
	}
	if (match_bin((rt_uint8_t *)last_data_ptr,len, STR_OK,rt_strlen(STR_OK)) != -1 && 
			(match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)) == -1)&& 
			(match_bin((rt_uint8_t *)last_data_ptr, len,STR_CSQ,rt_strlen(STR_CSQ))==-1) &&
			!flag) {
		//for (i=0;i<len;i++)
		//	rt_kprintf("%c",((rt_uint8_t *)last_data_ptr)[i]);
		//rt_kprintf("m26 read again\r\n");
		cnt++;
		if (cnt == 30) {
			g_m26_state = M26_STATE_DATA_PROCESSING;
			gprs_at_cmd(g_dev_m26,at_csq);
			g_data_in_m26 = RT_FALSE;
		} else {
			gprs_at_cmd(g_dev_m26,qird);
			g_data_in_m26 = RT_TRUE;
		}
		server_len_m26 = 0;

		return ;
	}

	cnt=0;
	if (/*have_str(last_data_ptr, STR_TCP)*/(ofs = match_bin((rt_uint8_t *)last_data_ptr, len,STR_TCP,rt_strlen(STR_TCP)))!=-1)
	{	
#if 0
		uint8_t *begin = (uint8_t *)strstr(last_data_ptr,STR_QIRD);
		uint8_t *pos = RT_NULL;
		if (begin == RT_NULL)
			pos = (uint8_t *)strstr(last_data_ptr, STR_TCP);
		else
			pos = (uint8_t *)strstr(begin, STR_TCP);
		if (pos != RT_NULL) {
			int i = 4;
			while (pos[i] != '\r' && pos[i+1] != '\n' && i<strlen(pos))
			{
				server_len_m26 = server_len_m26*10 + pos[i] - '0';
				i++;
			}
			//rt_kprintf("server len m26 is %d\r\n",server_len_m26);
			if(strlen(pos)<server_len_m26)
			{
				g_m26_state = M26_STATE_DATA_PROCESSING;
				gprs_at_cmd(g_dev_m26,at_csq);
				rt_mutex_release(&(g_pcie[g_index]->lock));
				return;
			}
			server_buf_m26 = (uint8_t *)rt_malloc(server_len_m26 * sizeof(uint8_t));
			rt_memset(server_buf_m26,0,server_len_m26);
			server_len_m26 = 0;
			i+=2;
			while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{

				server_buf_m26[server_len_m26++] = pos[i++];
				//rt_kprintf("%c",server_buf_m26[server_len_m26-1]);
			}
			if (strstr(pos, "OK")!=RT_NULL)
			{
				rt_data_queue_push(&g_data_queue[3], server_buf_m26, server_len_m26, RT_WAITING_FOREVER);
				if (server_len_m26 == 1500) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				} else {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_m26,at_csq);
				}
				if (!have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_FALSE;

				rt_mutex_release(&(g_pcie[g_index]->lock));
				flag = RT_FALSE;
			}
			else
				flag = RT_TRUE;
		}
#else
		uint8_t *pos = (uint8_t *)last_data_ptr;
		//for(i=0;i<256;i++)
		//	rt_kprintf("%c",pos[i]);
		rt_kprintf("ofs is %d\r\n",ofs);
		//if (pos != RT_NULL) {
		int i = 4+ofs;
		//rt_kprintf("\r\n<>%x%x%x%x%x%x%x%x%x%x%x%x<>\r\n",
		//	pos[ofs],pos[ofs+1],pos[ofs+2],pos[ofs+3],pos[ofs+4],pos[ofs+5],pos[ofs+6],pos[ofs+7],
		//	pos[ofs+8],pos[ofs+9],pos[ofs+10],pos[ofs+11]);
		while (pos[i] != '\r' && pos[i+1] != '\n' && i<len)
		{
			server_len_m26 = server_len_m26*10 + pos[i] - '0';
			i++;
		}
		//server_len_ec20 = get_len(pos+i,len-i);
		rt_kprintf("server len %d\r\n", server_len_m26);
		if (len > server_len_m26) {
			if ((len - server_len_m26) != 44)
				rt_kprintf("cut-short message\r\n");
		} else
			rt_kprintf("short-cut message %d %d\r\n", len, server_len_m26);
		//server_buf_m26 = (uint8_t *)rt_malloc(server_len_m26 * sizeof(uint8_t));
		
		server_buf_m26 = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
		if (server_buf_m26 == RT_NULL)
		{
			g_m26_state = M26_STATE_DATA_PROCESSING;
			gprs_at_cmd(g_dev_m26,at_csq);
			show_memory_info();
			rt_kprintf("malloc buf 26 failed %d\r\n",server_len_m26);
			return ;
		}
		rt_memset(server_buf_m26,0,server_len_m26);
		//server_len_ec20 = 0;
		i+=2;
		if (i+server_len_m26 < len)
			rt_memcpy(server_buf_m26,pos+i,server_len_m26);
		else
		{
			bak_server_len_m26 = server_len_m26;
			server_len_m26 = i+server_len_m26-len;
			rt_memcpy(server_buf_m26,pos+i,server_len_m26);
			rt_kprintf("copy1 %d byte\r\n",server_len_m26);
		}
		/*while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
		  {

		  server_buf_ec20[server_len_ec20++] = pos[i++];
		  rt_kprintf("<%02x>\r\n", server_buf_ec20[server_len_ec20-1]);
		  }*/
		if (match_bin(pos, len, "OK",rt_strlen("OK"))!=-1)
		{
			rt_data_queue_push(&g_data_queue[3], server_buf_m26, server_len_m26, RT_WAITING_FOREVER);
			if (server_len_m26 == 1500) {
				g_m26_state = M26_STATE_CHECK_QISTAT;
				gprs_at_cmd(g_dev_m26,qistat);
			} else {
				g_m26_state = M26_STATE_DATA_PROCESSING;
				gprs_at_cmd(g_dev_m26,at_csq);
			}
			if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI))==-1)
				g_data_in_m26 = RT_FALSE;

			flag = RT_FALSE;
		}
		else {
			rt_kprintf("waiting for second part %d\r\n",server_len_m26);
			flag = RT_TRUE;
		}
#endif
	}
		else if (flag){	
			int i=0;
			uint8_t *pos = (uint8_t *)last_data_ptr;
			while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{
				server_buf_m26[server_len_m26++] = pos[i++];
				if (server_len_m26 == bak_server_len_m26)
					break;
			}

			if (match_bin((rt_uint8_t *)pos, len,"OK",2)!=-1)
			{
				rt_kprintf("got the second parts %d\r\n",server_len_m26);
				rt_data_queue_push(&g_data_queue[3], server_buf_m26, server_len_m26, RT_WAITING_FOREVER);
				if (server_len_m26 == 1500) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				} else {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_m26,at_csq);		
				}
				if (!have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_FALSE;
				//rt_mutex_release(&(g_pcie[g_index]->lock));
				flag = RT_FALSE;
			}

		}else {

		}
}

void m26_start(int index)
{
	rt_uint32_t power_rcc,pwr_key_rcc;
	rt_uint16_t power_pin,pwr_key_pin;
	GPIO_TypeDef* GPIO_power,*GPIO_pwr;
	GPIO_InitTypeDef GPIO_InitStructure;
	g_index = index;
	g_dev_m26 = g_pcie[index]->dev;
	if (index) {
		power_rcc = RCC_APB2Periph_GPIOE;
		pwr_key_rcc = RCC_APB2Periph_GPIOE;
		power_pin = GPIO_Pin_14;
		pwr_key_pin = GPIO_Pin_14;
		GPIO_power = GPIOE;
		GPIO_pwr = GPIOE;
	} else {
		power_rcc = RCC_APB2Periph_GPIOC;
		pwr_key_rcc = RCC_APB2Periph_GPIOC;
		power_pin = GPIO_Pin_7;
		pwr_key_pin = GPIO_Pin_7;
		GPIO_power = GPIOC;
		GPIO_pwr = GPIOC;
		rt_kprintf("use pcie0 gpioc pin 7\r\n");
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
	rt_kprintf("m26 power on done\r\n");

	strcpy(ftp_addr,mp.updateDomainAddress.domain);//"u.110LW.com");
	//strcpy(ftp_addr,"47.93.48.167");
	strcpy(ftp_user,"minfei");
	strcpy(ftp_passwd,"minfei123");
	g_m26_state = M26_STATE_INIT;
}

void m26_proc(void *last_data_ptr, rt_size_t data_size)
{
	int i=0;
	rt_uint8_t *tmp = (rt_uint8_t *)last_data_ptr;
#if 0
	if (data_size != 6 && strstr(last_data_ptr, STR_QIRD)==NULL && strstr(last_data_ptr, STR_QIURC)==NULL && 
			!have_str(last_data_ptr,STR_CSQ))
		if (!have_str(last_data_ptr,STR_CONNECT))
			rt_kprintf("\r\n(M26<= %d %d %d %s)\r\n",g_m26_state, data_size,rt_strlen(last_data_ptr), last_data_ptr);
		else
			rt_kprintf("\r\n(M26<= %d %d)\r\n",g_m26_state, data_size);
#else
	if (!have_str(last_data_ptr,STR_CSQ)) {
		rt_kprintf("\r\n<== (M26 %d %d)\r\n",g_m26_state, data_size);
		for (i=0; i<data_size; i++)
			if (isascii(tmp[i]) && (g_m26_state != M26_STATE_READ_FILE))
				rt_kprintf("%c", tmp[i]);
			else
				break;
	}
#endif
	if (data_size >= 2) {
		if (have_str(last_data_ptr,STR_RDY)||have_str(last_data_ptr,STR_CFUN))
		{
			g_m26_state = M26_STATE_INIT;
		}
		switch (g_m26_state) {
			case M26_STATE_INIT:
				if (have_str(last_data_ptr,STR_RDY)||have_str(last_data_ptr,STR_CFUN)) {
					g_m26_state = M26_STATE_ATE0;
					gprs_at_cmd(g_dev_m26,e0);	
				}
				break;
			case M26_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					g_m26_state = M26_STATE_V;
					gprs_at_cmd(g_dev_m26,ati);
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_m26,e0);
				}
				break;
			case M26_STATE_V:
				if (have_str(last_data_ptr,STR_OK)) {
					g_pcie[g_index]->cpin_cnt=0;
					g_m26_state = M26_STATE_CHECK_CPIN;
					gprs_at_cmd(g_dev_m26,cpin);
				}
				break;
			case M26_STATE_CHECK_CPIN:
				if (have_str(last_data_ptr,STR_CPIN_READY)) {
					g_pcie[g_index]->cpin_cnt=0;
					g_m26_state = M26_STATE_CHECK_CGREG;
					gprs_at_cmd(g_dev_m26,cgreg);
				} 
				else/* if (have_str(last_data_ptr, STR_CPIN))*/
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
					gprs_at_cmd(g_dev_m26,cpin);
				}
				break;
			case M26_STATE_CSQ:
				if (have_str(last_data_ptr,STR_CSQ)) {
					if (tmp[9] == 0x2c)
						g_pcie[g_index]->csq = tmp[8]-0x30;
					else
						g_pcie[g_index]->csq = (tmp[8]-0x30)*10+tmp[9]-0x30;
					rt_kprintf("csq is %x %x %d\r\n",tmp[8],tmp[9],g_pcie[g_index]->csq);
					show_signal(g_pcie[g_index]->csq);
					//g_m26_state = M26_STATE_LAC;
					//gprs_at_cmd(g_dev_m26,cregs);
					gprs_at_cmd(g_dev_m26,qifgcnt);
					g_m26_state = M26_STATE_IFGCNT;
				} 
				break;
			case M26_STATE_IFGCNT:
				g_m26_state = M26_STATE_LAC;
				gprs_at_cmd(g_dev_m26,cregs);
				break;
			case M26_STATE_LAC:
				if (have_str(last_data_ptr,STR_OK)) {
					g_m26_state = M26_STATE_LACR;
					gprs_at_cmd(g_dev_m26,cregr);
				} 
				break;
			case M26_STATE_LACR:
				if (have_str(last_data_ptr,STR_CREG)) {
					if (((rt_uint8_t *)last_data_ptr)[14]>='A' && ((rt_uint8_t *)last_data_ptr)[14]<='F' )
						g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[14]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[14]-'0';

					if (((rt_uint8_t *)last_data_ptr)[15]>='A' && ((rt_uint8_t *)last_data_ptr)[15]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[15]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[15]-'0';
					if (((rt_uint8_t *)last_data_ptr)[16]>='A' && ((rt_uint8_t *)last_data_ptr)[16]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[16]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[16]-'0';
					if (((rt_uint8_t *)last_data_ptr)[17]>='A' && ((rt_uint8_t *)last_data_ptr)[17]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[17]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[17]-'0';
					if (((rt_uint8_t *)last_data_ptr)[21]>='A' && ((rt_uint8_t *)last_data_ptr)[21]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[21]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[21]-'0';
					if (((rt_uint8_t *)last_data_ptr)[22]>='A' && ((rt_uint8_t *)last_data_ptr)[22]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[22]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[22]-'0';
					if (((rt_uint8_t *)last_data_ptr)[23]>='A' && ((rt_uint8_t *)last_data_ptr)[23]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[23]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[23]-'0';
					if (((rt_uint8_t *)last_data_ptr)[24]>='A' && ((rt_uint8_t *)last_data_ptr)[24]<='F' )
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[24]-'A'+10;
					else
						g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[24]-'0';
					rt_kprintf("ORI %c%c%c%c%c%c%c%c LAC_CI %08x\r\n", 
							((rt_uint8_t *)last_data_ptr)[14],((rt_uint8_t *)last_data_ptr)[15],
							((rt_uint8_t *)last_data_ptr)[16],((rt_uint8_t *)last_data_ptr)[17],
							((rt_uint8_t *)last_data_ptr)[21],((rt_uint8_t *)last_data_ptr)[22],
							((rt_uint8_t *)last_data_ptr)[23],((rt_uint8_t *)last_data_ptr)[24],
							g_pcie[g_index]->lac_ci);
				} 
				g_m26_state = M26_STATE_ICCID;
				gprs_at_cmd(g_dev_m26,at_qccid);
				break;
			case M26_STATE_ICCID:
				i=2;
				while(tmp[i]!='\r')
				{
					if (tmp[i]>='0' && tmp[i]<='9')
						g_pcie[g_index]->qccid[i/2-1] = (tmp[i]-0x30)*16;
					else if (tmp[i]>='a' && tmp[i]<='f')
						g_pcie[g_index]->qccid[i/2-1] = (tmp[i]-'a'+10)*16;
					else if (tmp[i]>='A' && tmp[i]<='F')
						g_pcie[g_index]->qccid[i/2-1] = (tmp[i]-'A'+10)*16;

					if (tmp[i+1]>='0' && tmp[i+1]<='9')
						g_pcie[g_index]->qccid[i/2-1] += (tmp[i+1]-0x30);
					else if (tmp[i+1]>='a' && tmp[i+1]<='f')
						g_pcie[g_index]->qccid[i/2-1] += (tmp[i+1]-'a'+10);
					else if (tmp[i+1]>='A' && tmp[i+1]<='F')
						g_pcie[g_index]->qccid[i/2-1] += (tmp[i+1]-'A'+10);
					rt_kprintf("qccid[%d] = %02X\r\n",i/2,g_pcie[g_index]->qccid[i/2-1]);
					i+=2;						
				}					
				g_m26_state = M26_STATE_IMEI;
				gprs_at_cmd(g_dev_m26,gsn);
				break;
			case M26_STATE_IMEI:
				g_pcie[g_index]->imei[0]=0x0;
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
				g_m26_state = M26_STATE_CHECK_QISTAT;
				gprs_at_cmd(g_dev_m26,qistat);
				break;
			case M26_STATE_CHECK_CGREG:
				if (have_str(last_data_ptr, STR_CGREG_READY) ||
						have_str(last_data_ptr, STR_CGREG_READY1)) {
					//g_m26_state = M26_STATE_CHECK_QISTAT;
					g_m26_state = M26_STATE_CSQ;
					gprs_at_cmd(g_dev_m26,at_csq);
				} else if(have_str(last_data_ptr, STR_CGREG)) {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_m26,cgreg);
				}
				break;
			case M26_STATE_CHECK_QISTAT:
				if (have_str(last_data_ptr, STR_STAT_INIT) ||
						have_str(last_data_ptr, STR_STAT_DEACT)) {
					g_m26_state = M26_STATE_SET_QIMUX;
					gprs_at_cmd(g_dev_m26,qimux);
				} else if (have_str(last_data_ptr, STR_STAT_IND)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);
				} else if (have_str(last_data_ptr, STR_CONNECT_OK)){
					g_m26_state = M26_STATE_SET_QICLOSE;
					gprs_at_cmd(g_dev_m26,qiclose);
				} else if (have_str(last_data_ptr, STR_STAT_CLOSE) ||
						have_str(last_data_ptr, STR_STAT_STATUS) ||
						have_str(last_data_ptr,STR_CONNECT_FAIL)){
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen_m26, 0, 64);
					rt_sprintf(qiopen_m26, "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r\n",
							mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
							mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
							mp.socketAddress[g_ip_index].port);
					rt_kprintf("state ip index %d\r\n",g_ip_index);
					gprs_at_cmd(g_dev_m26,qiopen_m26);
				}			
				break;
				/******************************************************************************************************************************************/				
			case M26_STATE_CFG_FTP:
				{
					if (ftp_cfg_step == 0) {
						rt_sprintf(qiftp_m26,"AT+QFTPPASS=\"%s\"\r\n", ftp_passwd);
						gprs_at_cmd(g_dev_m26,qiftp_m26);
					} else if (ftp_cfg_step == 1) {
						rt_sprintf(qiftp_m26,"AT+QFTPOPEN=\"%s\",%d\r\n", ftp_addr,mp.updateDomainAddress.port);
						gprs_at_cmd(g_dev_m26,qiftp_m26);
						g_m26_state = M26_STATE_OPEN_FTP;
					}
					ftp_cfg_step++;
				}
				break;
			case M26_STATE_OPEN_FTP:
				{
					if (have_str(last_data_ptr, STR_FTP_OK_M26)) {
						rt_kprintf("login ftp ok\r\n");
						m26_cnt = 0;
						sprintf(qiftp_m26_ram, "AT+QFTPCFG=4,\"/RAM/stm32_%d.bin\"\r\n",m26_cnt);
						gprs_at_cmd(g_dev_m26,qiftp_m26_ram);
						g_m26_state = M26_STATE_SET_LOCATION;
						ftp_rty=0;			
						down_fd = open("/stm32.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);			
					} else {
						if (!have_str(last_data_ptr, STR_OK) || have_str(last_data_ptr, STR_FTP_FAILED)){
							rt_thread_delay(100);
							gprs_at_cmd(g_dev_m26,qiftp_m26);
							ftp_rty++;
							if (ftp_rty>5)
							{
								entering_ftp_mode=0;
								g_m26_state = M26_STATE_CHECK_QISTAT;
								gprs_at_cmd(g_dev_m26,qistat);
							}
						}
					}
				}
				break;
			case M26_STATE_SET_LOCATION:
				if (have_str(last_data_ptr, STR_QFTPCFG)) {					
					gprs_at_cmd(g_dev_m26,qiftp_clean_ram);
					g_m26_state = M26_STATE_CLEAN_RAM;
				}
				break;
			case M26_STATE_CLEAN_RAM:
				sprintf(qiftp_get_m26, "AT+QFTPGET=\"stm32_%d.bin\"\r\n",m26_cnt);
				gprs_at_cmd(g_dev_m26, qiftp_get_m26);
				g_m26_state = M26_STATE_GET_FILE;
				break;
			case M26_STATE_CLOSE_FILE:
				if (have_str(last_data_ptr, STR_OK)) {
					m26_cnt++;
					sprintf(qiftp_m26_ram, "AT+QFTPCFG=4,\"/RAM/stm32_%d.bin\"\r\n",m26_cnt);
					gprs_at_cmd(g_dev_m26,qiftp_m26_ram);
					g_m26_state = M26_STATE_SET_LOCATION;
				}
				break;
			case M26_STATE_GET_FILE:
				if (have_str(last_data_ptr, STR_QFTPGET_M26)) {					
					stm32_len = get_len(strstr(last_data_ptr, STR_QFTPGET_M26)+strlen(STR_QFTPGET_M26),data_size-strlen(STR_QFTPGET_M26));
					rt_kprintf("get stm32 len %d\r\n", stm32_len);		
					sprintf(qiftp_m26_ram,"AT+QFOPEN=\"RAM:stm32_%d.bin\",0\r\n",m26_cnt);
					gprs_at_cmd(g_dev_m26,qiftp_m26_ram);
					g_m26_state = M26_STATE_READ_FILE;
					stm32_fd=0;			
				}
				break;
			case M26_STATE_READ_FILE:
				if (stm32_fd == 0) {
					if (have_str(last_data_ptr, STR_QFOPEN)) {
						stm32_fd = get_len(strstr(last_data_ptr, STR_QFOPEN)+strlen(STR_QFOPEN),data_size-strlen(STR_QFOPEN));								
						sprintf(qiftp_read_file,"AT+QFREAD=%d,1024\r\n", stm32_fd);
						cur_stm32_len=0;
						rt_kprintf("get stm32 fd %d\r\n", stm32_fd);
						gprs_at_cmd(g_dev_m26,qiftp_read_file);	
						tmp_stm32_bin = (rt_uint8_t *)rt_malloc(1500*sizeof(rt_uint8_t));
						if (tmp_stm32_bin == RT_NULL)
						{	rt_kprintf("stm32 bin malloc failed\r\n");
							show_memory_info();}
						rt_memset(tmp_stm32_bin,0,1500);
						tmp_stm32_len=0;
					}		
				} else {
					rt_uint8_t *ptr = (rt_uint8_t *)last_data_ptr;					
					rt_uint16_t cnt=0;

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
						tmp_stm32_len=0;
						rt_memset(tmp_stm32_bin,0,1500);		
						if (cur_stm32_len < stm32_len) {		
							gprs_at_cmd(g_dev_m26,qiftp_read_file);									
							rt_hw_led_on(NET_LED);
						} else {
							rt_free(tmp_stm32_bin);
							//cat_file("/stm32.bin");
							sprintf(qiftp_close_file,"AT+QFCLOSE=%d\r\n",stm32_fd);
							gprs_at_cmd(g_dev_m26,qiftp_close_file);
							if (m26_cnt == 2) {
								g_m26_state = M26_STATE_LOGOUT_FTP;
							} else {
								g_m26_state = M26_STATE_CLOSE_FILE;
							}
						}	
					}
				}				
				break;
			case M26_STATE_LOGOUT_FTP:
				if (have_str(last_data_ptr, STR_OK))
				{
					close(down_fd);
					mp.firmCRC = CRC_check_file("/stm32.bin");
					if (g_app_v!=0)
						mp.firmVersion = g_app_v;
					mp.firmLength = stm32_len;
					rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
					if (!cur_status) {
					rt_thread_sleep(500);
					NVIC_SystemReset();
					}
					gprs_at_cmd(g_dev_m26,qiftp_close);
					g_m26_state = M26_STATE_SET_QIACT;
					entering_ftp_mode=0;
				}
				break;

			case M26_STATE_SET_QICLOSE:				
				if (entering_ftp_mode) {
					ftp_cfg_step = 0;
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);
				} else {
					if (have_str(last_data_ptr, STR_CLOSE_OK)) {
						g_m26_state = M26_STATE_SET_QIOPEN;
						rt_memset(qiopen_m26, 0, 64);
						rt_sprintf(qiopen_m26, "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r\n",
								mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
								mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
								mp.socketAddress[g_ip_index].port);
						rt_kprintf("close ip index %d\r\n",g_ip_index);
						gprs_at_cmd(g_dev_m26,qiopen_m26);
					}
					else
					{
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_m26,qistat);
					}
				}
				break;
				/******************************************************************************************************************************************/				
			case M26_STATE_SET_QINDI:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_CHECK_CIMI;
					gprs_at_cmd(g_dev_m26,cimi);
				}
				break;								
			case M26_STATE_SET_QIMUX:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_m26_state = M26_STATE_SET_QINDI;
					gprs_at_cmd(g_dev_m26,qindi);
				}
				break;		
			case M26_STATE_CHECK_CIMI:
				rt_memset(qicsgp_m26, 0, 32);
				if (have_str(last_data_ptr, STR_4600)) {
					if (have_str(last_data_ptr, STR_46000) ||
							have_str(last_data_ptr, STR_46002) ||
							have_str(last_data_ptr, STR_46004)) {
						rt_sprintf(qicsgp_m26, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
					} else if (have_str(last_data_ptr, STR_46001) ||
							have_str(last_data_ptr, STR_46006)){						
						rt_sprintf(qicsgp_m26, "AT+QICSGP=1,\"%s\"\r\n", "UNINET");
					} else if (have_str(last_data_ptr, STR_46003)){						
						rt_sprintf(qicsgp_m26, "AT+QICSGP=1,\"%s\"\r\n", "CTNET");
					} else {						
						rt_sprintf(qicsgp_m26, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
					}
					g_m26_state = M26_STATE_SET_QICSGP;
					gprs_at_cmd(g_dev_m26,qicsgp_m26);
				}
				else
					gprs_at_cmd(g_dev_m26,cimi);
				break;						
			case M26_STATE_SET_QICSGP:
				if (have_str(last_data_ptr, STR_OK)) {					
					g_m26_state = M26_STATE_SET_QIREGAPP;
					gprs_at_cmd(g_dev_m26,qiregapp);					
				}
				break;		
			case M26_STATE_SET_QIREGAPP:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_m26_state = M26_STATE_SET_QISRVC;
					gprs_at_cmd(g_dev_m26,qisrvc);
				}
				break;
			case M26_STATE_SET_QISRVC:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_SET_QIACT;
					gprs_at_cmd(g_dev_m26,qiact);
				}
				break;		
			case M26_STATE_SET_QIACT:
				if (entering_ftp_mode) {
					g_m26_state = M26_STATE_CFG_FTP;
					rt_sprintf(qiftp_m26,"AT+QFTPUSER=\"%s\"\r\n", ftp_user);
					gprs_at_cmd(g_dev_m26,qiftp_m26);
				} else {
					if (have_str(last_data_ptr, STR_OK)) {
						g_m26_state = M26_STATE_SET_QIOPEN;
						rt_memset(qiopen_m26, 0, 64);					
						rt_sprintf(qiopen_m26, "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r\n",
								mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
								mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
								mp.socketAddress[g_ip_index].port);
						rt_kprintf("open ip index %d\r\n",g_ip_index);
						gprs_at_cmd(g_dev_m26,qiopen_m26);
					} else {
						/*check error condition*/
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_m26,qistat);
					}
				}
				break;
			case M26_STATE_SET_QIDEACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_SET_QIREGAPP;
					gprs_at_cmd(g_dev_m26,qiregapp);
				} else if (have_str(last_data_ptr, STR_ERROR)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);
				}
				break;						
			case M26_STATE_SET_QIOPEN:
				if (have_str(last_data_ptr, STR_CONNECT_OK)) {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					/*send data here */
					//	rt_kprintf("connect to server ok\r\n");
					//	rt_event_send(&(g_pcie[g_index]->event), M26_EVENT_0);
					//	gprs_at_cmd(g_dev_m26,at_csq);
					//	rt_hw_led_on(NET_LED);


					/*send data here */
					g_server_addr = (mp.socketAddress[g_ip_index].IP[0] << 24)|
						(mp.socketAddress[g_ip_index].IP[1] << 16)|
						(mp.socketAddress[g_ip_index].IP[2] <<  8)|
						(mp.socketAddress[g_ip_index].IP[3] <<  0);
					g_server_port = mp.socketAddress[g_ip_index].port;
					g_server_addr_bak = g_server_addr;
					g_server_port_bak = g_server_port;					
					rt_kprintf("connect to server ok\r\n");
					g_heart_cnt=0;
					g_net_state = NET_STATE_INIT;
					rt_event_send(&(g_pcie[g_index]->event), M26_EVENT_0);
					gprs_at_cmd(g_dev_m26,at_csq);
					rt_hw_led_on(NET_LED);
					//entering_ftp_mode=1;
				} else if (have_str(last_data_ptr, STR_SOCKET_BUSSY)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);					

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
				else {
					if (!have_str(last_data_ptr, STR_OK)) {
						rt_thread_delay(100*3);
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_m26,qistat);
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
					}
				}

				break;
			case M26_STATE_DATA_PROCESSING:
				if (have_str(last_data_ptr, STR_QIRDI)||
						have_str(last_data_ptr, STR_QIURC) ||g_data_in_m26) {
					/*server data in */					
					//rt_mutex_take(&(g_pcie[g_index]->lock), RT_WAITING_FOREVER);
					g_m26_state = M26_STATE_DATA_READ;
					gprs_at_cmd(g_dev_m26,qird);
					server_len_m26 = 0;
					g_data_in_m26 = RT_TRUE;
					//} else if (have_str(last_data_ptr, STR_OK)){
		} else if (have_str(last_data_ptr,STR_CSQ)) {
			if (tmp[9] == 0x2c)
				g_pcie[g_index]->csq = tmp[8]-0x30;
			else
				g_pcie[g_index]->csq = (tmp[8]-0x30)*10+tmp[9]-0x30;
			//rt_kprintf("csq is %x %x %d\r\n",tmp[8],tmp[9],g_pcie[g_index]->csq);
			show_signal(g_pcie[g_index]->csq);


			if (g_server_addr != g_server_addr_bak || g_server_port != g_server_port_bak) {
				g_ip_index=0;
				g_m26_state = M26_STATE_SET_QICLOSE;
				gprs_at_cmd(g_dev_m26,qiclose);
				break;
			}
			if (g_heart_cnt > 5 || entering_ftp_mode) {
				g_m26_state = M26_STATE_SET_QICLOSE;
				gprs_at_cmd(g_dev_m26,qiclose);
				break;
			}

			/*check have data to send */
			//if (rt_data_queue_peak(&g_data_queue[2],(const void **)&send_data_ptr_m26,&send_size_m26) == RT_EOK)
			if (rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_m26, &send_size_m26, 0) == RT_EOK)
			{	
				//rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_m26, &send_size_m26, RT_WAITING_FOREVER);
				//rt_kprintf("should send data %d\r\n", send_size_m26);			
				rt_memcpy(test_buf, send_data_ptr_m26,send_size_m26);
				rt_sprintf(qisend_m26, "AT+QISEND=%d\r\n", send_size_m26);
				gprs_at_cmd(g_dev_m26,qisend_m26);
				g_m26_state = M26_STATE_DATA_PRE_WRITE;
				if (send_data_ptr_m26) {
					if (send_size_m26 <= 64) {
						rt_mp_free(send_data_ptr_m26);
					} else {
						rt_free(send_data_ptr_m26);
					}
					send_data_ptr_m26 = RT_NULL;
				}
			} else {								
				rt_thread_delay(100);
				gprs_at_cmd(g_dev_m26,at_csq);
			}
		} else if (have_str(last_data_ptr,STR_CREG)) {
			if (((rt_uint8_t *)last_data_ptr)[12]>='A' && ((rt_uint8_t *)last_data_ptr)[12]<='F' )
				g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[12]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = ((rt_uint8_t *)last_data_ptr)[12]-'0';

			if (((rt_uint8_t *)last_data_ptr)[13]>='A' && ((rt_uint8_t *)last_data_ptr)[13]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[13]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[13]-'0';
			if (((rt_uint8_t *)last_data_ptr)[14]>='A' && ((rt_uint8_t *)last_data_ptr)[14]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[14]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[14]-'0';
			if (((rt_uint8_t *)last_data_ptr)[15]>='A' && ((rt_uint8_t *)last_data_ptr)[15]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[15]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[15]-'0';
			if (((rt_uint8_t *)last_data_ptr)[19]>='A' && ((rt_uint8_t *)last_data_ptr)[19]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[19]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[19]-'0';
			if (((rt_uint8_t *)last_data_ptr)[20]>='A' && ((rt_uint8_t *)last_data_ptr)[20]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[20]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[20]-'0';
			if (((rt_uint8_t *)last_data_ptr)[21]>='A' && ((rt_uint8_t *)last_data_ptr)[21]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[21]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[21]-'0';
			if (((rt_uint8_t *)last_data_ptr)[22]>='A' && ((rt_uint8_t *)last_data_ptr)[22]<='F' )
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[22]-'A'+10;
			else
				g_pcie[g_index]->lac_ci = g_pcie[g_index]->lac_ci*16 + ((rt_uint8_t *)last_data_ptr)[22]-'0';
			rt_kprintf("LAC_CI %08x\r\n", g_pcie[g_index]->lac_ci);
			gprs_at_cmd(g_dev_m26,at_csq);
		} 
		else {
			rt_thread_delay(100);
		  	gprs_at_cmd(g_dev_m26,at_csq);
		  }

		break;
			case M26_STATE_DATA_READ:
		//for (int m=0;m<data_size;m++)
		//
		//{
		//	rt_kprintf("%c",((rt_uint8_t *)last_data_ptr)[m]);
		//}
		handle_m26_server_in(last_data_ptr,data_size);
		break;
			case M26_STATE_DATA_PRE_WRITE:
		if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
			g_m26_state = M26_STATE_DATA_WRITE;
			/*if (((rt_uint8_t *)send_data_ptr_m26)[0] != 0xad ||
				((rt_uint8_t *)send_data_ptr_m26)[1] != 0xac) {
				rt_kprintf("\r\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
				for (int n=0; n<send_size_m26; n++)
					rt_kprintf("%02x ", ((rt_uint8_t *)send_data_ptr_m26)[n]);
				rt_kprintf("\r\n");
				for (int n=0; n<send_size_m26; n++)
					rt_kprintf("%02x ", test_buf[n]);
				rt_kprintf("\r\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
				}*/
			rt_device_write(g_pcie[g_index]->dev, 0, test_buf, send_size_m26);	
		}
		else if(have_str(last_data_ptr, STR_ERROR))
		{
			g_m26_state = M26_STATE_CHECK_QISTAT;
			gprs_at_cmd(g_dev_m26,qistat);
		}
		if (have_str(last_data_ptr, STR_QIRDI)||have_str(last_data_ptr, STR_QIURC))
			g_data_in_m26 = RT_TRUE;
		
		break;
			case M26_STATE_DATA_WRITE:
		if (have_str(last_data_ptr, STR_SEND_OK)) {
			g_m26_state = M26_STATE_DATA_ACK;
			gprs_at_cmd(g_dev_m26,qisack);
			//g_m26_state = M26_STATE_DATA_PROCESSING;
			//gprs_at_cmd(g_dev_m26,at_csq);
		} else {
			if (!have_str(last_data_ptr, STR_QIRDI) && 
					!have_str(last_data_ptr, STR_QIURC)) {
				g_m26_state = M26_STATE_CHECK_QISTAT;
				gprs_at_cmd(g_dev_m26,qistat);
			}
		}
		if (have_str(last_data_ptr, STR_QIRDI)||have_str(last_data_ptr, STR_QIURC))
			g_data_in_m26 = RT_TRUE;
		break;
			case M26_STATE_DATA_ACK:
		g_m26_state = M26_STATE_DATA_PROCESSING;		
		if (have_str(last_data_ptr, STR_QISACK) ||have_str(last_data_ptr, STR_QIRDI)) {
			gprs_at_cmd(g_dev_m26,at_csq);						
			if (have_str(last_data_ptr, STR_QIRDI)) 
				g_data_in_m26 = RT_TRUE;
		}
		else{
			g_m26_state = M26_STATE_CHECK_QISTAT;
			gprs_at_cmd(g_dev_m26,qistat);
		}				
		rt_time_t cur_time = time(RT_NULL);
		rt_kprintf("send server ok %s\r\n",ctime(&cur_time));
		//rt_event_send(&(g_pcie[g_index]->event), M26_EVENT_0);
		break;		
		}
	}
}
