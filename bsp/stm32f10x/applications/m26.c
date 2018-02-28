#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "pcie.h"
#include "bsp_misc.h"
int g_index 						= 0;
uint8_t g_m26_state 				= M26_STATE_INIT;

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

const uint8_t qistat[] 				= "AT+QISTAT\r\n";
const uint8_t qistat_ec20[] 		= "AT+QISTATE=0,1\r\n";
const uint8_t qiclose[] 			= "AT+QICLOSE\r\n";
const uint8_t qilocip[] 			= "AT+QILOCIP\r\n";
const uint8_t qilport[] 			= "AT+QILPORT?\r\n";
const uint8_t e0[] 					= "ATE0\r\n";
const uint8_t cgreg[] 				= "AT+CGREG?\r\n";
const uint8_t cimi[] 				= "AT+CIMI\r\n";
const uint8_t cpin[] 				= "AT+CPIN?\r\n";
const uint8_t qifgcnt[] 			= "AT+QIFGCNT=0\r\n";
const uint8_t qisrvc[] 				= "AT+QISRVC=1\r\n";
const uint8_t qimux[] 				= "AT+QIMUX=0\r\n";
const uint8_t ask_qimux[] 			= "AT+QIMUX?\r\n";
const uint8_t qideact[]				= "AT+QIDEACT\r\n";
const uint8_t qideact_ec20[]		= "AT+QIDEACT=1\r\n";

const uint8_t qindi[]				= "AT+QINDI=1\r\n";
const uint8_t qird[]				= "AT+QIRD=0,1,0,1500\r\n";
const uint8_t qird_ec20[]			= "AT+QIRD=0,1500\r\r\n";
const uint8_t qisack[]				= "AT+QISACK\r\n";
const uint8_t qiat[]				= "AT\r\n";
uint8_t 	  qicsgp[32]			= {0};
uint8_t 	  qiopen[64]			= {0};
uint8_t 	  qisend[32] 			= {0};
const uint8_t qiregapp[]			= "AT+QIREGAPP\r\n";
const uint8_t qiact[] 				= "AT+QIACT\r\n";
const uint8_t qiact_ec20[] 			= "AT+QIACT=1\r\n";

