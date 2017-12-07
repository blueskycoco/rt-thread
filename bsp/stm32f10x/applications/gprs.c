#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include "cc1101.h"
#include "string.h"
#define GPRS_POWER_PORT GPIOA
#define GPRS_POWER_PIN	GPIO_Pin_7
#define GPRS_POWER_RCC	RCC_APB2Periph_GPIOA
#define MAGIC_OK	"OK"
struct rt_event gprs_event;
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

#define STR_CPIN_READY		"+CPIN: READY"
#define STR_CGREG_READY		"+CGREG: 0,1"
#define STR_STAT_INIT		"IP INITIAL"
#define STR_STAT_IND		"IP IND"
#define STR_QIMUX_0			"+QIMUX: 0"
#define STR_OK				"OK"
#define STR_ERROR			"ERROR"
#define STR_46000			"46000"
#define STR_46001			"46001"
#define STR_46002			"46002"
#define STR_46003			"46003"
#define STR_46004			"46004"
#define STR_STAT_DEACT_OK 	"DEACT OK"
#define STR_CONNECT_OK		"CONNECT OK"
#define STR_CLOSE_OK		"CLOSE OK"

#define STR_QIRDI			"+QIRDI"
#define DEFAULT_SERVER		"106.3.45.71"
#define DEFAULT_PORT		"60001"
const uint8_t qistat[] 		= "AT+QISTAT\n";
const uint8_t qiclose[] 	= "AT+QICLOSE\n";
const uint8_t qilocip[] 	= "AT+QILOCIP\n";
const uint8_t qilport[] 	= "AT+QILPORT?\n";
const uint8_t e0[] 			= "ATE0\n";
const uint8_t cgreg[] 		= "AT+CGREG?\n";
const uint8_t cimi[] 		= "AT+CIMI\n";
const uint8_t cpin[] 		= "AT+CPIN?\n";
const uint8_t qifgcnt[] 	= "AT+QIFGCNT=0\n";
const uint8_t qisrvc[] 		= "AT+QISRVC=1\n";
const uint8_t qimux[] 		= "AT+QIMUX=0\n";
const uint8_t ask_qimux[] 	= "AT+QIMUX?\n";
const uint8_t qideact[]		= "AT+QIDEACT\n";
const uint8_t qindi[]		= "AT+QINDI=1\n";
const uint8_t qird[]		= {"AT+QIRD=0,1,0,1024\n"};

uint8_t 	  qicsgp[32]	= {0};
uint8_t 	  qiopen[64]	= {0};
const uint8_t qiregapp[]	= "AT+QIREGAPP\n";
const uint8_t qiact[] 		= "AT+QIACT\n";
uint8_t server_addr[5][32] = {0};
uint8_t server_port[5][8] = {0};
uint8_t server_index = 0;
rt_bool_t m26_module_use = RT_TRUE;
uint8_t g_gprs_state = GPRS_STATE_INIT;
rt_device_t dev_gprs;
struct rt_semaphore gprs_rx_sem;
struct rt_data_queue *g_data_queue;
static rt_err_t gprs_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(gprs_rx_sem));    
	return RT_EOK;
}
/*read data from usart , put to dataqueue */
void gprs_rcv(void* parameter)
{	
	uint8_t rcv[64] = {0};
	uint8_t *buf = RT_NULL;
	rt_bool_t first_time = RT_TRUE;
	uint32_t len = 0, total_len = 0;
	while(1)	
	{			
		if (!m26_module_use) 
			return;
		if (first_time) {
			buf = (uint8_t *)rt_malloc(sizeof(uint8_t));
			first_time = RT_FALSE;
			}
		rt_sem_take(&gprs_rx_sem, RT_WAITING_FOREVER);
		while (1) {
			len = rt_device_read(dev_gprs, 0, rcv, 64);
			if (len > 0) {
				uint8_t *new_buf = (uint8_t *)rt_realloc(buf, total_len+len);
				memcpy(new_buf + total_len, rcv,len);
				total_len += len;
				buf = new_buf;
				rt_kprintf("<rlen %d %d %s>\r\n", len,total_len, buf);
			}
			else
				break;
		}
		if (total_len > 0) {			
			rt_kprintf("len %d %s\r\n",total_len,buf);
			buf[total_len] = '\0';
			rt_data_queue_push(g_data_queue, buf, total_len, RT_WAITING_FOREVER);
			total_len = 0;
			first_time = RT_TRUE;
		}
	}
}
#if 0
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &gprs_event, GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}

