#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include "cc1101.h"
#include <string.h>
#include "bsp_misc.h"
#define GPRS_POWER_PORT GPIOE
#define GPRS_POWER_PIN	GPIO_Pin_14
#define GPRS_POWER_RCC	RCC_APB2Periph_GPIOE

#define G4_POWER_PORT 	GPIOC
#define G4_POWER_PIN	GPIO_Pin_8
#define G4_POWER_RCC	RCC_APB2Periph_GPIOC
#define G4_STATUS_PORT 	GPIOA
#define G4_STATUS_PIN	GPIO_Pin_10
#define G4_STATUS_RCC	RCC_APB2Periph_GPIOA
#define G4_RESET_PORT 	GPIOC
#define G4_RESET_PIN	GPIO_Pin_9
#define G4_RESET_RCC	RCC_APB2Periph_GPIOC
#define G4_DTR_PORT 	GPIOA
#define G4_DTR_PIN		GPIO_Pin_12
#define G4_DTR_RCC		RCC_APB2Periph_GPIOA

#define MAGIC_OK	"OK"
struct rt_event gprs_event;
static struct rt_mutex gprs_lock;
#define GPRS_EVENT_0 (1<<0)
#define GPRS_STATE_INIT			0
#define GPRS_STATE_CHECK_CPIN	1
#define GPRS_STATE_SET_QINDI	2
#define GPRS_STATE_SET_QIMUX	3
#define GPRS_STATE_CHECK_CGREG	4
#define GPRS_STATE_CHECK_QISTAT	5
#define GPRS_STATE_SET_QICSGP	6
#define GPRS_STATE_SET_QIREGAPP	7
#define GPRS_STATE_SET_QISRVC	8
#define GPRS_STATE_SET_QIACT	9
#define GPRS_STATE_CHECK_CIMI	10
#define GPRS_STATE_SET_QIFGCNT	11
#define GPRS_STATE_SET_QIOPEN 	12
#define GPRS_STATE_SET_QIDEACT  13
#define GPRS_STATE_DATA_PROCESSING 14
#define GPRS_STATE_DATA_READ		15
#define GPRS_STATE_SET_QICLOSE		16
#define GPRS_STATE_DATA_WRITE		17
#define GPRS_STATE_DATA_ACK			18
#define GPRS_STATE_DATA_PRE_WRITE	19
#define GPRS_STATE_ATE0 21

#define STR_CPIN			"+CPIN:"
#define STR_CPIN_READY		"+CPIN: READY"
#define STR_CGREG		"+CGREG: 0,"

#define STR_CGREG_READY		"+CGREG: 0,1"
#define STR_CGREG_READY1	"+CGREG: 0,5"
#define STR_STAT_INIT		"IP INITIAL"
#define STR_STAT_IND		"IP IND"
#define STR_STAT_CLOSE		"IP CLOSE"
#define STR_STAT_STATUS		"IP STATUS"
#define STR_STAT_DEACT		"PDP DEACT"
#define STR_QIMUX_0			"+QIMUX: 0"
#define STR_OK				"OK"
#define STR_QIRD				"+QIRD:"
#define STR_QIURC			"+QIURC:"
#define STR_TCP				"TCP,"
#define STR_CLOSED				"CLOSED"
#define STR_BEGIN_WRITE		">"
#define STR_ERROR			"ERROR"
#define STR_4600			"4600"
#define STR_46000			"46000"
#define STR_46001			"46001"
#define STR_46002			"46002"
#define STR_46003			"46003"
#define STR_46004			"46004"
#define STR_46006			"46006"
#define STR_STAT_DEACT_OK 	"DEACT OK"
#define STR_CONNECT_OK		"CONNECT OK"
#define STR_CLOSE_OK		"CLOSE OK"
#define STR_SEND_OK			"SEND OK"
#define STR_QIRDI			"+QIRDI:"
#define STR_QISACK			"+QISACK"
#define STR_SOCKET_BUSSY	"SOCKET BUSY"
#define STR_CONNECT_FAIL	"CONNECT FAIL"
#define STR_CONNECT_OK_EC20		"+QIOPEN: 0,0"
#define STR_CONNECT_FAIL_EC20		"+QIOPEN: 0,"

//#define DEFAULT_SERVER		"106.3.45.71"
#define DEFAULT_SERVER		"101.132.177.116"
//#define DEFAULT_SERVER		"106.3.45.71"
//#define DEFAULT_PORT		"60002"
#define DEFAULT_PORT		"2011"
#define DATA_BEGIN0			0x40
#define DATA_BEGIN1			0x41
const uint8_t qistat[] 		= "AT+QISTAT\r\n";
const uint8_t qistat_ec20[] = "AT+QISTATE=0,1\r\n";

