#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "led.h"
#include "cc1101.h"
#include "string.h"
#define GPRS_POWER_PORT GPIOA
#define GPRS_POWER_PIN	GPIO_Pin_7
#define GPRS_POWER_RCC	RCC_APB2Periph_GPIOA
struct rt_event gprs_event;
#define GPRS_EVENT_0 (1<<0)
#define GPRS_STATE_INIT		0
#define GPRS_STATE_DIAL_UP	1
#define GPRS_STATE_OPEN_CNN	2
uint8_t g_gprs_state = GPRS_STATE_INIT;
rt_device_t dev_gprs;
uint8_t g_rcv[2048] = {0};
uint32_t g_len = 0;
struct rt_semaphore gprs_rx_sem;
static rt_err_t gprs_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(gprs_rx_sem));    
	return RT_EOK;
}
void gprs_rcv(void* parameter)
{	
	int len = 0;
	while(1)	
	{		
		if (rt_sem_take(&(gprs_rx_sem), RT_WAITING_FOREVER) != RT_EOK) continue;		
		do {
			len=rt_device_read(dev_gprs, 0, g_rcv+g_len, 2048-g_len);		
			g_len += len;
		} while (len > 0);
		if(g_len>0)	
		{
			//rt_kprintf(">>%s", g_rcv);
			rt_event_send(&gprs_event,GPRS_EVENT_0);
		}		
	}	
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
    } while (rt_sem_take(&gprs_rx_sem, timeout) == RT_EOK);

    return readlen;
}
rt_err_t gprs_wait_event(int timeout)
{
	rt_uint32_t ev;
	return rt_event_recv( &gprs_event, GPRS_EVENT_0, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, &ev ); 
}

