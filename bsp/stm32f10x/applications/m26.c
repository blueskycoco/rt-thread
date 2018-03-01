#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
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
#define STR_QIRDI					"+QIRDI:"
#define STR_QISACK					"+QISACK"
#define STR_SOCKET_BUSSY			"SOCKET BUSY"
#define STR_CONNECT_FAIL			"CONNECT FAIL"
#define STR_CONNECT_OK_EC20			"+QIOPEN: 0,0"
#define STR_CONNECT_FAIL_EC20		"+QIOPEN: 0,"

#define DEFAULT_SERVER				"101.132.177.116"
#define DEFAULT_PORT				"2011"

#define qistat 				"AT+QISTAT\r\n"
#define qiclose "AT+QICLOSE\r\n"
#define qilocip "AT+QILOCIP\r\n"
#define qilport "AT+QILPORT?\r\n"
#define e0 "ATE0\r\n"
#define cgreg "AT+CGREG?\r\n"
#define cimi "AT+CIMI\r\n"
#define cpin "AT+CPIN?\r\n"
#define qifgcnt "AT+QIFGCNT=0\r\n"
#define qisrvc "AT+QISRVC=1\r\n"
#define qimux "AT+QIMUX=0\r\n"
#define ask_qimux "AT+QIMUX?\r\n"
#define qideact "AT+QIDEACT\r\n"

#define qindi "AT+QINDI=1\r\n"
#define qird "AT+QIRD=0,1,0,1500\r\n"
#define qisack "AT+QISACK\r\n"
#define qiat "AT\r\n"
uint8_t 	  qicsgp_m26[32]			= {0};
uint8_t 	  qiopen_m26[64]			= {0};
uint8_t 	  qisend_m26[32] 			= {0};
#define qiregapp "AT+QIREGAPP\r\n"
#define qiact "AT+QIACT\r\n"
rt_bool_t g_data_in_m26 = RT_FALSE;
extern int g_index;