const uint8_t qiclose[] 	= "AT+QICLOSE\r\n";
const uint8_t qilocip[] 	= "AT+QILOCIP\r\n";
const uint8_t qilport[] 	= "AT+QILPORT?\r\n";
const uint8_t e0[] 			= "ATE0\r\n";
const uint8_t cgreg[] 		= "AT+CGREG?\r\n";
const uint8_t cimi[] 		= "AT+CIMI\r\n";
const uint8_t cpin[] 		= "AT+CPIN?\r\n";
const uint8_t qifgcnt[] 	= "AT+QIFGCNT=0\r\n";
const uint8_t qisrvc[] 		= "AT+QISRVC=1\r\n";
const uint8_t qimux[] 		= "AT+QIMUX=0\r\n";
const uint8_t ask_qimux[] 	= "AT+QIMUX?\r\n";
const uint8_t qideact[]		= "AT+QIDEACT\r\n";
const uint8_t qideact_ec20[]		= "AT+QIDEACT=1\r\n";

const uint8_t qindi[]		= "AT+QINDI=1\r\n";
const uint8_t qird[]		= {"AT+QIRD=0,1,0,1500\r\n"};
const uint8_t qird_ec20[]		= {"AT+QIRD=0,1500\r\r\n"};
const uint8_t qisack[]	= "AT+QISACK\r\n";
const uint8_t qiat[]	="AT\r\n";
uint8_t 	  qicsgp[32]	= {0};
uint8_t 	  qiopen[64]	= {0};
uint8_t 	  qisend[32] 	= {0};
const uint8_t qiregapp[]	= "AT+QIREGAPP\r\n";
const uint8_t qiact[] 		= "AT+QIACT\r\n";
const uint8_t qiact_ec20[] 		= "AT+QIACT=1\r\n";
uint8_t server_addr[5][32] = {0};
uint8_t server_port[5][8] = {0};
uint8_t server_index = 0;
rt_bool_t m26_module_use = RT_TRUE;
rt_bool_t g_data_in_m26 = RT_FALSE;
uint8_t g_gprs_state = GPRS_STATE_INIT;
rt_device_t dev_gprs;
struct rt_semaphore gprs_rx_sem;
struct rt_data_queue *g_data_queue;
uint8_t *server_buf = RT_NULL;
uint32_t server_len = 0;
uint32_t g_size = 0;
uint8_t g_type = 0;
static rt_err_t gprs_rx_ind(rt_device_t dev, rt_size_t size)
{
	//rt_kprintf("info_len %d\r\n",size);
	g_size = size;
	rt_sem_release(&(gprs_rx_sem));
	return RT_EOK;
}
/*read data from usart , put to dataqueue */
void gprs_rcv(void* parameter)
{	
	uint32_t len = 0, total_len = 0;
	uint8_t *buf = rt_malloc(1600);
	while(1)	
	{			
		rt_sem_take(&gprs_rx_sem, RT_WAITING_FOREVER);
		while (1) {
			len = rt_device_read(dev_gprs, 0, &(buf[total_len]) , 1600-total_len);

			if (len>0)
			{
				total_len += len;
				buf[total_len] = '\0';
			}
			else
				break;			
		}
		//rt_kprintf("==>%s", buf);
		if (total_len >= 4 && buf[total_len-2] == '\r' && buf[total_len-1] == '\n' || strchr(buf,'>')!=RT_NULL) {
			uint8_t *rcv = (uint8_t *)rt_malloc(total_len+1);
			rt_memcpy(rcv, buf, total_len);
			rcv[total_len] = '\0';
			rt_data_queue_push(&g_data_queue[0], rcv, total_len, RT_WAITING_FOREVER);
			total_len = 0;
		}
	}
}