static rt_size_t gprs_read_data(
		uint8_t *rcv,
        rt_size_t len,
        uint32_t timeout,
        uint8_t *magic)
{
    rt_size_t readlen = 0;
#if 0
    do
    {
        readlen += rt_device_read(dev_gprs,
                0, rcv+readlen, len-readlen);
        if (readlen >= len)
            return readlen;
		else if (magic != RT_NULL)
		{
			if (strstr((const char *)rcv, magic) != RT_NULL)
			return readlen;
			if (strcmp(magic, "CONNECT OK") == 0)
				if (strstr((const char *)rcv, "CONNECT FAIL") != RT_NULL)
				return readlen;
		}
		else {
			if (strstr((const char *)rcv, "OK") != RT_NULL ||
					strstr((const char *)rcv, "ERROR") != RT_NULL)
			return readlen;
		}
    } while (rt_sem_take(&gprs_rx_sem, timeout) == RT_EOK);
#else
	while(1) {
	rt_err_t ret = gprs_wait_event(timeout);
	if (ret != RT_EOK)
		continue;
	if (g_len >= len)
		{
			readlen = g_len;
			g_len = 0;			
            return readlen;
		}
		else if (magic != RT_NULL)
		{
			if (strstr((const char *)g_rcv, magic) != RT_NULL){
			readlen = g_len;
			g_len = 0;
			return readlen;
				}
			if (strcmp(magic, "CONNECT OK") == 0)
				if (strstr((const char *)g_rcv, "CONNECT FAIL") != RT_NULL){
			readlen = g_len;
			g_len = 0;
				return readlen;
					}
		}
		else {
			if (strstr((const char *)g_rcv, "OK") != RT_NULL ||
					strstr((const char *)g_rcv, "ERROR") != RT_NULL){
			readlen = g_len;
			g_len = 0;
			return readlen;
				}
		}
	}
#endif
    return readlen;
}