rt_device_t g_dev_m26;
uint8_t g_m26_state 				= M26_STATE_INIT;
uint32_t server_len_m26 = 0;
uint8_t *server_buf_m26 = RT_NULL;
void *send_data_ptr_m26 = RT_NULL;
rt_size_t send_size_m26;
void handle_m26_server_in(const void *last_data_ptr)
{
	static rt_bool_t flag = RT_FALSE;
	if (have_str(last_data_ptr, STR_TCP))
	{	
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
			server_buf_m26 = (uint8_t *)rt_malloc(server_len_m26 * sizeof(uint8_t));
			rt_memset(server_buf_m26,0,server_len_m26);
			server_len_m26 = 0;
			i+=2;
			while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{

				server_buf_m26[server_len_m26++] = pos[i++];
			}
			if (strstr(pos, "OK")!=RT_NULL)
			{
				rt_data_queue_push(&g_data_queue[3], server_buf_m26, server_len_m26, RT_WAITING_FOREVER);
				if (server_len_m26 == 1500) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				} else {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_m26,qiat);
				}
				if (!have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_FALSE;

				flag = RT_FALSE;
			}
			else
				flag = RT_TRUE;
		}
	}

	else if (flag){	
		int i=0;
		uint8_t *pos = (uint8_t *)last_data_ptr;
		while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
		{
			server_buf_m26[server_len_m26++] = pos[i++];
		}

		if (strstr(pos, "OK")!=RT_NULL)
		{
			rt_data_queue_push(&g_data_queue[3], server_buf_m26, server_len_m26, RT_WAITING_FOREVER);
			if (server_len_m26 == 1500) {
				g_m26_state = M26_STATE_CHECK_QISTAT;
				gprs_at_cmd(g_dev_m26,qistat);
			} else {
				g_m26_state = M26_STATE_DATA_PROCESSING;
				gprs_at_cmd(g_dev_m26,qiat);		
			}
			if (!have_str(last_data_ptr, STR_QIRDI))
				g_data_in_m26 = RT_FALSE;
			flag = RT_FALSE;
		}
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
		power_rcc = RCC_APB2Periph_GPIOE;
		pwr_key_rcc = RCC_APB2Periph_GPIOE;
		power_pin = GPIO_Pin_14;
		pwr_key_pin = GPIO_Pin_14;
		GPIO_power = GPIOE;
		GPIO_pwr = GPIOE;
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

void m26_proc(void *last_data_ptr, rt_size_t data_size)
{
	if (data_size != 6 && strstr(last_data_ptr, STR_QIRD)==NULL && strstr(last_data_ptr, STR_QIURC)==NULL)
		rt_kprintf("\r\n(M26<= %d %d %s)\r\n",g_m26_state, data_size,last_data_ptr);
	if (data_size >= 2) {
		switch (g_m26_state) {
			case M26_STATE_INIT:
				if (have_str(last_data_ptr,STR_RDY)) {
				g_m26_state = M26_STATE_ATE0;
				gprs_at_cmd(g_dev_m26,e0);	
				}
				break;
			case M26_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					g_m26_state = M26_STATE_CHECK_CPIN;
					gprs_at_cmd(g_dev_m26,cpin);
				} else {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(g_dev_m26,e0);
				}
				break;
			case M26_STATE_CHECK_CPIN:
				if (have_str(last_data_ptr,STR_CPIN_READY)) {
					g_m26_state = M26_STATE_CHECK_CGREG;
					gprs_at_cmd(g_dev_m26,cgreg);
				} else if (have_str(last_data_ptr, STR_CPIN))
				{
					rt_thread_delay(100);
					gprs_at_cmd(g_dev_m26,cpin);
				}

				break;
			case M26_STATE_CHECK_CGREG:
				if (have_str(last_data_ptr, STR_CGREG_READY) ||
						have_str(last_data_ptr, STR_CGREG_READY1)) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
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
							mp.socketAddress[0].IP[0],mp.socketAddress[0].IP[1],
							mp.socketAddress[0].IP[2],mp.socketAddress[0].IP[3],
							mp.socketAddress[0].port);
					gprs_at_cmd(g_dev_m26,qiopen_m26);
				}			
				break;
			case M26_STATE_SET_QICLOSE:
				if (have_str(last_data_ptr, STR_CLOSE_OK)) {
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen_m26, 0, 64);
					rt_sprintf(qiopen_m26, "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r\n",
							mp.socketAddress[0].IP[0],mp.socketAddress[0].IP[1],
							mp.socketAddress[0].IP[2],mp.socketAddress[0].IP[3],
							mp.socketAddress[0].port);
					gprs_at_cmd(g_dev_m26,qiopen_m26);
				}
				else
				{
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				}
				break;
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
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen_m26, 0, 64);					
					rt_sprintf(qiopen_m26, "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",\"%d\"\r\n",
							mp.socketAddress[0].IP[0],mp.socketAddress[0].IP[1],
							mp.socketAddress[0].IP[2],mp.socketAddress[0].IP[3],
							mp.socketAddress[0].port);
					gprs_at_cmd(g_dev_m26,qiopen_m26);
				} else {
					/*check error condition*/
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				}
				break;
			case M26_STATE_SET_QIDEACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				} else if (have_str(last_data_ptr, STR_ERROR)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);
				}
				break;						
			case M26_STATE_SET_QIOPEN:
				if (have_str(last_data_ptr, STR_CONNECT_OK)) {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					/*send data here */
					rt_kprintf("connect to server ok\r\n");
					gprs_at_cmd(g_dev_m26,qiat);
				} else if (have_str(last_data_ptr, STR_SOCKET_BUSSY)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(g_dev_m26,qideact);
				}
				else {
					if (!have_str(last_data_ptr, STR_OK)) {
						rt_thread_delay(100*3);
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_m26,qistat);
					}
				}

				break;
			case M26_STATE_DATA_PROCESSING:
				if (have_str(last_data_ptr, STR_QIRDI)||
						have_str(last_data_ptr, STR_QIURC)) {
					/*server data in */
					g_m26_state = M26_STATE_DATA_READ;
					gprs_at_cmd(g_dev_m26,qird);
					server_len_m26 = 0;
					g_data_in_m26 = RT_TRUE;
				} else if (have_str(last_data_ptr, STR_OK)){
					/*check have data to send */
					if (rt_data_queue_peak(&g_data_queue[2],(const void **)&send_data_ptr_m26,&send_size_m26) == RT_EOK)
					{	
						rt_data_queue_pop(&g_data_queue[2], (const void **)&send_data_ptr_m26, &send_size_m26, RT_WAITING_FOREVER);
						rt_kprintf("should send data %d\r\n", send_size_m26);
						rt_sprintf(qisend_m26, "AT+QISEND=%d\r\n", send_size_m26);
						gprs_at_cmd(g_dev_m26,qisend_m26);
						g_m26_state = M26_STATE_DATA_PRE_WRITE;
					} else {								
						rt_thread_delay(100);
						gprs_at_cmd(g_dev_m26,qiat);
					}

				} else {

					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				}

				break;
			case M26_STATE_DATA_READ:
				handle_m26_server_in(last_data_ptr);
				break;
			case M26_STATE_DATA_PRE_WRITE:
				if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
					g_m26_state = M26_STATE_DATA_WRITE;
					rt_device_write(g_pcie[g_index]->dev, 0, send_data_ptr_m26, send_size_m26);	
					rt_event_send(&(g_pcie[g_index]->event), M26_EVENT_0);
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
					//g_m26_state = M26_STATE_DATA_ACK;
					//gprs_at_cmd(g_dev_m26,(qisack);
					g_m26_state = M26_STATE_DATA_PROCESSING;
					gprs_at_cmd(g_dev_m26,qiat);
				} else {
					if (!have_str(last_data_ptr, STR_QIRDI) && 
							!have_str(last_data_ptr, STR_QIURC)) {
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(g_dev_m26,qistat);
					}
				}
				if (have_str(last_data_ptr, STR_QIRDI) ||
						!have_str(last_data_ptr, STR_QIURC))
					g_data_in_m26 = RT_TRUE;
				break;
			case M26_STATE_DATA_ACK:
				if (have_str(last_data_ptr, STR_QISACK)) {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					if (g_data_in_m26 || have_str(last_data_ptr, STR_QIRDI)) {
						g_m26_state = M26_STATE_DATA_READ;
						gprs_at_cmd(g_dev_m26,qird);
						server_len_m26 = 0;
					} else {
						gprs_at_cmd(g_dev_m26,qiat);
					}
				} else if (have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_TRUE;
				else{
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(g_dev_m26,qistat);
				}
				break;		
		}
	}
}