void m26_restart(void)
{
	/*use gpio to identify different module*/
	if (g_type == 1 && GPIO_ReadInputDataBit(G4_STATUS_PORT,G4_STATUS_PIN)==SET) {
		rt_kprintf("power up g4\r\n");
		GPIO_ResetBits(G4_POWER_PORT, G4_POWER_PIN);
	}
	if (g_type == 0) 
		GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PIN);
	rt_thread_delay(RT_TICK_PER_SECOND);
	if (g_type == 0)
		GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PIN);
	if (g_type == 1)
	{		
		GPIO_SetBits(G4_POWER_PORT, G4_POWER_PIN);
	}

}
void change_baud(int baud)
{
	struct serial_configure config;			
	config.baud_rate = baud;
	config.bit_order = BIT_ORDER_LSB;			
	config.data_bits = DATA_BITS_8;			
	config.parity	 = PARITY_NONE;			
	config.stop_bits = STOP_BITS_1;				
	config.invert	 = NRZ_NORMAL;				
	config.bufsz	 = RT_SERIAL_RB_BUFSZ;			
	rt_device_control(dev_gprs,RT_DEVICE_CTRL_CONFIG,&config);	
}
rt_bool_t have_str(const char *str, const char *magic)
{
	if (strstr(str, magic) != RT_NULL)
		return RT_TRUE;

	return RT_FALSE;
}
void gprs_at_cmd(const char *cmd)
{
	if (strcmp(cmd, qiat) != 0)
		rt_kprintf("=> %s",cmd);
	rt_device_write(dev_gprs, 0, (void *)cmd, rt_strlen(cmd));
}
void handle_server_in_ec20(const void *last_data_ptr)
	{
		static rt_bool_t flag = RT_FALSE;
		//rt_kprintf("in_ec20 \r\n");
		if (have_str(last_data_ptr, STR_OK) && !have_str(last_data_ptr, STR_QIRD) && !flag)
			return ;
		
		if (have_str(last_data_ptr, STR_QIRD))
		{	
			uint8_t *pos = (uint8_t *)strstr(last_data_ptr,STR_QIRD);
			
			if (pos != RT_NULL) {
				int i = 7;
				rt_kprintf("\r\n<>%c%c%c%c%c%c%c%c%c%c%c%c<>\r\n",pos[0],pos[1],pos[2],pos[3],pos[4],pos[5],pos[6],pos[7],
					pos[8],pos[9],pos[10],pos[11]);
				while (pos[i] != '\r' && pos[i+1] != '\n' && i<strlen(pos))
				{
					//rt_kprintf("%c",pos[i]);
					server_len = server_len*10 + pos[i] - '0';
					i++;
				}
				//rt_kprintf("server len %d\r\n", server_len);
				server_buf = (uint8_t *)rt_malloc(server_len * sizeof(uint8_t));
				rt_memset(server_buf,0,server_len);
				server_len = 0;
				i+=2;
				while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
				{
	
					server_buf[server_len++] = pos[i++];
					//rt_kprintf("%c", server_buf[server_len-1]);
				}
				//rt_kprintf("\r\n<<<>>>\r\n");
				//rt_kprintf("%c %c %c %c\r\n",pos[i],pos[i+1],pos[i+2],pos[i+3]);
				if (/*pos[i]=='\r' &&pos[i+1]=='\n' &&pos[i+2]=='O' &&pos[i+3]=='K'*/strstr(pos, "OK")!=RT_NULL)
				{
					//rt_kprintf("{%s}\r\n",server_buf);
					//rt_kprintf("free server buf %d\r\n",server_len);
					rt_data_queue_push(&g_data_queue[2], server_buf, server_len, RT_WAITING_FOREVER);
					//rt_free(server_buf);	
					//rt_kprintf("free server buf>\r\n");
					if (server_len == 1500) {
						g_gprs_state = GPRS_STATE_CHECK_QISTAT;
						gprs_at_cmd(qistat_ec20);
					} else {
						g_gprs_state = GPRS_STATE_DATA_PROCESSING;
						gprs_at_cmd(qiat);
					}
					if (!have_str(last_data_ptr, STR_QIRDI))
						g_data_in_m26 = RT_FALSE;
					//else
					//	rt_kprintf("still have server data in\r\n");
	
					flag = RT_FALSE;
					//rt_mutex_release(&gprs_lock);
				}
				else
					flag = RT_TRUE;
				/*handle server request
				  put to another dataqueue
				  */
			}
		}
	
		else if (flag){ 
			int i=0;
			//rt_kprintf("><%d><%d\r\n",server_len,data_size);
			///rt_kprintf("\r\n");
			uint8_t *pos = (uint8_t *)last_data_ptr;
			while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{
				server_buf[server_len++] = pos[i++];
				//rt_kprintf("%c", server_buf[server_len-1]);
			}
			//rt_kprintf("\r\n<><<>>\r\n");
			//rt_kprintf("%c %c %c %c\r\n",pos[i],pos[i+1],pos[i+2],pos[i+3]);
	
			if (/*pos[i]=='\r' &&pos[i+1]=='\n' &&pos[i+2]=='O' &&pos[i+3]=='K'*/strstr(pos, "OK")!=RT_NULL)
			{
				//rt_kprintf("\r\nfree server buf2 %d\r\n",server_len);
				rt_data_queue_push(&g_data_queue[2], server_buf, server_len, RT_WAITING_FOREVER);
				//rt_free(server_buf);						
				//rt_kprintf("\r\nfree server buf2>\r\n");
				if (server_len == 1500) {
					g_gprs_state = GPRS_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				} else {
					g_gprs_state = GPRS_STATE_DATA_PROCESSING;
					gprs_at_cmd(qiat);		
				}
				if (!have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_FALSE;
				//else
				//rt_kprintf("still have server data in 2\r\n");
	
				//rt_mutex_release(&gprs_lock);
				flag = RT_FALSE;
			}
		}
	}

void handle_server_in(const void *last_data_ptr)
{
	static rt_bool_t flag = RT_FALSE;
	if (/*have_str(last_data_ptr, STR_QIRD) && */have_str(last_data_ptr, STR_TCP))
	{	
		uint8_t *begin = (uint8_t *)strstr(last_data_ptr,STR_QIRD);
		uint8_t *pos = RT_NULL;
		if (begin == RT_NULL)
			pos = (uint8_t *)strstr(last_data_ptr, STR_TCP);
		else
			pos = (uint8_t *)strstr(begin, STR_TCP);
		if (pos != RT_NULL) {
			int i = 4;
			//rt_kprintf("\r\n<><>\r\n");
			while (pos[i] != '\r' && pos[i+1] != '\n' && i<strlen(pos))
			{
				//rt_kprintf("%c",pos[i]);
				server_len = server_len*10 + pos[i] - '0';
				i++;
			}
			//rt_kprintf("server len %d\r\n", server_len);
			server_buf = (uint8_t *)rt_malloc(server_len * sizeof(uint8_t));
			rt_memset(server_buf,0,server_len);
			server_len = 0;
			i+=2;
			while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
			{

				server_buf[server_len++] = pos[i++];
				//rt_kprintf("%c", server_buf[server_len-1]);
			}
			//rt_kprintf("\r\n<<<>>>\r\n");
			//rt_kprintf("%c %c %c %c\r\n",pos[i],pos[i+1],pos[i+2],pos[i+3]);
			if (/*pos[i]=='\r' &&pos[i+1]=='\n' &&pos[i+2]=='O' &&pos[i+3]=='K'*/strstr(pos, "OK")!=RT_NULL)
			{
				//rt_kprintf("{%s}\r\n",server_buf);
				//rt_kprintf("free server buf %d\r\n",server_len);
				rt_data_queue_push(&g_data_queue[2], server_buf, server_len, RT_WAITING_FOREVER);
				//rt_free(server_buf);	
				//rt_kprintf("free server buf>\r\n");
				if (server_len == 1500) {
					g_gprs_state = GPRS_STATE_CHECK_QISTAT;
					gprs_at_cmd(qistat);
				} else {
					g_gprs_state = GPRS_STATE_DATA_PROCESSING;
					gprs_at_cmd(qiat);
				}
				if (!have_str(last_data_ptr, STR_QIRDI))
					g_data_in_m26 = RT_FALSE;
				//else
				//	rt_kprintf("still have server data in\r\n");

				flag = RT_FALSE;
				//rt_mutex_release(&gprs_lock);
			}
			else
				flag = RT_TRUE;
			/*handle server request
			  put to another dataqueue
			  */
		}
	}

	else if (flag){	
		int i=0;
		//rt_kprintf("><%d><%d\r\n",server_len,data_size);
		///rt_kprintf("\r\n");
		uint8_t *pos = (uint8_t *)last_data_ptr;
		while(i<strlen(pos) && pos[i]!='\r' &&pos[i+1]!='\n' &&pos[i+2]!='O' &&pos[i+3]!='K' && pos[i]!=0x1e &&pos[i+1]!=0x01)
		{
			server_buf[server_len++] = pos[i++];
			//rt_kprintf("%c", server_buf[server_len-1]);
		}
		//rt_kprintf("\r\n<><<>>\r\n");
		//rt_kprintf("%c %c %c %c\r\n",pos[i],pos[i+1],pos[i+2],pos[i+3]);

		if (/*pos[i]=='\r' &&pos[i+1]=='\n' &&pos[i+2]=='O' &&pos[i+3]=='K'*/strstr(pos, "OK")!=RT_NULL)
		{
			//rt_kprintf("\r\nfree server buf2 %d\r\n",server_len);
			rt_data_queue_push(&g_data_queue[2], server_buf, server_len, RT_WAITING_FOREVER);
			//rt_free(server_buf);						
			//rt_kprintf("\r\nfree server buf2>\r\n");
			if (server_len == 1500) {
				g_gprs_state = GPRS_STATE_CHECK_QISTAT;
				gprs_at_cmd(qistat);
			} else {
				g_gprs_state = GPRS_STATE_DATA_PROCESSING;
				gprs_at_cmd(qiat);		
			}
			if (!have_str(last_data_ptr, STR_QIRDI))
				g_data_in_m26 = RT_FALSE;
			//else
			//rt_kprintf("still have server data in 2\r\n");

			//rt_mutex_release(&gprs_lock);
			flag = RT_FALSE;
		}
	}
}
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &gprs_event, GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}
uint8_t m26_send(const uint8_t *data, uint32_t send_len)
{
	uint8_t qisend[32] 	= {0};
	uint8_t *rcv;
	uint32_t len;
	//while (g_gprs_state != GPRS_STATE_DATA_PROCESSING)
	//	rt_thread_delay(100);
	//rt_mutex_take(&gprs_lock, RT_WAITING_FOREVER);
	//g_gprs_state = GPRS_STATE_DATA_PRE_WRITE;
	//rt_sprintf(qisend, "AT+QISEND=%d\n", send_len);
	//gprs_at_cmd(qisend);
	//gprs_wait_event(RT_WAITING_FOREVER);
	//rt_device_write(dev_gprs, 0, data, send_len);
	//gprs_wait_event(RT_WAITING_FOREVER);
	//rt_mutex_release(&gprs_lock);
	return 0;
}