void m26_start(int index)
{
	rt_uint32_t power_rcc,pwr_key_rcc;
	rt_uint16_t power_pin,pwr_key_pin;
	GPIO_TypeDef* GPIO_power,GPIO_pwr;
	GPIO_InitTypeDef GPIO_InitStructure;
	g_index = index;
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
		rt_kprintf("\r\n(<= %d %d %s)\r\n",g_m26_state, data_size,last_data_ptr);
	if (data_size >= 2) {
		switch (g_m26_state) {
			case M26_STATE_INIT:
				g_m26_state = M26_STATE_ATE0;
				gprs_at_cmd(e0);						
				break;
			case M26_STATE_ATE0:
				if (have_str(last_data_ptr,STR_OK)) {
					g_m26_state = M26_STATE_CHECK_CPIN;
					gprs_at_cmd(cpin);
				}
				break;
			case M26_STATE_CHECK_CPIN:
				if (have_str(last_data_ptr,STR_CPIN_READY)) {
					g_m26_state = M26_STATE_CHECK_CGREG;
					gprs_at_cmd(cgreg);
				} else if (have_str(last_data_ptr, STR_CPIN))
				{
					rt_thread_delay(100);
					gprs_at_cmd(cpin);
				}

				break;
			case M26_STATE_CHECK_CGREG:
				if (have_str(last_data_ptr, STR_CGREG_READY) ||
						have_str(last_data_ptr, STR_CGREG_READY1)) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				} else if(have_str(last_data_ptr, STR_CGREG)) {
					rt_thread_delay(RT_TICK_PER_SECOND);
					gprs_at_cmd(cgreg);
				}
				break;
			case M26_STATE_CHECK_QISTAT:
				if (have_str(last_data_ptr, STR_STAT_INIT) ||
						have_str(last_data_ptr, STR_STAT_DEACT)) {
					g_m26_state = M26_STATE_SET_QIMUX;
					gprs_at_cmd(qimux);
				} else if (have_str(last_data_ptr, STR_STAT_IND)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(qideact);
				} else if (have_str(last_data_ptr, STR_CONNECT_OK)){
					g_m26_state = M26_STATE_SET_QICLOSE;
					gprs_at_cmd(qiclose);
					//g_m26_state = M26_STATE_DATA_PROCESSING;
					/*send data here */
					//rt_kprintf("already connect to server ok\r\n");
				} else if (have_str(last_data_ptr, STR_STAT_CLOSE) ||
						have_str(last_data_ptr, STR_STAT_STATUS) ||
						have_str(last_data_ptr,STR_CONNECT_FAIL)){
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen, 0, 64);
					rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
							server_addr[server_index],server_port[server_index]);
					gprs_at_cmd(qiopen);
				}			
				break;
			case M26_STATE_SET_QICLOSE:
				if (have_str(last_data_ptr, STR_CLOSE_OK)) {
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen, 0, 64);
					rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
							server_addr[server_index],server_port[server_index]);
					gprs_at_cmd(qiopen);
				}
				else
				{
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				}
				break;
			case M26_STATE_SET_QINDI:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_CHECK_CIMI;
					gprs_at_cmd(cimi);
				}
				break;								
			case M26_STATE_SET_QIMUX:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_m26_state = M26_STATE_SET_QINDI;
					gprs_at_cmd(qindi);
				}
				break;		
			case M26_STATE_CHECK_CIMI:
				rt_memset(qicsgp, 0, 32);
				if (have_str(last_data_ptr, STR_4600)) {
					if (have_str(last_data_ptr, STR_46000) ||
							have_str(last_data_ptr, STR_46002) ||
							have_str(last_data_ptr, STR_46004)) {
						rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
					} else if (have_str(last_data_ptr, STR_46001) ||
							have_str(last_data_ptr, STR_46006)){						
						rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "UNINET");
					} else if (have_str(last_data_ptr, STR_46003)){						
						rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CTNET");
					} else {						
						rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
					}
					g_m26_state = M26_STATE_SET_QICSGP;
					gprs_at_cmd(qicsgp);
				}
				else
					gprs_at_cmd(cimi);
				break;						
			case M26_STATE_SET_QICSGP:
				if (have_str(last_data_ptr, STR_OK)) {					
					g_m26_state = M26_STATE_SET_QIREGAPP;
					gprs_at_cmd(qiregapp);					
				}
				break;		
			case M26_STATE_SET_QIREGAPP:
				if (have_str(last_data_ptr, STR_OK) ||
						have_str(last_data_ptr, STR_ERROR)) {
					g_m26_state = M26_STATE_SET_QISRVC;
					gprs_at_cmd(qisrvc);
				}
				break;
			case M26_STATE_SET_QISRVC:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_SET_QIACT;
					gprs_at_cmd(qiact);
				}
				break;		
			case M26_STATE_SET_QIACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_SET_QIOPEN;
					rt_memset(qiopen, 0, 64);					
					rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
							server_addr[server_index],server_port[server_index]);
					gprs_at_cmd(qiopen);
				} else {
					/*check error condition*/
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				}
				break;
			case M26_STATE_SET_QIDEACT:
				if (have_str(last_data_ptr, STR_OK)) {
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				} else if (have_str(last_data_ptr, STR_ERROR)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(qideact);
				}
				break;						
			case M26_STATE_SET_QIOPEN:
				if (have_str(last_data_ptr, STR_CONNECT_OK)) {
					g_m26_state = M26_STATE_DATA_PROCESSING;
					/*send data here */
					rt_kprintf("connect to server ok\r\n");
					gprs_at_cmd(qiat);
				} else if (have_str(last_data_ptr, STR_SOCKET_BUSSY)){
					g_m26_state = M26_STATE_SET_QIDEACT;
					gprs_at_cmd(qideact);
				}
				else {
					if (!have_str(last_data_ptr, STR_OK)) {
						rt_thread_delay(100*3);
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(qistat);
					}
				}

				break;
			case M26_STATE_DATA_PROCESSING:
				if (have_str(last_data_ptr, STR_QIRDI)||
						have_str(last_data_ptr, STR_QIURC)) {
					/*server data in */
					g_m26_state = M26_STATE_DATA_READ;
					gprs_at_cmd(qird);
					server_len = 0;
					g_data_in_m26 = RT_TRUE;
				} else if (have_str(last_data_ptr, STR_OK)){
					/*check have data to send */
					if (rt_data_queue_peak(&g_data_queue[1],(const void **)&send_data_ptr,&send_size) == RT_EOK)
					{	
						rt_data_queue_pop(&g_data_queue[1], (const void **)&send_data_ptr, &send_size, RT_WAITING_FOREVER);
						rt_kprintf("should send data %d\r\n", send_size);
						rt_sprintf(qisend, "AT+QISEND=%d\r\n", send_size);
						gprs_at_cmd(qisend);
						/*uint8_t ch;
						  while (1) {
						  while(rt_device_read(dev_gprs, 0, &ch, 1) != 1);
						  if (ch == '>')
						  break;
						  }
						  rt_kprintf("send ptr %p\r\n",send_data_ptr);
						  rt_device_write(dev_gprs, 0, send_data_ptr, send_size);	
						  rt_free(send_data_ptr);*/
						g_m26_state = M26_STATE_DATA_PRE_WRITE;
					} else {								
						rt_thread_delay(100);
						gprs_at_cmd(qiat);
					}

				} else {

					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				}

				break;
			case M26_STATE_DATA_READ:
				handle_server_in(last_data_ptr);
				break;
			case M26_STATE_DATA_PRE_WRITE:
				if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
					g_m26_state = M26_STATE_DATA_WRITE;
					rt_device_write(dev_gprs, 0, send_data_ptr, send_size);	
					//if (send_data_ptr != RT_NULL) {
					//rt_free(send_data_ptr);
					//send_data_ptr = RT_NULL;
					//}
					rt_event_send(&gprs_event, M26_EVENT_0);
				}
				else if(have_str(last_data_ptr, STR_ERROR))
				{
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				}
				if (have_str(last_data_ptr, STR_QIRDI)||have_str(last_data_ptr, STR_QIURC))
					g_data_in_m26 = RT_TRUE;
				break;
			case M26_STATE_DATA_WRITE:
				if (have_str(last_data_ptr, STR_SEND_OK)) {
					//g_m26_state = M26_STATE_DATA_ACK;
					//gprs_at_cmd(qisack);
					g_m26_state = M26_STATE_DATA_PROCESSING;
					gprs_at_cmd(qiat);
				} else {
					if (!have_str(last_data_ptr, STR_QIRDI) && 
							!have_str(last_data_ptr, STR_QIURC)) {
						g_m26_state = M26_STATE_CHECK_QISTAT;
						gprs_at_cmd(qistat);
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
						gprs_at_cmd(qird);
						server_len = 0;
					} else {
						gprs_at_cmd(qiat);
					}
				} else if (have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_TRUE;
				else{
					g_m26_state = M26_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				}
				break;		
		}
	}
}
