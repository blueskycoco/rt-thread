#include <stdio.h>
#include <unistd.h> 
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <ctype.h>
#include "led.h"
#include <string.h>
#include "bc26.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"

#define BC26_EVENT_0 					(1<<0)
#define BC26_STATE_INIT					0
#define BC26_STATE_CHECK_CPIN			1
#define BC26_STATE_SET_QINDI			2
#define BC26_STATE_SET_QIMUX			3
#define BC26_STATE_CHECK_CGREG			4
#define BC26_STATE_CHECK_QISTAT			5
#define BC26_STATE_SET_QICSGP			6
#define BC26_STATE_SET_QIREGAPP			7
#define BC26_STATE_SET_QISRVC			8
#define BC26_STATE_SET_QIACT			9
#define BC26_STATE_CHECK_CIMI			10
#define BC26_STATE_SET_QIFGCNT			11
#define BC26_STATE_SET_QIOPEN 			12
#define BC26_STATE_SET_QIDEACT  		13
#define BC26_STATE_DATA_PROCESSING 		14
#define BC26_STATE_DATA_READ			15
#define BC26_STATE_SET_QICLOSE			16
#define BC26_STATE_DATA_WRITE			17
#define BC26_STATE_DATA_ACK				18
#define BC26_STATE_DATA_PRE_WRITE		19
#define BC26_STATE_ATE0 				21
#define BC26_STATE_CSQ					22
#define BC26_STATE_LAC					23
#define BC26_STATE_ICCID				24
#define BC26_STATE_IMEI					25
#define BC26_STATE_LACR					26
#define BC26_STATE_CSCON				27
#define BC26_STATE_CFG_FTP				28
#define BC26_STATE_OPEN_FTP				29
#define BC26_STATE_GET_FILE				30
#define BC26_STATE_READ_FILE			31
#define BC26_STATE_LOGOUT_FTP			32
#define BC26_STATE_SET_LOCATION			33
#define BC26_STATE_V					34
#define BC26_STATE_CLEAN_RAM			35
#define BC26_STATE_CLOSE_FILE			36
#define BC26_STATE_CGATT				37
#define BC26_CREATE_SOCKET				38
#define BC26_NSOCR						39
#define BC26_QENG						40
#define BC26_CPSMS						41
#define BC26_STATE_BAND					42
#define BC26_STATE_QENG					43
#define BC26_CFUN_0						44
#define BC26_CFUN_1						45
#define BC26_QREG_ASK					46
#define BC26_QREG_SET					47
#define STR_NSOCR						"+NSOCR"
#define STR_CSCON						"+CSCON:0,1"
#define STR_GSN							"+CGSN:"
#define STR_CGATT_OK					"+CGATT:1"
#define STR_CFUN						"+CFUN:"
#define STR_RDY							"Neul"
#define STR_CPIN						"+CPIN:"
#define STR_CSQ							"+CSQ:"
#define STR_CREG						"+CREG:"
#define STR_CPIN_READY					"+CPIN: READY"
#define STR_CGREG						"+CEREG: 0,"
#define STR_CGREG_READY					"+CEREG: 0,1"
#define STR_CGREG_READY1				"+CEREG: 0,5"
#define STR_STAT_INIT					"IP INITIAL"
#define STR_STAT_IND					"IP IND"
#define STR_STAT_CLOSE					"IP CLOSE"
#define STR_STAT_STATUS					"IP STATUS"
#define STR_STAT_DEACT					"PDP DEACT"
#define STR_QIMUX_0						"+QIMUX: 0"
#define STR_OK							"OK"
#define STR_QIRD						"+QIRD:"
#define STR_QIURC						"+QIURC:"
#define STR_TCP							"TCP,"
#define STR_CLOSED						"+NSOCLI"
#define STR_BEGIN_WRITE					">"
#define STR_ERROR						"ERROR"
#define STR_4600						"4600"
#define STR_46000						"46000"
#define STR_46001						"46001"
#define STR_46002						"46002"
#define STR_46003						"46003"
#define STR_46004						"46004"
#define STR_46006						"46006"
#define STR_STAT_DEACT_OK 				"DEACT OK"
#define STR_CONNECT_OK					"+QIOPEN: 0,0"
#define STR_CLOSE_OK					"CLOSE OK"
#define STR_SEND_OK						"SEND OK"
#define STR_QIRDI						"+NSONMI:1" 
#define STR_QISACK						"+QISACK"
#define STR_SOCKET_BUSSY				"+QIOPEN: 0,566"
#define STR_CONNECT_FAIL				"CONNECT FAIL"
#define STR_CONNECT_OK_EC20				"+QIOPEN: 0,0"
#define STR_CONNECT_FAIL_EC20			"+QIOPEN: 0,"
#define STR_QFTPCFG						"+QFTPCFG:0"
#define STR_FTP_FILE_SIZE				"+QFTPSIZE:"
#define DEFAULT_SERVER					"101.132.177.116"
#define DEFAULT_PORT					"2011"
#define STR_QCCID						"+NCCID:"
#define STR_QSOC						"+QSOC=0"
#define STR_PDP_DEACT					"+PDP DEACT"
#define STR_QREG_2						"+QREGSWT:2"
#define STR_QREG_1						"+QREGSWT:1"
#define STR_QREG_0						"+QREGSWT:0"