void gprs_process(void* parameter)
{
	uint8_t *rcv_server = RT_NULL;
	uint8_t cpin_count = 0;
	rt_size_t data_size;
	rt_size_t send_size;
	const void *last_data_ptr = RT_NULL;
	void *send_data_ptr = RT_NULL;
	//new module need reset to 115200 baud
	//gprs_at_cmd("AT+IPR?\r\n");
	//rt_thread_delay(200);
	//gprs_at_cmd("AT+IPR=115200;&W\r\n");
	//rt_thread_delay(200);
	//gprs_at_cmd("AT&W\r\n");
	//rt_thread_delay(200);
	//gprs_at_cmd(e0);
	if (g_type == 1 && GPIO_ReadInputDataBit(G4_STATUS_PORT,G4_STATUS_PIN)==RESET)
	{
		gprs_at_cmd(qiat);
		rt_thread_delay(50);
		gprs_at_cmd(qiat);
	}
	while (1) {
		rt_err_t r = rt_data_queue_pop(&g_data_queue[0], &last_data_ptr, &data_size, RT_WAITING_FOREVER);

		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			if (data_size != 6 && strstr(last_data_ptr, STR_QIRD)==NULL && strstr(last_data_ptr, STR_QIURC)==NULL)
			rt_kprintf("\r\n(<= %d %d %s)\r\n",g_gprs_state, data_size,last_data_ptr);
			if (data_size >= 2) {
				switch (g_gprs_state) {
					case GPRS_STATE_INIT:
						g_gprs_state = GPRS_STATE_ATE0;
						gprs_at_cmd(e0);						
						break;
					case GPRS_STATE_ATE0:
						if (have_str(last_data_ptr,STR_OK)) {
							g_gprs_state = GPRS_STATE_CHECK_CPIN;
							gprs_at_cmd(cpin);
						}
						break;
					case GPRS_STATE_CHECK_CPIN:
						if (have_str(last_data_ptr,STR_CPIN_READY)) {
							g_gprs_state = GPRS_STATE_CHECK_CGREG;
							gprs_at_cmd(cgreg);
						} else if (have_str(last_data_ptr, STR_CPIN))
						{
							rt_thread_delay(100);
							gprs_at_cmd(cpin);
							if (cpin_count > 3)
							{
								m26_module_use = RT_FALSE;
								return;
							}
							cpin_count ++;
						}

						break;
					case GPRS_STATE_CHECK_CGREG:
						if (have_str(last_data_ptr, STR_CGREG_READY) ||
								have_str(last_data_ptr, STR_CGREG_READY1)) {
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							if (g_type == 0)
								gprs_at_cmd(qistat);
							else
								gprs_at_cmd(qistat_ec20);
						} else if(have_str(last_data_ptr, STR_CGREG)) {
							rt_thread_delay(RT_TICK_PER_SECOND);
							gprs_at_cmd(cgreg);
						}
						break;
					case GPRS_STATE_CHECK_QISTAT:
						if (g_type == 0) {
							if (have_str(last_data_ptr, STR_STAT_INIT) ||
									have_str(last_data_ptr, STR_STAT_DEACT)) {
								g_gprs_state = GPRS_STATE_SET_QIMUX;
								gprs_at_cmd(qimux);
							} else if (have_str(last_data_ptr, STR_STAT_IND)){
								g_gprs_state = GPRS_STATE_SET_QIDEACT;
								gprs_at_cmd(qideact);
							} else if (have_str(last_data_ptr, STR_CONNECT_OK)){
								g_gprs_state = GPRS_STATE_SET_QICLOSE;
								gprs_at_cmd(qiclose);
								//g_gprs_state = GPRS_STATE_DATA_PROCESSING;
								/*send data here */
								//rt_kprintf("already connect to server ok\r\n");
							} else if (have_str(last_data_ptr, STR_STAT_CLOSE) ||
									have_str(last_data_ptr, STR_STAT_STATUS) ||
									have_str(last_data_ptr,STR_CONNECT_FAIL)){
								g_gprs_state = GPRS_STATE_SET_QIOPEN;
								rt_memset(qiopen, 0, 64);
								rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
										server_addr[server_index],server_port[server_index]);
								gprs_at_cmd(qiopen);
							} 							
						}else if (g_type == 1) {
							if(have_str(last_data_ptr, STR_OK)) { //e20
								g_gprs_state = GPRS_STATE_CHECK_CIMI;
								gprs_at_cmd(cimi);
							}
						}
						break;
					case GPRS_STATE_SET_QICLOSE:
						if (have_str(last_data_ptr, STR_CLOSE_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIOPEN;
							rt_memset(qiopen, 0, 64);
							rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
									server_addr[server_index],server_port[server_index]);
							gprs_at_cmd(qiopen);
						}
						else
						{
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							gprs_at_cmd(qistat);
						}
						break;
					case GPRS_STATE_SET_QINDI:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_CHECK_CIMI;
							gprs_at_cmd(cimi);
						}
						break;								
					case GPRS_STATE_SET_QIMUX:
						if (have_str(last_data_ptr, STR_OK) ||
								have_str(last_data_ptr, STR_ERROR)) {
							g_gprs_state = GPRS_STATE_SET_QINDI;
							gprs_at_cmd(qindi);
						}
						break;		
					case GPRS_STATE_CHECK_CIMI:
						rt_memset(qicsgp, 0, 32);
						if (have_str(last_data_ptr, STR_4600)) {
							if (have_str(last_data_ptr, STR_46000) ||
									have_str(last_data_ptr, STR_46002) ||
									have_str(last_data_ptr, STR_46004)) {
								if (g_type == 0)
									rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
								if (g_type == 1)
									rt_sprintf(qicsgp, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CMNET");
							} else if (have_str(last_data_ptr, STR_46001) ||
									have_str(last_data_ptr, STR_46006)){
								if (g_type == 0)
									rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "UNINET");
								if (g_type == 1)
									rt_sprintf(qicsgp, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "UNINET");
							} else if (have_str(last_data_ptr, STR_46003)){
								if (g_type == 0)
									rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CTNET");
								if (g_type == 1)
									rt_sprintf(qicsgp, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CTNET");
							} else {
								if (g_type == 0)
									rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\r\n", "CMNET");
								if (g_type ==1)
									rt_sprintf(qicsgp, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n", "CMNET");
							}
							g_gprs_state = GPRS_STATE_SET_QICSGP;
							gprs_at_cmd(qicsgp);
						}
						else
							gprs_at_cmd(cimi);
						break;						
					case GPRS_STATE_SET_QICSGP:
						if (have_str(last_data_ptr, STR_OK)) {
							if (g_type == 0) {
								g_gprs_state = GPRS_STATE_SET_QIREGAPP;
								gprs_at_cmd(qiregapp);
							} else if(g_type == 1) {
								g_gprs_state = GPRS_STATE_SET_QIACT;
								gprs_at_cmd(qiact_ec20);
							}
						}
						break;		
					case GPRS_STATE_SET_QIREGAPP:
						if (have_str(last_data_ptr, STR_OK) ||
								have_str(last_data_ptr, STR_ERROR)) {
							g_gprs_state = GPRS_STATE_SET_QISRVC;
							gprs_at_cmd(qisrvc);
						}
						break;
					case GPRS_STATE_SET_QISRVC:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIACT;
							gprs_at_cmd(qiact);
						}
						break;		
					case GPRS_STATE_SET_QIACT:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIOPEN;
							rt_memset(qiopen, 0, 64);
							if (g_type == 0)
								rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\r\n",
										server_addr[server_index],server_port[server_index]);
							else if(g_type ==1)
								rt_sprintf(qiopen, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%s,0,0\r\n",
										server_addr[server_index],server_port[server_index]);
							gprs_at_cmd(qiopen);
						} else {
							/*check error condition*/
							if (g_type == 0) {
								g_gprs_state = GPRS_STATE_CHECK_QISTAT;
								gprs_at_cmd(qistat);
							}
							else if(g_type == 1)
							{	g_gprs_state = GPRS_STATE_SET_QIDEACT;
								gprs_at_cmd(qideact_ec20);
							}
						}
						break;
					case GPRS_STATE_SET_QIDEACT:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							if (g_type == 0)
								gprs_at_cmd(qistat);
							else
								gprs_at_cmd(qistat_ec20);
						} else if (have_str(last_data_ptr, STR_ERROR)){
							g_gprs_state = GPRS_STATE_SET_QIDEACT;
							gprs_at_cmd(qideact);
						}
						break;						
					case GPRS_STATE_SET_QIOPEN:
						if (g_type == 0) {
							if (have_str(last_data_ptr, STR_CONNECT_OK)) {
								g_gprs_state = GPRS_STATE_DATA_PROCESSING;
								/*send data here */
								rt_kprintf("connect to server ok\r\n");
								gprs_at_cmd(qiat);
							} else if (have_str(last_data_ptr, STR_SOCKET_BUSSY)){
								g_gprs_state = GPRS_STATE_SET_QIDEACT;
								gprs_at_cmd(qideact);
							}
							else {
								if (!have_str(last_data_ptr, STR_OK)) {
									rt_thread_delay(100*3);
									g_gprs_state = GPRS_STATE_CHECK_QISTAT;
									gprs_at_cmd(qistat);
								}
							}
						} else {
							if (have_str(last_data_ptr, STR_CONNECT_OK_EC20)) {
								/*send data here */
								rt_kprintf("connect to server ok\r\n");
								g_gprs_state = GPRS_STATE_DATA_PROCESSING;
								gprs_at_cmd(qiat);
							} else if (have_str(last_data_ptr, STR_CONNECT_FAIL_EC20) ||
									have_str(last_data_ptr, STR_ERROR))
							{/*need detail error , then try different ways*/
								g_gprs_state = GPRS_STATE_SET_QIDEACT;
								gprs_at_cmd(qideact_ec20);
							}
						}
						break;
					case GPRS_STATE_DATA_PROCESSING:
						if (have_str(last_data_ptr, STR_QIRDI)||
						 	have_str(last_data_ptr, STR_QIURC)) {
							/*server data in */
							g_gprs_state = GPRS_STATE_DATA_READ;
							if (g_type == 0)
								gprs_at_cmd(qird);
							else
								gprs_at_cmd(qird_ec20);
							server_len = 0;
							g_data_in_m26 = RT_TRUE;
						} else if (have_str(last_data_ptr, STR_OK)){
							/*check have data to send */
							if (rt_data_queue_peak(&g_data_queue[1],(const void **)&send_data_ptr,&send_size) == RT_EOK)
							{	
								rt_data_queue_pop(&g_data_queue[1], (const void **)&send_data_ptr, &send_size, RT_WAITING_FOREVER);
								rt_kprintf("should send data %d\r\n", send_size);
								if (g_type ==0)
								rt_sprintf(qisend, "AT+QISEND=%d\r\n", send_size);
								else
								rt_sprintf(qisend, "AT+QISEND=0,%d\r\n", send_size);	
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
								g_gprs_state = GPRS_STATE_DATA_PRE_WRITE;
							} else {								
								rt_thread_delay(100);
								gprs_at_cmd(qiat);
							}

						} else {
							if (g_type == 0) 
							{
								g_gprs_state = GPRS_STATE_CHECK_QISTAT;
								gprs_at_cmd(qistat);
							} else {
								/*for ec20 need more check*/
							}
						}
							
						break;
					case GPRS_STATE_DATA_READ:
						if (g_type == 0)
							handle_server_in(last_data_ptr);
						else
							handle_server_in_ec20(last_data_ptr);
						break;
					case GPRS_STATE_DATA_PRE_WRITE:
						if (have_str(last_data_ptr, STR_BEGIN_WRITE)) {
							g_gprs_state = GPRS_STATE_DATA_WRITE;
							rt_device_write(dev_gprs, 0, send_data_ptr, send_size);	
							//if (send_data_ptr != RT_NULL) {
							//rt_free(send_data_ptr);
							//send_data_ptr = RT_NULL;
							//}
							rt_event_send(&gprs_event, GPRS_EVENT_0);
						}
						else if(have_str(last_data_ptr, STR_ERROR))
						{
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							if (g_type ==0)
							gprs_at_cmd(qistat);
							else
							gprs_at_cmd(qistat_ec20);
						}
						if (have_str(last_data_ptr, STR_QIRDI)||have_str(last_data_ptr, STR_QIURC))
							g_data_in_m26 = RT_TRUE;
						break;
					case GPRS_STATE_DATA_WRITE:
						if (have_str(last_data_ptr, STR_SEND_OK)) {
							//g_gprs_state = GPRS_STATE_DATA_ACK;
							//gprs_at_cmd(qisack);
							g_gprs_state = GPRS_STATE_DATA_PROCESSING;
							gprs_at_cmd(qiat);
						} else {
							if (!have_str(last_data_ptr, STR_QIRDI) && 
								!have_str(last_data_ptr, STR_QIURC)) {
								g_gprs_state = GPRS_STATE_CHECK_QISTAT;
								gprs_at_cmd(qistat);
							}
						}
						if (have_str(last_data_ptr, STR_QIRDI) ||
							!have_str(last_data_ptr, STR_QIURC))
							g_data_in_m26 = RT_TRUE;
						break;
					case GPRS_STATE_DATA_ACK:
						if (have_str(last_data_ptr, STR_QISACK)) {
							g_gprs_state = GPRS_STATE_DATA_PROCESSING;
							if (g_data_in_m26 || have_str(last_data_ptr, STR_QIRDI)) {
								g_gprs_state = GPRS_STATE_DATA_READ;
								gprs_at_cmd(qird);
								server_len = 0;
							} else {
								gprs_at_cmd(qiat);
							}
						} else if (have_str(last_data_ptr, STR_QIRDI))
							g_data_in_m26 = RT_TRUE;
						else{
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							gprs_at_cmd(qistat);
						}
						break;		
				}
			}
			if (last_data_ptr != RT_NULL) {
				rt_free((void *)last_data_ptr);
				last_data_ptr = RT_NULL;
			}
		}
	}

}
void send_process(void* parameter)
{
	int i=0;
	while(1)	{		
		char id[10] = {0};	
		char data[10] = {0};
		rt_thread_delay(500);
		rt_sprintf(id, "id>%d", i);
		rt_sprintf(data, "data>%d", i);
		char *json = add_item(RT_NULL,id, data);
		json = add_item(json,"id1", "data1");
		json = add_item(json,"id2", "data2");
		json = add_item(json,"id3", "data3");
		if (json != RT_NULL) {		
			int len = rt_strlen(json) + 2;
			//uint8_t *buf = (uint8_t *)rt_malloc(len + 4);
			//buf[0] = DATA_BEGIN0;buf[1] = DATA_BEGIN1;
			//buf[2] = (len >> 8) & 0xff;
			//buf[3] = len & 0xff;
			//rt_memcpy(buf+4, json, rt_strlen(json));
			//rt_uint16_t crc = CRC_check(buf, len+2);
			//buf[len+2] = (crc >> 8) & 0xff;
			//buf[len+3] = crc & 0xff;
			//rt_kprintf("push ptr %p\r\n",buf);
			rt_kprintf("push ptr %p\r\n",json);
			//rt_data_queue_push(&g_data_queue[1], buf, len+4, RT_WAITING_FOREVER);
			rt_data_queue_push(&g_data_queue[1], json, rt_strlen(json), RT_WAITING_FOREVER);
			gprs_wait_event(RT_WAITING_FOREVER);
			rt_free(json);
			//rt_free(buf);
			i++;
		} else 
			rt_kprintf("send process malloc failed\r\n");
	}
}
void server_process(void* parameter)
{	
	rt_size_t data_size;
	const void *last_data_ptr = RT_NULL;
	while (1) {
		rt_err_t r = rt_data_queue_pop(&g_data_queue[2], &last_data_ptr, &data_size, RT_WAITING_FOREVER);
		rt_kprintf("<<%d ",data_size);
		for (int i=0; i<data_size;i++)
			rt_kprintf("%c", ((char *)last_data_ptr)[i]);
		rt_kprintf(">>\r\n");
		rt_free((void *)last_data_ptr);
	}
}