rt_err_t gprs_cmd(const uint8_t *cmd, uint32_t cmd_len, uint8_t **rcv, uint32_t *rcv_len, uint32_t timeout,uint8_t *magic)
{
	uint32_t read_len;
	
	if (cmd != RT_NULL)	
	{
		read_len = rt_device_write(dev_gprs, 0, (void *)cmd, cmd_len);
		rt_kprintf("%s", cmd);
	}
	
	if (read_len == cmd_len)
	{
		//ret = gprs_wait_event(timeout);
		//memset(g_rcv,0,2048);
		read_len = gprs_read_data(g_rcv, 100, timeout, magic);
	}
	
	if (read_len > 0 && rcv != RT_NULL && rcv_len != RT_NULL) {
		*rcv = (uint8_t *)rt_malloc((read_len+1) * sizeof(uint8_t));
		(*rcv)[read_len] = '\0';
		memcpy(*rcv, g_rcv, read_len);
		*rcv_len = read_len;
		rt_kprintf("<%s>", *rcv);
		rt_memset(g_rcv,0,2048);
		return RT_EOK;
	} else
		rt_kprintf("rcv timeout\r\n");

	return RT_ERROR;
}
#endif
void m26_restart(void)
{
    GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PIN);
    rt_thread_delay(RT_TICK_PER_SECOND);
    GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PIN);
    rt_thread_delay(RT_TICK_PER_SECOND*5);
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
static rt_size_t gprs_read_data(
		uint8_t *rcv,
        rt_size_t len,
        uint32_t timeout)
{
    rt_size_t readlen = 0;
    do
    {
        readlen += rt_device_read(dev_gprs,
                0, rcv+readlen, len-readlen);
        if (readlen >= len)
            return readlen;
		else if (strstr((const char *)rcv, "OK") != RT_NULL)
			return readlen;
		
    } while (rt_sem_take(&gprs_rx_sem, timeout) == RT_EOK);
    return readlen;
}
rt_err_t gprs_cmd(const uint8_t *cmd, uint32_t cmd_len, uint8_t *rcv, uint32_t *rcv_len, uint32_t timeout)
{
	uint32_t write_len;
	uint8_t ch;
	int i=0;
	write_len = rt_device_write(dev_gprs, 0, (void *)cmd, cmd_len);
	rt_kprintf("sending %s", cmd);
	*rcv_len = 0;
	if (write_len == cmd_len)
	{
		while (1) {
			if (rt_device_read(dev_gprs, 0, &ch, 1) == 1)
			{
				if (ch == '\n')
				{
					break;
				}
				else
					rcv[(*rcv_len)++] = ch;
			}
			else
				i++;
			rt_thread_delay(1);
			if (i>20)
				break;				
		}
	}
	if (*rcv_len > 0)
		return RT_EOK;
	return RT_ERROR;
}
int auto_baud(void)
{
	int baud = 115200;
	const uint8_t at[] 	= "AT\n";
	uint8_t rcv[10] = {0};
	uint32_t len;
	while (1) {
		rt_kprintf("trying baud %d\r\n", baud);
    	change_baud(baud);	
    	rt_err_t ret = gprs_cmd(at, rt_strlen(at), rcv, &len, 200);
    	if (ret == RT_EOK && len > 0) {
			if (strstr((const char *)rcv, "OK") != NULL)
			{
				break;
			}
		}
		
		if (baud == 75)
		{
			rt_kprintf("fuck!\r\n");
			break;
		}
		else if (baud == 150)
			baud = 75;
		else if (baud == 300)
			baud = 150;
		else if (baud == 600)
			baud = 300;
		else if (baud == 1200)
			baud = 600;
		else if (baud == 2400)
			baud = 1200;
		else if (baud == 4800)
			baud = 2400;
		else if (baud == 9600)
			baud = 4800;
		else if (baud == 14400)
			baud = 9600;
		else if (baud == 19200)
			baud = 14400;
		else if (baud == 28800)
			baud = 19200;
		else if (baud == 38400)
			baud = 28800;
		else if (baud == 57600)
			baud = 38400;
		else if (baud == 115200)
			baud = 115200;
	}
	rt_kprintf("baud is %d\r\n",baud);
	return baud;
}
#if 0
/*
 * power on m26
 * waiting for gprs register network
 * do gprs dial up
 */