rt_err_t gprs_cmd(const uint8_t *cmd, uint32_t cmd_len, uint8_t **rcv, uint32_t *rcv_len, uint32_t timeout)
{
	uint32_t read_len;
	
	if (cmd != RT_NULL)	
	{
		read_len = rt_device_write(dev_gprs, 0, (void *)cmd, cmd_len);
		rt_kprintf("<%d><%d> sending %s", read_len,cmd_len,cmd);
	}
	
	if (read_len == cmd_len)
	{
		//ret = gprs_wait_event(timeout);
		memset(g_rcv,0,2048);
		read_len = gprs_read_data(g_rcv, 100, timeout);
		//rt_kprintf("read len %d \r\n", read_len);
	}
	
	//rt_kprintf("ret %d %d\r\n", ret,g_len);
	//if (ret == RT_EOK && g_len > 0 && rcv != RT_NULL && rcv != RT_NULL && rcv_len != RT_NULL) {
	if (read_len > 0) {
		*rcv = (uint8_t *)rt_malloc((read_len+1) * sizeof(uint8_t));
		(*rcv)[read_len] = '\0';
		memcpy(*rcv, g_rcv, read_len);
		*rcv_len = read_len;
		rt_kprintf("%s", *rcv);
		return RT_EOK;
	}

	return RT_ERROR;
}
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
int auto_baud(void)
{
	int baud = 75;
	uint8_t *rcv = RT_NULL;
	const uint8_t at[] 	= "AT\n";
	uint32_t len;
	while (1) {
		rt_kprintf("trying baud %d\r\n", baud);
    	change_baud(baud);	
    	rt_err_t ret = gprs_cmd(at, rt_strlen(at), &rcv, &len, 200);
    	if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
			if (strstr((const char *)rcv, "OK") != NULL)
			{
				rt_free(rcv);
				break;
			}
			rt_free(rcv);
		}
		
		if (baud == 75)
			baud = 150;
		else if (baud == 150)
			baud = 300;
		else if (baud == 300)
			baud = 600;
		else if (baud == 600)
			baud = 1200;
		else if (baud == 1200)
			baud = 2400;
		else if (baud == 2400)
			baud = 4800;
		else if (baud == 4800)
			baud = 9600;
		else if (baud == 9600)
			baud = 14400;
		else if (baud == 14400)
			baud = 19200;
		else if (baud == 19200)
			baud = 28800;
		else if (baud == 28800)
			baud = 38400;
		else if (baud == 38400)
			baud = 57600;
		else if (baud == 57600)
			baud = 115200;
		else if (baud == 115200)
		{
			rt_kprintf("fuck!\r\n");
			break;
		}
	}
	rt_kprintf("baud is %d\r\n",baud);
	return baud;
}
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
	const uint8_t qifgcnt[] = "AT+QIFGCNT=0\n";
	const uint8_t qisrvc[] = "AT+QISRVC=1\n";
	const uint8_t qimux[] = "AT+QIMUX=0\n";
	uint8_t qicsgp[32]= {0};//"AT+QICSGP=1,\"CMNET\"\n";
	const uint8_t qiregapp[]= "AT+QIREGAPP\n";
	const uint8_t qiact[] 	= "AT+QIACT\n";
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
	auto_baud();

    ret = gprs_cmd(e0, rt_strlen(e0), &rcv, &len, 200);
    if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
    	rt_free(rcv);
	}
	while (1) {
    	/*waiting for gprs register complete */
    	while (1) {
    		ret = gprs_cmd(cgreg, rt_strlen(cgreg), &rcv, &len, 200);
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
		ret = gprs_cmd(cimi, rt_strlen(cimi), &rcv, &len, 200);
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
			ret = gprs_cmd(qifgcnt, rt_strlen(qifgcnt), &rcv, &len, 200);
		}
		
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qicsgp, rt_strlen(qicsgp), &rcv, &len, 200);
		}

		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qiregapp, rt_strlen(qiregapp), &rcv, &len, 200);
		}

		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qimux, rt_strlen(qimux), &rcv, &len, 200);
		}
		
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qisrvc, rt_strlen(qisrvc), &rcv, &len, 200);
		}
		if (ret == RT_EOK) {
			if (rcv)
				rt_free(rcv);
			ret = gprs_cmd(qiact, rt_strlen(qiact), &rcv, &len, 6000);
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
void handle_open_error(void)
{

}
uint8_t m26_open_cnn(uint8_t* server_addr, uint8_t* server_port)
{
	uint8_t qiopen[64] = {0};
	uint8_t *rcv = RT_NULL;
	uint32_t len = 0;
	uint8_t can_open = 0;
	const uint8_t qistat[] 	= "AT+QISTAT\n";
	const uint8_t qiclose[] 	= "AT+QICLOSE\n";
	if (g_gprs_state != GPRS_STATE_DIAL_UP) {
		rt_kprintf("open cnn, gprs state not dial up,return\r\n");
		return 0;
	}
	rt_err_t ret = gprs_cmd(qistat, rt_strlen(qistat), &rcv, &len, 200);
	if (ret == RT_EOK && rcv != RT_NULL) {
		rt_kprintf("gprs status %s\r\n", rcv);
		if (strstr((const char *)rcv, "IP INITIAL") != RT_NULL ||
			strstr((const char *)rcv, "IP STATUS") != RT_NULL ||
			strstr((const char *)rcv, "IP CLOSE") != RT_NULL)
			can_open = 1;
		else if(strstr((const char *)rcv, "TCP CONNECTING") != RT_NULL) {
			rt_free(rcv);
			gprs_cmd(qiclose, rt_strlen(qiclose), RT_NULL, RT_NULL, 200);
		}
		rt_free(rcv);
	}

	if (can_open) {
	rt_sprintf(qiopen, "AT+QIOPEN=\"TCP\",\"%s\",\"%s\"\n",
			server_addr,server_port);
	ret = gprs_cmd(qiopen, rt_strlen(qiopen), &rcv, &len, 300);
	if (ret == RT_EOK && rcv != RT_NULL) {
		if (strstr((const char *)rcv, "OK") != RT_NULL) {
			rt_free(rcv);
			ret = gprs_cmd(RT_NULL, 0, &rcv, &len, 10000);
			if (ret == RT_EOK && rcv != RT_NULL) {				
				if (strstr((const char *)rcv, "CONNECT OK") != RT_NULL) {
					g_gprs_state = GPRS_STATE_OPEN_CNN;
				} else {
					/*error handle*/
				}
			}
		} else {
			/* error handle*/
		}
	}
	else
		rt_kprintf("connect to %s:%s timeout\r\n",server_addr,server_port);
	}
}
int gprs_init(void)
{
	/*handle m26*/
	dev_gprs=rt_device_find("uart3");
	if (rt_device_open(dev_gprs, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) == RT_EOK)			
	{
		change_baud(9600);
		rt_event_init(&gprs_event, "gprs_event", RT_IPC_FLAG_FIFO );
		rt_sem_init(&(gprs_rx_sem), "ch2o_rx", 0, 0);
		rt_device_set_rx_indicate(dev_gprs, gprs_rx_ind);
		//rt_thread_startup(rt_thread_create("thread_gprs",gprs_rcv, 0,512, 20, 10));
    	uint8_t *rcv;
    	uint32_t len;
		const uint8_t cimi[] 	= "AT+CIMI\n";
    	rt_err_t ret = gprs_cmd(cimi, rt_strlen(cimi), &rcv, &len, 200);
    	if (ret == RT_EOK && len > 0 && rcv != RT_NULL) {
    		rt_kprintf("rcv %s\r\n", rcv);
    		rt_free(rcv);
		}
		m26_startup();
		m26_open_cnn("106.3.45.71", "60000");
	}
	return 0;
}
INIT_APP_EXPORT(gprs_init);