int gprs_init(void)
{
	/*handle m26*/
	//rt_thread_delay(1000);
	//dev_gprs=rt_device_find("uart2"); //m26
	dev_gprs=rt_device_find("uart3"); //ec20
	g_type = 1;/*0 is m26,1 is ec20, 2 is wire net, 3 is wifi*/
	if (rt_device_open(dev_gprs, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX) == RT_EOK)			
	{
		change_baud(115200);
		rt_event_init(&gprs_event, "gprs_event", RT_IPC_FLAG_FIFO );		
		rt_mutex_init(&gprs_lock, "gprs_lock", RT_IPC_FLAG_FIFO);
		rt_sem_init(&(gprs_rx_sem), "ch2o_rx", 0, 0);
		g_data_queue = (struct rt_data_queue *)rt_malloc(sizeof(struct rt_data_queue)*3);
		rt_data_queue_init(&g_data_queue[0],64,4,RT_NULL);
		rt_data_queue_init(&g_data_queue[1],64,4,RT_NULL);
		rt_data_queue_init(&g_data_queue[2],64,4,RT_NULL);
		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd(GPRS_POWER_RCC,ENABLE);
		RCC_APB2PeriphClockCmd(G4_POWER_RCC,ENABLE);
		RCC_APB2PeriphClockCmd(G4_STATUS_RCC,ENABLE);
		RCC_APB2PeriphClockCmd(G4_DTR_RCC,ENABLE);
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin   = GPRS_POWER_PIN;
		GPIO_Init(GPRS_POWER_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin   = G4_POWER_PIN;
		GPIO_Init(G4_POWER_PORT, &GPIO_InitStructure);
		//GPIO_InitStructure.GPIO_Pin   = G4_DTR_PIN;
		//GPIO_Init(G4_DTR_PORT, &GPIO_InitStructure);
		//GPIO_SetBits(G4_DTR_PORT, G4_DTR_PIN);
		//GPIO_InitStructure.GPIO_Pin   = G4_RESET_PIN;
		//GPIO_Init(G4_RESET_PORT, &GPIO_InitStructure);
		//GPIO_SetBits(G4_RESET_PORT, G4_RESET_PIN);

		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Pin   = G4_STATUS_PIN;
		GPIO_Init(G4_STATUS_PORT, &GPIO_InitStructure);

		strcpy(server_addr[0], DEFAULT_SERVER);
		strcpy(server_port[0], DEFAULT_PORT);
		m26_restart();
		rt_device_set_rx_indicate(dev_gprs, gprs_rx_ind);
		rt_thread_startup(rt_thread_create("thread_gprs",gprs_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("gprs_init",gprs_process, 0,2048, 20, 10));
		rt_thread_startup(rt_thread_create("server",server_process, 0,2048, 20, 10));
	//rt_thread_startup(rt_thread_create("gprs_send",send_process, 0,1024, 20, 10));

	}
	return 0;
}
INIT_APP_EXPORT(gprs_init);