#define c_socket						"AT+QSOC=1,1,1\r\n"
#define cscon							"AT+CSCON?\r\n"
#define qistat 							"AT+QISTAT\r\n"
#define qiclose 						"AT+NSOCL=1\r\n"
#define qilocip 						"AT+QILOCIP\r\n"
#define qilport 						"AT+QILPORT?\r\n"
#define e0 								"ATE0\r\n"
#define cgreg 							"AT+CEREG?\r\n"
#define cregs 							"AT+CREG=2\r\n"
#define cregr 							"AT+CREG?\r\n"
#define at_csq 							"AT+CSQ\r\n"
#define at_qccid 						"AT+NCCID\r\n"
#define gsn 							"AT+CGSN=1\r\n"
#define cpsms							"AT+CPSMS=0\r\n"
#define qeng							"AT+QENG=0\r\n"
#define qicfg							"AT+NSOCR=\"dataformat\",1,1\r\n"
#define nsocr							"AT+NSOCR=STREAM,6,56000,1\r\n"
#define cimi 							"AT+CIMI\r\n"
#define cpin 							"AT+CPIN?\r\n"
#define qifgcnt 						"AT+QIFGCNT=0\r\n"
#define qisrvc 							"AT+QISRVC=1\r\n"
#define qimux 							"AT+QIMUX=0\r\n"
#define ask_qimux 						"AT+QIMUX?\r\n"
#define qideact 						"AT+QIDEACT\r\n"
#define ati								"ATI\r\n"
#define qindi 							"AT+QINDI=1\r\n"
//#define at_band							"AT+NBAND=5,8,3,28,20,1\r\n"
#define at_band							"AT+NBAND?\r\n"
#define at_qeng 						"AT+QENG=0\r\n"
//#define qird 							"AT+QIRD=0,512\r\n"
#define qisack 							"AT+QISACK\r\n"
#define qiat 							"AT\r\n"
#define cgatt							"AT+CGATT?\r\n"
#define qiftp_set_path					"AT+QFTPPATH=\"/\"\r\n"
#define qiftp_file_size					"AT+QFTPSIZE=\"stm32_0.bin\"\r\n"
#define qiftp_clean_ram 				"AT+QFDEL=\"RAM:*\"\r\n"
#define qiregapp 						"AT+QIREGAPP\r\n"
#define qiact 							"AT+QIACT\r\n"
#define cfun0							"AT+CFUN=0\r\n"
#define cfun1							"AT+CFUN=1\r\n"
#define qregswt_ask						"AT+QREGSWT?\r\n"
#define qregswt_set						"AT+QREGSWT=2\r\n"
uint8_t 	  qicsgp_bc26[32]			= {0};
uint8_t 	  qiopen_bc26[64]			= {0};
uint8_t 	  qisend_bc26[32] 			= {0};
//uint8_t 	  qiftp_set_resuming[32] 	= {0};
uint8_t 	  qiftp_bc26_ram[32]		= {0};
uint8_t		  qiftp_get_bc26[32]		= {0};
uint8_t 	  qird[32] = {0};
rt_bool_t 	  g_data_in_bc26 			= RT_FALSE;
extern int 	  g_index;
//rt_uint32_t ftp_ofs 					= 0;
rt_device_t   g_dev_bc26;
uint8_t 	  g_bc26_state 				= BC26_STATE_INIT;
uint32_t 	  server_len_bc26 			= 0;
uint8_t 	  *server_buf_bc26 			= RT_NULL;
void 		  *send_data_ptr_bc26 		= RT_NULL;
rt_size_t 	  send_size_bc26;
rt_uint8_t 	  bc26_cnt 					= 0;
extern rt_uint8_t 	g_net_state;
extern rt_uint32_t 	g_server_addr;
extern rt_uint32_t 	g_server_addr_bak;
extern rt_uint16_t 	g_server_port;
extern rt_uint16_t 	g_server_port_bak;
extern rt_uint8_t 	g_ip_index;
extern rt_uint8_t 	ftp_cfg_step;
extern rt_uint8_t 	ftp_addr[32];
extern rt_uint8_t 	ftp_user[32];
extern rt_uint8_t 	ftp_passwd[32];
extern rt_uint8_t 	g_heart_cnt;
extern rt_uint8_t 	entering_ftp_mode;
extern rt_uint8_t 	*tmp_stm32_bin;
extern rt_size_t	tmp_stm32_len;
extern int 			stm32_fd;
extern int 			stm32_len;
extern int 			cur_stm32_len;
extern int 			down_fd;
uint8_t 	  		qiftp_bc26[64];
extern uint8_t		qiftp_read_file[32];//		"AT+QFREAD=\"RAM:stm32.bin\",0\r\n"
extern uint8_t		qiftp_close_file[32];
extern rt_uint8_t 	ftp_rty;
extern rt_uint16_t 	g_app_v;
extern rt_mp_t 		server_mp;
extern rt_uint8_t 	g_module_type;
rt_uint8_t 			entering_ftp_mode_bc26=0;
extern rt_uint8_t 	update_flag;
extern struct rt_event 	g_info_event;
void toHex(uint8_t *input, uint32_t len, uint8_t *output)
{
	int ix=0,iy=0;
	for (ix=0; ix<len; ix++) {
		output[iy] = (input[ix]&0xf0) >>4;
		if(output[iy]>=10 && output[iy]<=15)
		{
			output[iy] = output[iy] + 'a' - 10;
		}
		else
		{
			output[iy] = output[iy] + '0';
		}
		iy++;
		output[iy] = input[ix]&0x0f;
		if(output[iy] >= 10 && output[iy] <= 15)
		{
			output[iy] = output[iy] + 'a' - 10;
		}
		else
		{
			output[iy] = output[iy] + '0';
		}
		iy++;
	}
	//rt_kprintf("\r\nHexString:\r\n");
	//for (ix=0; ix<len*2; ix++)
	//	rt_kprintf("%x",output[ix]);
	//rt_kprintf("\r\n");
}
void fromHex(uint8_t *input, uint32_t len, uint8_t *output)
{
	int ix=0,iy=0;
	//rt_kprintf("\r\nHex:\r\n");
	for (ix=0; ix<len; ix++) {
		if (input[ix] >= '0' && input[ix] <= '9') {
			output[iy] = (input[ix] - '0' )<< 4;
		}else {
			output[iy] = (input[ix] - 'A' +10 )<< 4;
		}
		ix++;
		if (input[ix] >= '0' && input[ix] <= '9') {
			output[iy] |= (input[ix] - '0' );
		}else {
			output[iy] |= (input[ix] - 'A' +10);
		}
		//rt_kprintf("%x", output[iy]);
		//rt_kprintf("\r\n");
		iy++;
	}
}
void handle_bc26_server_in(const void *last_data_ptr,rt_size_t len)
{
	static rt_bool_t flag = RT_FALSE;	
	int i;
	int ofs;
	rt_kprintf("server in 1\r\n");
	if (/*have_str(last_data_ptr, STR_OK) &&*/ have_str(last_data_ptr, "ADAC")) {
		uint8_t *pos = (uint8_t *)strstr(last_data_ptr, "ADAC");
		uint32_t len_ofs = 0;
		rt_kprintf("server in 2\r\n");
		while (pos[len_ofs] != ',' && len_ofs < len)
			len_ofs++;
		rt_uint8_t *server_buf_bc26_1 = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
		fromHex(pos, len_ofs,server_buf_bc26_1);
		rt_data_queue_push(&g_data_queue[3], server_buf_bc26_1, len_ofs/2, RT_WAITING_FOREVER);
		g_bc26_state = BC26_STATE_DATA_PROCESSING;
		g_data_in_bc26 = 0;
		gprs_at_cmd(g_dev_bc26,at_csq);
	}
	#if 0
	if ((ofs = match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRD,rt_strlen(STR_QIRD)))!=-1)
	{	
			uint8_t *pos = (uint8_t *)last_data_ptr;
		//	for(i=0;i<256;i++)
	//			rt_kprintf("%02x",pos[i]);
			rt_kprintf("ofs is %d\r\n",ofs);
			//if (pos != RT_NULL) {
				int i = 7+ofs;
				//rt_kprintf("\r\n<>%x%x%x%x%x%x%x%x%x%x%x%x<>\r\n",
				//	pos[ofs],pos[ofs+1],pos[ofs+2],pos[ofs+3],pos[ofs+4],pos[ofs+5],pos[ofs+6],pos[ofs+7],
				//	pos[ofs+8],pos[ofs+9],pos[ofs+10],pos[ofs+11]);
				while (pos[i] != '\r' && pos[i+1] != '\n' && i<len)
				{
					server_len_bc26 = server_len_bc26*10 + pos[i] - '0';
					i++;
				}
				//server_len_ec20 = get_len(pos+i,len-i);
				server_len_bc26 *=2;
				rt_kprintf("server len %d\r\n", server_len_bc26);
				server_buf_bc26 = (uint8_t *)rt_malloc(server_len_bc26* sizeof(uint8_t));
				if (server_buf_bc26 == RT_NULL)
					rt_kprintf("malloc buf 26 failed\r\n");
				rt_memset(server_buf_bc26,0,server_len_bc26);
				//server_len_ec20 = 0;
				i+=2;
				rt_kprintf("i %d\r\n",i);
				if (i+server_len_bc26 < len)
					rt_memcpy(server_buf_bc26,pos+i,server_len_bc26);
				else
				{
					server_len_bc26 = i+server_len_bc26-len;
					rt_memcpy(server_buf_bc26,pos+i,server_len_bc26);
				}
				/*while(i<len && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
				{
	
					server_buf_ec20[server_len_ec20++] = pos[i++];
					rt_kprintf("<%02x>\r\n", server_buf_ec20[server_len_ec20-1]);
				}*/
				if (match_bin(pos, len, "OK",rt_strlen("OK"))!=-1)
				{
					rt_uint8_t *server_buf_bc26_1 = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
					fromHex(server_buf_bc26, server_len_bc26,server_buf_bc26_1);
					rt_free(server_buf_bc26);
					rt_data_queue_push(&g_data_queue[3], server_buf_bc26_1, server_len_bc26/2, RT_WAITING_FOREVER);
					if (server_len_bc26 == 1500) {
						g_bc26_state = BC26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_bc26,qistat);
					} else {
						g_bc26_state = BC26_STATE_DATA_PROCESSING;
						gprs_at_cmd(g_dev_bc26,at_csq);
					}
					if (match_bin((rt_uint8_t *)last_data_ptr, len,STR_QIRDI,rt_strlen(STR_QIRDI))==-1)
						g_data_in_bc26 = RT_FALSE;
	
					flag = RT_FALSE;
				}
				else
					flag = RT_TRUE;
	}
	else if (flag){	
		int i=0;
		uint8_t *pos = (uint8_t *)last_data_ptr;
		while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
		{
			server_buf_bc26[server_len_bc26++] = pos[i++];
		}

		if (strstr(pos, "OK")!=RT_NULL)
		{
			
			rt_uint8_t *server_buf_bc26_1 = rt_mp_alloc(server_mp, RT_WAITING_FOREVER);
			fromHex(server_buf_bc26, server_len_bc26,server_buf_bc26_1);
			rt_free(server_buf_bc26);
			rt_data_queue_push(&g_data_queue[3], server_buf_bc26_1, server_len_bc26/2, RT_WAITING_FOREVER);
			if (server_len_bc26 == 1500) {
				g_bc26_state = BC26_STATE_CHECK_QISTAT;
				gprs_at_cmd(g_dev_bc26,qistat);
			} else {
				g_bc26_state = BC26_STATE_DATA_PROCESSING;
				gprs_at_cmd(g_dev_bc26,at_csq);		
			}
			if (!have_str(last_data_ptr, STR_QIRDI))
				g_data_in_bc26 = RT_FALSE;
			//rt_mutex_release(&(g_pcie[g_index]->lock));
			flag = RT_FALSE;
		}
		
	}else {
		
	}
	#endif
}