uint8_t m26_startup(void)
{
	uint8_t *rcv = RT_NULL;
	uint32_t len = 0, retry = 0;
	uint8_t apn[10] = {0};
	rt_err_t	ret = RT_EOK;
	const uint8_t e0[] 		= "ATE0\n";
	const uint8_t cgreg[] 	= "AT+CGREG?\n";
	const uint8_t cimi[] 	= "AT+CIMI\n";
	const uint8_t cpin[] 	= "AT+CPIN?\n";
	const uint8_t qifgcnt[] = "AT+QIFGCNT=0\n";
	const uint8_t qisrvc[] 	= "AT+QISRVC=1\n";
	const uint8_t qimux[] 	= "AT+QIMUX=0\n";
	const uint8_t ask_qimux[] 	= "AT+QIMUX?\n";
	uint8_t 	  qicsgp[32]= {0};
	const uint8_t qiregapp[]= "AT+QIREGAPP\n";
	const uint8_t qiact[] 	= "AT+QIACT\n";
	const uint8_t qistat[] 	= "AT+QISTAT\n";
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(GPRS_POWER_RCC,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPRS_POWER_PIN;
    GPIO_Init(GPRS_POWER_PORT, &GPIO_InitStructure);
	
	if (g_gprs_state != GPRS_STATE_INIT)
	{
		rt_kprintf("m26 startup gprs state not init ,return\r\n");
		return 0;
	}
	m26_restart();
	//auto_baud();

    ret = gprs_cmd(e0, rt_strlen(e0), &rcv, &len, 200, RT_NULL);
    if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
    	rt_free(rcv);
	}
	/*check sim card inserted*/
	while (1) {
	ret = gprs_cmd(cpin, rt_strlen(cpin), &rcv, &len, 200, RT_NULL);
    if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
		if (strstr((const char *)rcv, "+CPIN: READY") == RT_NULL)
			rt_kprintf("sim card not inserted.\r\n");
		else {
			rt_free(rcv);
			break;
		}
    	rt_free(rcv);
	}
	rt_thread_delay(100*5);
	}

    ret = gprs_cmd(ask_qimux, rt_strlen(ask_qimux), &rcv, &len, 200, RT_NULL);
    if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
    	rt_free(rcv);
	}
	while (1) {
    	/*waiting for gprs register complete */
    	while (1) {
    		ret = gprs_cmd(cgreg, rt_strlen(cgreg), &rcv, &len, 200, RT_NULL);
    		if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
				if (strstr((const char *)rcv, "+CGREG: 0,1") != NULL) 
				{
					rt_free(rcv);
					rt_kprintf("gprs success to register network\r\n");
					break;
				}
				retry++;
				rt_free(rcv);
				rt_thread_delay(RT_TICK_PER_SECOND);
				if (retry > 300)
				{
					m26_restart();
					retry = 0;
					rt_kprintf("gprs failed to register network\r\n");
				}
			}
		}

		/* config gprs */
		ret = gprs_cmd(cimi, rt_strlen(cimi), &rcv, &len, 200, RT_NULL);
		if (ret == RT_EOK && rcv != RT_NULL)
		{
			//rt_kprintf("len %d => %s", len, rcv);
			if (strstr((const char *)rcv, "46000") != RT_NULL 
				|| strstr((const char *)rcv, "46002") != RT_NULL
				|| strstr((const char *)rcv, "46004") != RT_NULL)
				rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "CMNET");
			else if(strstr((const char *)rcv, "46001") != RT_NULL)
				rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "UNINET");
			else if(strstr((const char *)rcv, "46003") != RT_NULL)
				rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "CTNET");
			//rt_kprintf("%s", qicsgp);
			rt_free(rcv);
			ret = gprs_cmd(qifgcnt, rt_strlen(qifgcnt), &rcv, &len, 200, RT_NULL);
		}
		
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qistat, rt_strlen(qistat), &rcv, &len, 100, "STATE1");
			if (strstr((const char *)rcv, "IP START") == RT_NULL &&
				strstr((const char *)rcv, "PDP DEACT") == RT_NULL)
			{
				rt_kprintf("gprs already dial ok\r\n");
				g_gprs_state = GPRS_STATE_DIAL_UP;
				rt_free(rcv);
				break;
			}

		}
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qicsgp, rt_strlen(qicsgp), &rcv, &len, 200, RT_NULL);
		}

		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qiregapp, rt_strlen(qiregapp), &rcv, &len, 200, RT_NULL);
		}

		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qimux, rt_strlen(qimux), &rcv, &len, 200, RT_NULL);
		}
		
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qisrvc, rt_strlen(qisrvc), &rcv, &len, 200, RT_NULL);
		}
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qiact, rt_strlen(qiact), &rcv, &len, 6000, RT_NULL);
		}
		
		if (ret == RT_EOK) {
			if (rcv) {
				if (strstr((const char *)rcv, "OK") != RT_NULL)
				{
					rt_kprintf("gprs dial ok\r\n");
					g_gprs_state = GPRS_STATE_DIAL_UP;
					rt_free(rcv);
					break;
				}
				else
					m26_restart();
				rt_free(rcv);
			}
		}
	}

	return ret;
}
void handle_connect_fail(void)
{
	uint8_t *rcv = RT_NULL;
	uint32_t len = 0;
	const uint8_t qistat[] 	= "AT+QISTAT\n";
	const uint8_t qiclose[] 	= "AT+QICLOSE\n";
	const uint8_t qideact[] 	= "AT+QIDEACT\n";
	const uint8_t qiact[] 	= "AT+QIACT\n";
	rt_err_t ret = gprs_cmd(qistat, rt_strlen(qistat), &rcv, &len, 100, "STATE1");
	if (ret == RT_EOK && rcv != RT_NULL) {
		if (strstr((const char *)rcv, "TCP CONNECTING") != RT_NULL) {
			gprs_cmd(qiclose, rt_strlen(qiclose), RT_NULL, RT_NULL, 200, RT_NULL);
		} else {
			rt_free(rcv);
			ret = gprs_cmd(qideact, rt_strlen(qideact), &rcv, &len, 6000, RT_NULL);		
			if (ret == RT_EOK && rcv != RT_NULL)
				rt_free(rcv);
			gprs_cmd(qiact, rt_strlen(qiact), &rcv, &len, 6000, RT_NULL);			
		}
		if (rcv)
		rt_free(rcv);
	}
}
uint8_t m26_open_cnn(uint8_t* server_addr, uint8_t* server_port)
{
	uint8_t qiopen[64] = {0};
	uint8_t *rcv = RT_NULL;
	uint32_t len = 0;
	uint8_t can_open = 0;
	rt_err_t ret;
	const uint8_t qistat[] 	= "AT+QISTAT\n";
	const uint8_t qiclose[] 	= "AT+QICLOSE\n";
	const uint8_t qilocip[] 	= "AT+QILOCIP\n";
	const uint8_t qilport[] 	= "AT+QILPORT?\n";
	if (g_gprs_state != GPRS_STATE_DIAL_UP) {
		rt_kprintf("open cnn, gprs state not dial up,return\r\n");
		return 0;
	}
	#if 0
	rt_err_t ret = gprs_cmd(qistat, rt_strlen(qistat), &rcv, &len, 100, "STATE1");
	if (ret == RT_EOK && rcv != RT_NULL) {
		//rt_kprintf("gprs status %s\r\n", rcv);
		if (strstr((const char *)rcv, "IP INITIAL") != RT_NULL ||
			strstr((const char *)rcv, "IP STATUS") != RT_NULL ||
			strstr((const char *)rcv, "IP GPRSACT") != RT_NULL ||
			strstr((const char *)rcv, "IP CLOSE") != RT_NULL)
			can_open = 1;
		else if(strstr((const char *)rcv, "TCP CONNECTING") != RT_NULL) {
			rt_free(rcv);
			gprs_cmd(qiclose, rt_strlen(qiclose), RT_NULL, RT_NULL, 200, RT_NULL);
		}
		rt_free(rcv);
	}
	#endif
	//if (can_open) {
	while (1) {
	rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\n",
			server_addr,server_port);
	ret = gprs_cmd(qiopen, rt_strlen(qiopen), &rcv, &len, 300, RT_NULL);
	if (ret == RT_EOK && rcv != RT_NULL) {
		if (strstr((const char *)rcv, "OK") != RT_NULL) {
			rt_free(rcv);
			ret = gprs_cmd(RT_NULL, 0, &rcv, &len, 10000, "CONNECT OK");
			if (ret == RT_EOK && rcv != RT_NULL) {				
				if (strstr((const char *)rcv, "CONNECT OK") != RT_NULL) {
					g_gprs_state = GPRS_STATE_OPEN_CNN;
					ret = gprs_cmd(qilocip, rt_strlen(qilocip), &rcv, &len, 100, "STATE1");
					if (ret == RT_EOK && rcv != RT_NULL) {
						rt_free(rcv);
					}
					ret = gprs_cmd(qilport, rt_strlen(qilport), &rcv, &len, 100, "STATE1");
					if (ret == RT_EOK && rcv != RT_NULL) {
						rt_free(rcv);
					}
					break;
				} else {
					/*error handle*/
					handle_connect_fail();
				}
			} 
		}
		else if (strstr((const char *)rcv, "ALREADY CONNECT") != RT_NULL) {
					rt_free(rcv);
			gprs_cmd(qiclose, rt_strlen(qiclose), RT_NULL, RT_NULL, 200, RT_NULL);
			}
		else {
			handle_connect_fail();
		}
	}
		}
	//}
}
uint8_t m26_send(const uint8_t *data, uint32_t send_len)
{
	uint8_t qisend[32] 	= {0};//"AT+QISEND=\n";
	const uint8_t qisack[32] 	= "AT+QISACK\n";
	uint8_t *rcv;
	uint32_t len;
	rt_sprintf(qisend, "AT+QISEND=%d\n", send_len);
	rt_err_t ret = gprs_cmd(qisend, rt_strlen(qisend), &rcv, &len, 300, ">");
	if (ret == RT_EOK && rcv != RT_NULL) {
		if (strstr((const char *)rcv,">") != RT_NULL) {
			rt_device_write(dev_gprs, 0, data, send_len);
			rt_free(rcv);
			ret = gprs_cmd(RT_NULL, 0, &rcv, &len, 300, RT_NULL);
			if (ret == RT_EOK && rcv != RT_NULL) {
				if (strstr((const char *)rcv, "SEND OK") != RT_NULL) {
					rt_free(rcv);
					ret = gprs_cmd(qisack, rt_strlen(qisack), &rcv, &len, 100, RT_NULL);
					if (ret == RT_EOK && rcv != RT_NULL)
						rt_free(rcv);
				}
			}
		}
	}

	return 0;
}
#endif
rt_bool_t have_str(const char *str, const char *magic)
{
	if (strstr(str, magic) != RT_NULL)
		return RT_TRUE;

	return RT_FALSE;
}
void gprs_at_cmd(const char *cmd)
{
	rt_kprintf("%s",cmd);
	rt_device_write(dev_gprs, 0, (void *)cmd, rt_strlen(cmd));
}
void gprs_process(void* parameter)
{
#if 0
	const char test_string[] = "hello m26 client upload";
	char test[64] = {0};
	int i = 0;
	m26_startup();
	m26_open_cnn("106.3.45.71", "60001");
	while (1) {
		rt_memset(test,0,64);
		rt_sprintf(test,"%s %d", test_string,i);
		m26_send(test, rt_strlen(test));
		i++;
		rt_thread_delay(100);
		if (i>10)
			break;
	}
#endif
	uint8_t cpin_count = 0;
	rt_size_t data_size;
	const void *last_data_ptr;
	gprs_at_cmd(e0);
	while (1) {
	//	if (rt_data_queue_peak(g_data_queue, &last_data_ptr, &data_size) != RT_EOK)
	//		continue;
		rt_err_t r = rt_data_queue_pop(g_data_queue, &last_data_ptr, &data_size, RT_WAITING_FOREVER);

		if (r == RT_EOK && last_data_ptr != RT_NULL) {
			rt_kprintf("<%d %s>",data_size, last_data_ptr);
			switch (g_gprs_state) {
				case GPRS_STATE_INIT:
						g_gprs_state = GPRS_STATE_CHECK_CPIN;
						gprs_at_cmd(cpin);
						break;
				case GPRS_STATE_CHECK_CPIN:
						if (have_str(last_data_ptr,STR_CPIN_READY)) {
							g_gprs_state = GPRS_STATE_CHECK_CGREG;
							gprs_at_cmd(cgreg);
						} else {
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
						if (have_str(last_data_ptr, STR_CGREG_READY)) {
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							gprs_at_cmd(qistat);
						} else {
							rt_thread_delay(100);
							gprs_at_cmd(cgreg);
						}
						break;
				case GPRS_STATE_CHECK_QISTAT:
						if (have_str(last_data_ptr, STR_STAT_INIT)) {
							g_gprs_state = GPRS_STATE_SET_QIMUX;
							gprs_at_cmd(qimux);
						} else if (have_str(last_data_ptr, STR_STAT_IND)){
							g_gprs_state = GPRS_STATE_SET_QIDEACT;
							gprs_at_cmd(qideact);
						} else if (have_str(last_data_ptr, STR_CONNECT_OK)){
							g_gprs_state = GPRS_STATE_SET_QICLOSE;
							gprs_at_cmd(qiclose);
						}
						break;
				case GPRS_STATE_SET_QICLOSE:
						if (have_str(last_data_ptr, STR_CLOSE_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIOPEN;
							rt_memset(qiopen, 0, 64);
							rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\n",
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
						} else {
							gprs_at_cmd(qindi); 						
						}
						break;								
				case GPRS_STATE_SET_QIMUX:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QINDI;
							gprs_at_cmd(qindi);
						} else {
							gprs_at_cmd(qimux);							
						}
						break;		
				case GPRS_STATE_CHECK_CIMI:
						rt_memset(qicsgp, 0, 32);
						if (have_str(last_data_ptr, STR_46000) ||
							have_str(last_data_ptr, STR_46002) ||
							have_str(last_data_ptr, STR_46004)) {
							rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "CMNET");
						} else if (have_str(last_data_ptr, STR_46001)){
							rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "UNINET");
						} else if (have_str(last_data_ptr, STR_46003)){
							rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "CTNET");
						} else
							rt_sprintf(qicsgp, "AT+QICSGP=1,\"%s\"\n", "CMNET");
							g_gprs_state = GPRS_STATE_SET_QICSGP;
							gprs_at_cmd(qicsgp);
						break;						
				case GPRS_STATE_SET_QICSGP:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIREGAPP;
							gprs_at_cmd(qiregapp);
						} else {
							gprs_at_cmd(qicsgp); 						
						}
						break;		
				case GPRS_STATE_SET_QIREGAPP:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QISRVC;
							gprs_at_cmd(qisrvc);
						} else {
							gprs_at_cmd(qiregapp); 						
						}
						break;
				case GPRS_STATE_SET_QISRVC:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIACT;
							gprs_at_cmd(qiact);
						} else {
							gprs_at_cmd(qisrvc); 						
						}
						break;		
				case GPRS_STATE_SET_QIACT:
						if (have_str(last_data_ptr, STR_OK)) {
							g_gprs_state = GPRS_STATE_SET_QIOPEN;
							rt_memset(qiopen, 0, 64);
							rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\n",
										server_addr[server_index],server_port[server_index]);
							gprs_at_cmd(qiopen);
						} else {
							/*check error condition*/
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							gprs_at_cmd(qistat);
						}
						break;
				case GPRS_STATE_SET_QIDEACT:
						if (have_str(last_data_ptr, STR_STAT_DEACT_OK)) {
							g_gprs_state = GPRS_STATE_CHECK_QISTAT;
							gprs_at_cmd(qistat);
						} else if (have_str(last_data_ptr, STR_ERROR)){
							g_gprs_state = GPRS_STATE_SET_QIDEACT;
							gprs_at_cmd(qideact);
						}
						break;						
				case GPRS_STATE_SET_QIOPEN:
						if (have_str(last_data_ptr, STR_CONNECT_OK)) {
							g_gprs_state = GPRS_STATE_DATA_PROCESSING;
							/*send data here */
							rt_kprintf("connect to server ok\r\n");
						} else {
							//gprs_at_cmd(qiopen);
						}
						break;
				case GPRS_STATE_DATA_PROCESSING:
						if (have_str(last_data_ptr, STR_QIRDI)) {
							/*server data in */
							g_gprs_state = GPRS_STATE_DATA_READ;
							gprs_at_cmd(qird);
						} /*else if (have_str(last_data_ptr, STR_SERVER_DATA_IN)){

						}*/
						break;
				case GPRS_STATE_DATA_READ:
						if (have_str(last_data_ptr, STR_OK)) {
							rt_kprintf("rcving full\r\n");
							g_gprs_state = GPRS_STATE_DATA_PROCESSING;
						}
						else
						{
							rt_kprintf("not recving full\r\n");
						}
			}
			rt_free((void *)last_data_ptr);
		}
	}

}
int gprs_init(void)
{
	/*handle m26*/
	rt_thread_delay(1000);
	dev_gprs=rt_device_find("uart3");
	if (rt_device_open(dev_gprs, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX) == RT_EOK)			
	{
		change_baud(115200);
		rt_event_init(&gprs_event, "gprs_event", RT_IPC_FLAG_FIFO );
		rt_sem_init(&(gprs_rx_sem), "ch2o_rx", 0, 0);
		rt_device_set_rx_indicate(dev_gprs, gprs_rx_ind);
		g_data_queue = (struct rt_data_queue *)rt_malloc(sizeof(struct rt_data_queue)*1);
		rt_data_queue_init(g_data_queue,64,4,RT_NULL);
    	GPIO_InitTypeDef GPIO_InitStructure;

    	RCC_APB2PeriphClockCmd(GPRS_POWER_RCC,ENABLE);
    	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    	GPIO_InitStructure.GPIO_Pin   = GPRS_POWER_PIN;
    	GPIO_Init(GPRS_POWER_PORT, &GPIO_InitStructure);

		strcpy(server_addr[0], DEFAULT_SERVER);
		strcpy(server_port[0], DEFAULT_PORT);
		m26_restart();
		auto_baud();
		rt_thread_startup(rt_thread_create("thread_gprs",gprs_rcv, 0,1524, 20, 10));
		rt_thread_startup(rt_thread_create("gprs_init",gprs_process, 0,2048, 20, 10));
		//rt_thread_delay(100);
		//gprs_at_cmd(e0);
	}
	return 0;
}
INIT_APP_EXPORT(gprs_init);