void bc26_start(int index)
{
	rt_uint32_t power_rcc,pwr_key_rcc;
	rt_uint16_t power_pin,pwr_key_pin;
	GPIO_TypeDef* GPIO_power,*GPIO_pwr;
	GPIO_InitTypeDef GPIO_InitStructure;
	g_index = index;
	g_dev_bc26 = g_pcie[index]->dev;
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
	g_bc26_state = BC26_STATE_INIT;
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
	rt_kprintf("bc26 power on done\r\n");
	
	strcpy(ftp_addr,"u.110LW.com");
	//strcpy(ftp_addr,"47.93.48.167");
	strcpy(ftp_user,"minfei");
	strcpy(ftp_passwd,"minfei123");
}

void bc26_proc(void *last_data_ptr, rt_size_t data_size)
{
	int i=0;
	rt_uint8_t *tmp = (rt_uint8_t *)last_data_ptr;
	//if (!have_str(last_data_ptr,STR_CSQ)&&(data_size>6)) {
		rt_kprintf("\r\n<== (BC28 %d %d)\r\n",g_bc26_state, data_size);
		for (i=0; i<data_size; i++)
			if (isascii(tmp[i]) && (g_bc26_state != BC26_STATE_READ_FILE))
				rt_kprintf("%c", tmp[i]);
			else
				//rt_kprintf("%02x", tmp[i]);
				break;
		if (have_str(last_data_ptr, STR_QIRDI)) {
			rt_time_t cur_time = time(RT_NULL);
			rt_kprintf("get server message %s\r\n",ctime(&cur_time));
		
		}
	//}
	if (data_size >= 2) {
		if (have_str(last_data_ptr,STR_RDY))
		{
			rt_kprintf("got bc28 rdy\r\n");
			g_bc26_state = BC26_STATE_INIT;
		}
		if (have_str(last_data_ptr,STR_CLOSED)||have_str(last_data_ptr,STR_PDP_DEACT))
		{
			//if (have_str(last_data_ptr,STR_PDP_DEACT)) {
				rt_kprintf("MODULE lost\r\n");
				g_heart_cnt=0;
				g_net_state = NET_STATE_UNKNOWN;
				pcie_switch(g_module_type);
				return;
			//} else {				
			//	g_bc26_state = BC26_CREATE_SOCKET;
			//	gprs_at_cmd(g_dev_bc26,nsocr);
			//	return;
			//}
		}
		switch (g_bc26_state) {
			case BC26_STATE_INIT:
				//if (have_str(last_data_ptr,STR_RDY)) {
				g_bc26_state = BC26_STATE_ATE0;
				gprs_at_cmd(g_dev_bc26,e0);	
				//}
				break;
			case BC26_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					g_bc26_state = BC26_STATE_V;
					gprs_at_cmd(g_dev_bc26,ati);
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_bc26,e0);
				}
				break;
			case BC26_STATE_BAND:
				if (have_str(last_data_ptr,STR_OK)) {
					g_pcie[g_index]->cpin_cnt=0;
					g_bc26_state = BC26_STATE_CGATT;
					gprs_at_cmd(g_dev_bc26,cgatt);
				}
				break;
			case BC26_STATE_V:
				if (have_str(last_data_ptr, STR_OK)) {
					g_bc26_state = BC26_QREG_ASK;
					gprs_at_cmd(g_dev_bc26,qregswt_ask);
				}
				break;
			case BC26_QREG_ASK:
				if (have_str(last_data_ptr, STR_QREG_2)) {
					g_bc26_state = BC26_STATE_BAND;
					gprs_at_cmd(g_dev_bc26,at_band);
				} else if (have_str(last_data_ptr, STR_QREG_1) ||
						have_str(last_data_ptr, STR_QREG_0)) {
					g_bc26_state = BC26_QREG_SET;
					gprs_at_cmd(g_dev_bc26,qregswt_set);
				}
			case BC26_QREG_SET:
				if (have_str(last_data_ptr, STR_OK)) {
					g_heart_cnt=0;
					g_net_state = NET_STATE_UNKNOWN;
					pcie_switch(g_module_type);
					rt_kprintf("set qregswt to 2 ok\r\n");
				}
			case BC26_CFUN_0:
				if (have_str(last_data_ptr, STR_OK)) {
					g_bc26_state = BC26_STATE_BAND;
					gprs_at_cmd(g_dev_bc26,at_band);
				}
				break;
			case BC26_CFUN_1:
				if (have_str(last_data_ptr, STR_OK)) {
					g_pcie[g_index]->cpin_cnt=0;
					g_bc26_state = BC26_STATE_CGATT;
					gprs_at_cmd(g_dev_bc26,cgatt);
				}
				break;
			case BC26_STATE_QENG:
				if (have_str(last_data_ptr,STR_OK)) {
					g_bc26_state = BC26_STATE_BAND;
					gprs_at_cmd(g_dev_bc26,at_qeng);
				}
				break;
			case BC26_STATE_CGATT:
				if (have_str(last_data_ptr, STR_CGATT_OK)) {
					g_bc26_state = BC26_STATE_CSQ;
					gprs_at_cmd(g_dev_bc26,at_csq);
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					//gprs_at_cmd(g_dev_bc26,at_csq);
					gprs_at_cmd(g_dev_bc26,cgatt);
				}
				break;	
			case BC26_STATE_CSQ:
				if (have_str(last_data_ptr,STR_CSQ)) {
					if (tmp[9] == 0x2c) {
						if (tmp[7] == ':')
							g_pcie[g_index]->csq = tmp[8]-0x30;
						else
							g_pcie[g_index]->csq = (tmp[7]-0x30)*10 + tmp[8]-0x30;
					}
					else if (tmp[8] == 0x2c)
						g_pcie[g_index]->csq = tmp[7]-0x30;
					rt_kprintf("csq is %x %x %x %d\r\n",tmp[7],tmp[8],tmp[9],g_pcie[g_index]->csq);
					show_signal(g_pcie[g_index]->csq);
					//g_bc26_state = BC26_STATE_LAC;
					//gprs_at_cmd(g_dev_bc26,cregs);
					//gprs_at_cmd(g_dev_bc26,qeng);
					//g_bc26_state = BC26_QENG;
					gprs_at_cmd(g_dev_bc26,cpsms);
					g_bc26_state = BC26_CPSMS;
				} 
				break;
			case BC26_QENG:				
				gprs_at_cmd(g_dev_bc26,cpsms);
				g_bc26_state = BC26_CPSMS;
				break;
			case BC26_CPSMS:
				gprs_at_cmd(g_dev_bc26,cscon);
				g_bc26_state = BC26_STATE_CSCON;
				break;
			case BC26_STATE_CSCON:
				if (have_str(last_data_ptr, STR_CSCON)) {
					//g_bc26_state = BC26_STATE_LAC;
					//gprs_at_cmd(g_dev_bc26,cregs);
					g_bc26_state = BC26_STATE_ICCID;
						gprs_at_cmd(g_dev_bc26,at_qccid);
					} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_bc26,cscon);
					}
					break;
			case BC26_STATE_LAC:
				if (have_str(last_data_ptr,STR_OK)) {
					g_bc26_state = BC26_STATE_LACR;
					gprs_at_cmd(g_dev_bc26,cregr);
				} 
				break;
			case BC26_STATE_LACR:
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
						g_bc26_state = BC26_STATE_ICCID;
						gprs_at_cmd(g_dev_bc26,at_qccid);
				} 
				break;
			case BC26_STATE_ICCID:
				if (have_str(last_data_ptr, STR_QCCID)) {
					i=2;
					while(/*tmp[i]!='\r'*/i<22)
					{
						if (tmp[i+7]>='0' && tmp[i+7]<='9')
							g_pcie[g_index]->qccid[i/2-1] = (tmp[i+7]-0x30)*16;
						else if (tmp[i+7]>='a' && tmp[i+7]<='f')
							g_pcie[g_index]->qccid[i/2-1] = (tmp[i+7]-'a'+10)*16;
						else if (tmp[i+7]>='A' && tmp[i+7]<='F')
							g_pcie[g_index]->qccid[i/2-1] = (tmp[i+7]-'A'+10)*16;

						if (tmp[i+8]>='0' && tmp[i+8]<='9')
							g_pcie[g_index]->qccid[i/2-1] += (tmp[i+8]-0x30);
						else if (tmp[i+8]>='a' && tmp[i+8]<='f')
							g_pcie[g_index]->qccid[i/2-1] += (tmp[i+8]-'a'+10);
						else if (tmp[i+8]>='A' && tmp[i+8]<='F')
							g_pcie[g_index]->qccid[i/2-1] += (tmp[i+8]-'A'+10);
						rt_kprintf("qccid[%d] = %02X\r\n",i/2-1,g_pcie[g_index]->qccid[i/2-1]);
						i+=2;						
					}					
					g_bc26_state = BC26_STATE_IMEI;
					gprs_at_cmd(g_dev_bc26,gsn);
					}
					break;
			case BC26_STATE_IMEI:

				if (have_str(last_data_ptr, STR_GSN)) {
				g_pcie[g_index]->imei[0]=0x0;
				i=2;//866159032379171
				while(tmp[i+6]!='\r' && i<17)
				{
					if (tmp[i+6]>='0' && tmp[i+6]<='9')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i+6]-0x30);
					else if (tmp[i]>='a' && tmp[i+6]<='f')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i+6]-'a'+10);
					else if (tmp[i]>='A' && tmp[i+6]<='F')
						g_pcie[g_index]->imei[i/2-1] += (tmp[i+6]-'A'+10);
					rt_kprintf("imei[%d] = %02X\r\n",i/2-1,g_pcie[g_index]->imei[i/2-1]);
					//i+=2;
					if (tmp[i+7]>='0' && tmp[i+7]<='9')
						g_pcie[g_index]->imei[i/2] = (tmp[i+7]-0x30)*16;
					else if (tmp[i+1]>='a' && tmp[i+7]<='f')
						g_pcie[g_index]->imei[i/2] = (tmp[i+7]-'a'+10)*16;
					else if (tmp[i+1]>='A' && tmp[i+7]<='F')
						g_pcie[g_index]->imei[i/2] = (tmp[i+7]-'A'+10)*16;
				
					i+=2;						
				}					
					g_bc26_state = BC26_CREATE_SOCKET;
					gprs_at_cmd(g_dev_bc26,nsocr);
					}
					break;
			//case BC26_NSOCR:
			//	if (have_str(last_data_ptr, STR_QSOC)) {						
			//		g_bc26_state = BC26_CREATE_SOCKET;
			//		gprs_at_cmd(g_dev_bc26,qicfg);
			//	}
			//	break;
			case BC26_STATE_SET_QICLOSE:
				if (have_str(last_data_ptr, STR_OK)) {
					g_net_state = NET_STATE_UNKNOWN;
					g_bc26_state = BC26_CREATE_SOCKET;
					gprs_at_cmd(g_dev_bc26,nsocr);
				}
				break;
			case BC26_CREATE_SOCKET:
				if (have_str(last_data_ptr, STR_OK)) {					
					g_bc26_state = BC26_STATE_SET_QIOPEN;
					if (!entering_ftp_mode) {
						rt_memset(qiopen_bc26, 0, 64);					
						rt_sprintf(qiopen_bc26, "AT+NSOCO=1,%d.%d.%d.%d,%d\r\n",
								mp.socketAddress[g_ip_index].IP[0],mp.socketAddress[g_ip_index].IP[1],
								mp.socketAddress[g_ip_index].IP[2],mp.socketAddress[g_ip_index].IP[3],
								mp.socketAddress[g_ip_index].port);
						rt_kprintf("open ip index %d\r\n",g_ip_index);
						gprs_at_cmd(g_dev_bc26,qiopen_bc26);
					} else {
						gprs_at_cmd(g_dev_bc26,"AT+NSOCO=1,106.14.177.87,1706\r\n");
						update_flag=1;
					}
				}
				break;
			case BC26_STATE_SET_QIOPEN:
				if (have_str(last_data_ptr, STR_OK)) {
					g_bc26_state = BC26_STATE_DATA_PROCESSING;
					/*send data here */
				//	rt_kprintf("connect to server ok\r\n");
				//	rt_event_send(&(g_pcie[g_index]->event), BC26_EVENT_0);
				//	gprs_at_cmd(g_dev_bc26,at_csq);
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
					rt_event_send(&(g_pcie[g_index]->event), BC26_EVENT_0);
					gprs_at_cmd(g_dev_bc26,at_csq);
					rt_hw_led_on(NET_LED);
					//entering_ftp_mode=1;
				} else if (have_str(last_data_ptr, STR_SOCKET_BUSSY)){
					g_bc26_state = BC26_CREATE_SOCKET;			
					
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
					gprs_at_cmd(g_dev_bc26,at_csq);		
				}
					#if 0
				else {
					if (!have_str(last_data_ptr, STR_OK)) {
						rt_thread_delay(100*3);
						g_bc26_state = BC26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_bc26,qistat);
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
				#endif
				break;
			case BC26_STATE_DATA_PROCESSING:
				if (have_str(last_data_ptr, STR_QIRDI)) {
					/*server data in */					
					//rt_mutex_take(&(g_pcie[g_index]->lock), RT_WAITING_FOREVER);
					int len=0;
					char *len_ptr = strstr(last_data_ptr, "1,");
					while(len_ptr[len] != '\r' && len_ptr[len] != '\n')
						len++;
					g_bc26_state = BC26_STATE_DATA_READ;
					//if (strstr(len_ptr, "535")) 
					//	strcpy(qird,"AT+NSORF=10\r\n");
					//else						
					//rtk_printf("len ptr len %d\r\n", strlen(len_ptr));
					//rt_sprintf(qird,"AT+NSORF=%s", len_ptr);
					rt_kprintf("len %d\r\n", len);
					memset(qird,0,32);
					strcpy(qird,"AT+NSORF=");
					memcpy(qird+strlen("AT+NSORF="),len_ptr, len+2);
					//strcat(qird,"\r\n");
					gprs_at_cmd(g_dev_bc26,qird);
					server_len_bc26 = 0;
					g_data_in_bc26 = RT_TRUE;
				//} else if (have_str(last_data_ptr, STR_OK)){
				} else if (have_str(last_data_ptr,STR_CSQ)) {
					if (tmp[9] == 0x2c) {
						if (tmp[7] == ':')
							g_pcie[g_index]->csq = tmp[8]-0x30;
						else
							g_pcie[g_index]->csq = (tmp[7]-0x30)*10 + tmp[8]-0x30;
					}
					else if (tmp[8] == 0x2c)
						g_pcie[g_index]->csq = tmp[7]-0x30;
					rt_kprintf("csq is %x %x %x %d\r\n",tmp[7],tmp[8],tmp[9],g_pcie[g_index]->csq);
					show_signal(g_pcie[g_index]->csq);

				
					if (g_server_addr != g_server_addr_bak || g_server_port != g_server_port_bak) {
						g_ip_index=0;
						g_bc26_state = BC26_STATE_SET_QICLOSE;
						gprs_at_cmd(g_dev_bc26,qiclose);
						break;
					}
//			if (g_heart_cnt > 5 || entering_ftp_mode) {
							//if (g_heart_cnt > 5) {
							//	g_bc26_state = BC26_STATE_SET_QICLOSE;
							//	gprs_at_cmd(g_dev_bc26,qiclose);
							//}/ else {
								if (entering_ftp_mode_bc26 && entering_ftp_mode) {
									rt_kprintf("goto update\r\n");
									entering_ftp_mode_bc26=0;
									g_heart_cnt=0;
									g_net_state = NET_STATE_UNKNOWN;
									pcie_switch(g_module_type);
									break;
								}
								if (g_heart_cnt > 5) {
									rt_kprintf("hehehe 1\r\n");
									g_heart_cnt=0;
									g_net_state = NET_STATE_UNKNOWN;
									pcie_switch(g_module_type);
									break;
								}
					//}
	//			break;
					//

					/*check have data to send */
					if (rt_data_queue_peak(&g_data_queue[2],(const void **)&send_data_ptr_bc26,&send_size_bc26) == RT_EOK)
					{	
						rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_bc26, &send_size_bc26, RT_WAITING_FOREVER);
						rt_kprintf("should send data %d\r\n", send_size_bc26);
						rt_uint8_t *hex_string = (rt_uint8_t *)rt_malloc((send_size_bc26*2+1)*sizeof(rt_uint8_t));
						rt_memset(hex_string,0,send_size_bc26*2+1);
						toHex(send_data_ptr_bc26, send_size_bc26, hex_string);
						rt_sprintf(qisend_bc26, "AT+NSOSD=1,%d,", send_size_bc26);
						gprs_at_cmd(g_dev_bc26,qisend_bc26);
						gprs_at_cmd(g_dev_bc26,hex_string);
						gprs_at_cmd(g_dev_bc26,"\r\n");
						rt_free(hex_string);
						if (send_data_ptr_bc26) {
							if (send_size_bc26 <= 64) {
								rt_mp_free(send_data_ptr_bc26);
							} else {
								rt_free(send_data_ptr_bc26);
							}
							send_data_ptr_bc26 = RT_NULL;
						}
						g_bc26_state = BC26_STATE_DATA_WRITE;
					} else {
						if (!g_data_in_bc26) {
						rt_thread_delay(100);
						gprs_at_cmd(g_dev_bc26,at_csq);
						}
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
							gprs_at_cmd(g_dev_bc26,at_csq);
					} 
					/*else {
					g_bc26_state = BC26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_bc26,qistat);
					rt_hw_led_off(NET_LED);
					}*/

				break;
			case BC26_STATE_DATA_READ:
				//for (int m=0;m<data_size;m++)
				//
				//{
				//	rt_kprintf("%c",((rt_uint8_t *)last_data_ptr)[m]);
				//}
				handle_bc26_server_in(last_data_ptr,data_size);
				break;
			case BC26_STATE_DATA_WRITE:
				if (have_str(last_data_ptr, STR_OK)) {
					//g_bc26_state = BC26_STATE_DATA_ACK;
					//gprs_at_cmd(g_dev_bc26,(qisack);
					g_bc26_state = BC26_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_bc26,at_csq);
					rt_time_t cur_time = time(RT_NULL);
					rt_kprintf("send server ok %s\r\n",ctime(&cur_time));
					if (!entering_ftp_mode)
						rt_event_send(&(g_pcie[g_index]->event), BC26_EVENT_0);
				} else {
					/*if (!have_str(last_data_ptr, STR_QIRDI) && 
							!have_str(last_data_ptr, STR_QIURC)) {
						g_bc26_state = BC26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_bc26,qistat);
					}*/
				}
				if (have_str(last_data_ptr, STR_QIRDI) || g_data_in_bc26)
				{
						g_bc26_state = BC26_STATE_DATA_READ;
						gprs_at_cmd(g_dev_bc26,qird);
						server_len_bc26 = 0;
				}
				break;
			case BC26_STATE_DATA_ACK:
				if (have_str(last_data_ptr, STR_QISACK)) {
					g_bc26_state = BC26_STATE_DATA_PROCESSING;
					if (g_data_in_bc26 || have_str(last_data_ptr, STR_QIRDI)) {
						g_bc26_state = BC26_STATE_DATA_READ;
						gprs_at_cmd(g_dev_bc26,qird);
						server_len_bc26 = 0;
					} else {
						gprs_at_cmd(g_dev_bc26,at_csq);
					}
				} else if (have_str(last_data_ptr, STR_QIRDI))
					g_data_in_bc26 = RT_TRUE;
				else{
					g_bc26_state = BC26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_bc26,qistat);
				}
				break;		
		}
	}
}
