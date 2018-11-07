#include <rtthread.h>
#include <stdint.h>
#include "spi1.h"
#include "cc1101_def.h"
#include "cc1101.h"
#define RF_XTAL 26000
#define SCALING_FREQ     (float)((RF_XTAL)/65.536)
#define SCALING_FREQEST  (unsigned long)((RF_XTAL)/16.384)
unsigned char volatile timer_event;
unsigned long volatile time_counter = 0;
#define MODE_RX                  0x00  
#define MODE_TX                  0x01  
#define TX_BUF_SIZE              128  
#define RX_BUF_SIZE              256  
#define MAX_FIFO_SIZE            0x38 
struct rt_semaphore cc1101_rx_sem;
extern rt_uint8_t r_signal;
rt_uint8_t g_retry=0;
unsigned char paTable[1];           
unsigned char rf_end_packet = 0;
rt_uint32_t cc1101_crash_cnt = 0;
struct rf_dev  
{  
    unsigned char tx_buf[TX_BUF_SIZE];  
    unsigned short tx_rd;  
    unsigned short tx_wr;  
    unsigned char rx_buf[RX_BUF_SIZE];  
    unsigned short rx_rd;  
    unsigned short rx_wr;  
    unsigned short mode;  
};  
static struct rf_dev rf_dev;  
#define cc1101_hex_printf(buf, count) \
{\
    int i;\
    int flag=0; \
	for(i = 0; i < count; i++)\
	{\
		if (buf[i] < 32 || buf[i] > 126) \
			flag =1;\
	}\
    for(i = 0; i < count; i++)\
    {\
    	if (!flag) \
        	rt_kprintf("%c", buf[i]);\
        else \
			rt_kprintf("%02x ", buf[i]);\
    }\
}
#if 0
#define VAL_MDMCFG3	0x83
#define VAL_MDMCFG4 0xf5
#define MAX_PAYLOAD	32
#define MRFI_RSSI_VALID_DELAY_US    13
void MRFI_RSSI_VALID_WAIT()                                                
{                                                                             
  int16_t delay = MRFI_RSSI_VALID_DELAY_US;                                   
  unsigned char status;															
  do                                                                          
  {	
  	trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_SINGLE_ACCESS, PKTSTATUS, &status, 1); 
    if(status & (0x50))  
    {                                                                         
      break;                                                                  
    }                                                                         
    rt_thread_delay(1); /* sleep */                                           
    delay -= 640;                                                              
  }while(delay > 0);                                                          
}        

void MRFI_STROBE_IDLE_AND_WAIT()                   
{													  
  trxSpiCmdStrobe( RF_SIDLE );						  
  while (trxSpiCmdStrobe( RF_SNOP ) & 0xF0) ; 		  
}

static uint8_t mrfiRndSeed=0;
static uint16_t sReplyDelayScalar=0;
static uint16_t sBackoffHelper=0;

#define   MRFI_RADIO_OSC_FREQ         	26000000
#define   PLATFORM_FACTOR_CONSTANT    	2
#define   PHY_PREAMBLE_SYNC_BYTES     	8
#define   MRFI_BACKOFF_PERIOD_USECS 	250
#define   MRFI_RSSI_OFFSET	74
#define MRFI_RANDOM_OFFSET                   67
#define MRFI_RANDOM_MULTIPLIER              109

int8_t Mrfi_CalculateRssi(uint8_t rawValue)
{
  int16_t rssi;

  /* The raw value is in 2's complement and in half db steps. Convert it to
   * decimal taking into account the offset value.
   */
  if(rawValue >= 128)
  {
    rssi = (int16_t)(rawValue - 256)/2 - MRFI_RSSI_OFFSET;
  }
  else
  {
    rssi = (rawValue/2) - MRFI_RSSI_OFFSET;
  }

  /* Restrict this value to least value can be held in an 8 bit signed int */
  if(rssi < -128)
  {
    rssi = -128;
  }

  return rssi;
}

void create_seed()
{
	unsigned char rssi;
	uint32_t dataRate, bits;
	uint16_t exponent, mantissa;
	trxSpiCmdStrobe(RF_SRX);
	MRFI_RSSI_VALID_WAIT();	
	for(uint8_t i=0; i<16; i++)
	{
		trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_SINGLE_ACCESS, RSSI, &rssi, 1);
		mrfiRndSeed = (mrfiRndSeed << 1) | (rssi & 0x01);
		Mrfi_CalculateRssi(rssi);
	}

	mrfiRndSeed |= 0x0080;
	//RF_GDO_PxIE  &= ~RF_GDO_PIN;
	trxRfDisableInt();
	MRFI_STROBE_IDLE_AND_WAIT();
	trxSpiCmdStrobe( RF_SFRX );
	//RF_GDO_PxIFG &= ~RF_GDO_PIN;
	clearIntFlag();
	mantissa = 256 + VAL_MDMCFG3;

	exponent = 28 - (VAL_MDMCFG4 & 0x0F);

	dataRate = mantissa * (MRFI_RADIO_OSC_FREQ>>exponent);

	bits = ((uint32_t)((PHY_PREAMBLE_SYNC_BYTES + MAX_PAYLOAD)*8))*10000;

	sReplyDelayScalar = PLATFORM_FACTOR_CONSTANT + (((bits/dataRate)+5)/10);
	sBackoffHelper = MRFI_BACKOFF_PERIOD_USECS + (sReplyDelayScalar>>5)*1000;
	//RF_GDO_PxIE  |= RF_GDO_PIN;
	trxRfEnableInt();
	rt_kprintf("s %d\r\n", sBackoffHelper);
}
uint8_t MRFI_RandomByte(void)
{
  mrfiRndSeed = (mrfiRndSeed*MRFI_RANDOM_MULTIPLIER) + MRFI_RANDOM_OFFSET;

  return mrfiRndSeed;
}

static void Mrfi_RandomBackoffDelay(void)
{
  uint8_t backoffs;
  uint8_t i;

  /* calculate random value for backoffs - 1 to 16 */
  backoffs = (MRFI_RandomByte() & 0x0F) + 1;

  /* delay for randomly computed number of backoff periods */
  for (i=0; i<backoffs*sBackoffHelper; i++)
  {
    rt_thread_delay( 1 );
  }
}
void Mrfi_RxModeOff(void)
{
  //RF_GDO_PxIE	&= ~RF_GDO_PIN;
	trxRfDisableInt();
  MRFI_STROBE_IDLE_AND_WAIT();

  trxSpiCmdStrobe( RF_SFRX );

  //RF_GDO_PxIFG	&= ~RF_GDO_PIN;
  clearIntFlag();
}
static void Mrfi_RxModeOn(void)
{
  //RF_GDO_PxIFG	&= ~RF_GDO_PIN;
	clearIntFlag();
  trxSpiCmdStrobe( RF_SRX );

 //RF_GDO_PxIE	|= RF_GDO_PIN;
 trxRfEnableInt();
}
void cca()
{
	uint8_t ccaRetries = 4;
	uint8_t papd = 0x1b;
	uint8_t sync = 0x06;
	trx8BitRegAccess(RADIO_WRITE_ACCESS, IOCFG0, &papd, 1);
	for (;;)
	{
		trxSpiCmdStrobe( RF_SRX );

		MRFI_RSSI_VALID_WAIT();

		//RF_GDO_PxIFG  &= ~RF_GDO_PIN;
		clearIntFlag();
		trxSpiCmdStrobe( RF_STX );
		rt_kprintf("go here\r\n");

		//__delay_cycles(750);
		rt_thread_delay(3);
		if (/*RF_GDO_PxIFG & RF_GDO_PIN*/getIntFlag())
		{
			//RF_GDO_PxIFG  &= ~RF_GDO_PIN;
			clearIntFlag();
			rt_kprintf("before here\r\n");
			while (!gdo_level());
			//RF_GDO_PxIFG  &= ~RF_GDO_PIN;
			rt_kprintf("break here\r\n");
			break;
		}
		else
		{
			MRFI_STROBE_IDLE_AND_WAIT();

			trxSpiCmdStrobe( RF_SFRX );

			if (ccaRetries != 0)
			{
				//Mrfi_RandomBackoffDelay();
				//__delay_cycles(25);
				rt_thread_delay(1);
				ccaRetries--;
			}
			else 
			{
				rt_kprintf("timeout\r\n");
				break;
			}
		} 
	} 
	trxSpiCmdStrobe( RF_SFTX );
	trx8BitRegAccess(RADIO_WRITE_ACCESS, IOCFG0, &sync, 1);
	Mrfi_RxModeOn();
	rt_kprintf("return \r\n");
}

#endif

const registerSetting_t preferredSettings_1200bps[]=
{
	{IOCFG0,	0x06},
	{PKTCTRL0,	0x05},
	{FSCTRL1,	0x06},
	{ADDR,		0x01},
	{PKTCTRL1, 	0x05},
	#if 0
    {FREQ2,		0x10},
	{FREQ1,		0xa7},
	{FREQ0,		0x62},
	#else
	{FREQ2,		0x10},//0x11},
	{FREQ1,		0xaa},//0xec},
	{FREQ0,		0x1e},//0x4e},
	#endif
	{MDMCFG4,	0xf5},
	{MDMCFG3,	0x83},
	{MDMCFG2,	0x13},
	{DEVIATN,	0x15},
	{AGCCTRL2,	0x07},
	{AGCCTRL1,	0x40},
//	{MCSM1, 	0x3f},
	{MCSM0,		0x18},
	{FOCCFG,	0x16},
	{WORCTRL,	0xFB},
	{FSCAL3,	0xE9},
	{FSCAL2,	0x2A},
	{FSCAL1,	0x00},
	{FSCAL0,	0x1F},
	{TEST0,		0x09},
	{PATABLE,	0xCB} 
};
static void cc1101_set_rx_mode(void)  
{  

    trxSpiCmdStrobe(RF_SFRX);  
    trxSpiCmdStrobe(RF_SIDLE);  
    trxSpiCmdStrobe(RF_SRX);  
  
    rf_dev.mode = MODE_RX;  
} 
void __delay_cycles(uint32_t count)
{
	volatile long i,j,k;
	for (i=0;i<count;i++)
		for (j=0;j<10;j++)
			k=j;
}

#define MRFI_RSSI_VALID_DELAY_US    1300
void MRFI_RSSI_VALID_WAIT()                                                
{                                                                             
  uint16_t delay = MRFI_RSSI_VALID_DELAY_US;                                   
  unsigned char status;	
  rt_kprintf("st 1\r\n");
  do                                                                    
  {	
  	trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_BURST_ACCESS, PKTSTATUS, &status, 1); 
    if(status & (0x50))
    {
      break;                                                                  
    }
    __delay_cycles(64);
	trxSpiCmdStrobe( RF_SRX );
   // rt_thread_delay(1);
	if (delay>64)
    	delay -= 64; 
	else
		break;
  }while (delay>0);
	rt_kprintf("st %x %d\r\n", status,delay);
}        

static void cc1101_set_tx_mode(void)  
{    
	#if 1
	//rt_kprintf("st 0\r\n");
    //trxSpiCmdStrobe(RF_SIDLE);  
	MRFI_RSSI_VALID_WAIT();
    trxSpiCmdStrobe(RF_STX);  
	  #else
	  cca();
	  #endif
    rf_dev.mode = MODE_TX; 
}  
static int cc1101_receive_packet(unsigned char *buf, unsigned char *count)  
{ 
	int i;
    unsigned char packet_len, status[2];
  	trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_SINGLE_ACCESS, RXBYTES, &packet_len, 1);
	//rt_kprintf("packet len is %x\r\n",packet_len);
    if((packet_len & 0x7f) == 0 || (packet_len & 0x80) != 0)    
    {  
        return -1;  
    }  
  
    //packet_len = cc1101_read_signle_reg(RF_RXFIFO);  
    trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_SINGLE_ACCESS, RXFIFO, &packet_len, 1);
	//rt_kprintf("packet len is %d\r\n",packet_len);
    if(packet_len <= *count)  
    {  
        //cc1101_read_burst_reg(RF_RXFIFO, buf, packet_len);            
        //cc1101_read_burst_reg(RF_RXFIFO, status, 2);  
		trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_BURST_ACCESS, RXFIFO, buf, packet_len);
		*count = packet_len;
		trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_BURST_ACCESS, RXFIFO, status, 2);
		//rt_kprintf("status %x %x \r\n",status[0],status[1]);
		//trx8BitRegAccess(RADIO_READ_ACCESS | RADIO_BURST_ACCESS, RSSI, &status[0], 1);
		//trx8BitRegAccess(RADIO_READ_ACCESS | RADIO_BURST_ACCESS, LQI, &status[1], 1);
		//rt_kprintf("status %x %x \r\n",status[0],status[1]);
        //*count = packet_len;  
        r_signal = status[0];
        return ((status[1] & 0x80) ? 0 : -2);  
    }  
    else   
    {  
        cc1101_set_rx_mode();  
           
        return -3;  
    }       
       
}  
static int cc1101_send_packet(unsigned char *buf, unsigned char count)  
{     
    rt_kprintf("cc1101 send data %d:", count);  
    cc1101_hex_printf(buf, count);  
    rt_kprintf("\r\n");  
//	Mrfi_RxModeOff();
    //cc1101_write_signle_reg(RF_TXFIFO, count);  
    //cc1101_write_burst_reg(RF_RXFIFO, buf, count);  
    trx8BitRegAccess(RADIO_WRITE_ACCESS|RADIO_SINGLE_ACCESS, TXFIFO, &count,1);
  	trx8BitRegAccess(RADIO_WRITE_ACCESS|RADIO_BURST_ACCESS, TXFIFO, buf, count);
    return 0;    
} 
void wait_cc1101_sem()
{
		rt_sem_take(&cc1101_rx_sem, RT_WAITING_FOREVER);
}
static void cc1101_gdo0_rx_it(void)  
{  
    unsigned char rx_buf[MAX_FIFO_SIZE], rx_count;  
    int ret, i;  
         
    rx_count = MAX_FIFO_SIZE;  
  
    ret = cc1101_receive_packet(rx_buf, &rx_count);  

	cc1101_set_rx_mode();  
      
    if(ret == 0)  
    {  
        for(i = 0; i < rx_count; i++)  
        {  
            rf_dev.rx_buf[rf_dev.rx_wr++] = rx_buf[i];  
            rf_dev.rx_wr %= RX_BUF_SIZE;  
  
            /* 
                overflow handle 
            */  
            if(rf_dev.rx_wr == rf_dev.rx_rd)  
            {  
                rf_dev.rx_rd++;  
                rf_dev.rx_rd %= RX_BUF_SIZE;  
            }  
        }   
		cc1101_crash_cnt=0;
        rt_kprintf("cc1101 receive data<%d>:",rx_count);  
        //cc1101_hex_printf(rx_buf, rx_count);  
		rt_kprintf("\r\n");  
		rt_sem_release(&(cc1101_rx_sem));
    }  
}
static unsigned short cc1101_get_tx_buf_count(void)  
{  
    unsigned short count;  
    if(rf_dev.tx_rd < rf_dev.tx_wr)  
    {  
        count = rf_dev.tx_wr - rf_dev.tx_rd;  
    }  
    else if(rf_dev.tx_rd == rf_dev.tx_wr)   
    {  
        count = 0;  
    }  
    else   
    {  
        count = TX_BUF_SIZE - rf_dev.tx_rd + rf_dev.tx_wr;  
    }  
    return count;  
}  
static int cc1101_read_tx_buf(void *_buf, int count)  
{  
    unsigned char *buf = (unsigned char *)_buf;  
    int i, real_count;  
    for(i = 0, real_count = 0; i < count; i++)  
    {  
        if(rf_dev.tx_rd == rf_dev.tx_wr)  
        {  
            return real_count;  
        }  
        else    
        {  
            buf[i] = rf_dev.tx_buf[rf_dev.tx_rd++];  
            rf_dev.tx_rd %= TX_BUF_SIZE;  
            real_count++;  
        }  
    }  
    return real_count;   
}  
static void cc1101_write_tx_buf(void *_buf, int count)  
{  
    unsigned char *buf = (unsigned char *)_buf;  
    int i;  
    for(i = 0; i < count; i++)  
    {  
        while(((rf_dev.tx_wr+1) % TX_BUF_SIZE) == rf_dev.tx_rd)
			rt_kprintf("waiting here\r\n");  
        rf_dev.tx_buf[rf_dev.tx_wr++] = buf[i];  
        rf_dev.tx_wr %= TX_BUF_SIZE;  
    }  
}  
static void cc1101_gdo0_tx_it(void)  
{  
    short wait_tx_count;  
    unsigned char buf_tmp[TX_BUF_SIZE], status;  
    rt_uint32_t loop = 0;
    wait_tx_count = cc1101_get_tx_buf_count();  
    status = trxSpiCmdStrobe(RF_SFTX);  
  
    /* 
        wait device to IDLE status, very important !!!! 
        ohterwise, transmit loss packet. 
        if device into IDLE, prove transmit end. 
    */  
   // rt_time_t cur_time = time(RT_NULL);
    rt_kprintf("tx 1count \r\n");
    while(status & 0x70)  
    {  
        status = trxSpiCmdStrobe(RF_SNOP);
		loop++;
		if (loop > 10)
			break;
    }  
	//cur_time = time(RT_NULL);
	rt_kprintf("tx 2count %d %d\r\n", wait_tx_count,loop);
    if(wait_tx_count == 0)  
    {               
        cc1101_set_rx_mode();  
		//if (!gdo_level()) {
		//	rt_kprintf("continue intr\r\n");
		//} else
		//	rt_kprintf("alone intr\r\n");
    }  
    else if(wait_tx_count < MAX_FIFO_SIZE)  
    {     
        cc1101_read_tx_buf(buf_tmp, wait_tx_count);    
        cc1101_send_packet(buf_tmp, wait_tx_count);  
        cc1101_set_tx_mode();  
    }  
    else if(wait_tx_count >= MAX_FIFO_SIZE)  
    {  
        cc1101_read_tx_buf(buf_tmp, MAX_FIFO_SIZE);  
        cc1101_send_packet(buf_tmp, MAX_FIFO_SIZE);  
        cc1101_set_tx_mode();  
    }  
}  
static void cc1101_send(void*_buf, unsigned short count)  
{  
    unsigned char *buf = (unsigned char *)_buf;  
    unsigned char buf_tmp[TX_BUF_SIZE];  
  rt_kprintf("cc1101 send 0 %d\r\n",count);
    if(count == 0 || count > TX_BUF_SIZE)  
    {  
        return;  
    }  
  
    /* 
      if device is transmit mode, write data to transmit buffer,  
      and interrupt auto transmit data. 
      don't write data to device fifo directly. 
       
      if device is receive mode, tx buf free size is TX_BUF_SIZE 
    */  
  rt_kprintf("cc1101 send 1\r\n");
    cc1101_write_tx_buf(buf, count);  
  rt_kprintf("cc1101 send 2\r\n");
    if(rf_dev.mode == MODE_RX)  
    {  
    	rt_kprintf("cc1101 send 3\r\n");
        if(count < MAX_FIFO_SIZE)  
        {  
            cc1101_read_tx_buf(buf_tmp, count);  
            cc1101_send_packet(buf_tmp, count);  
			rt_kprintf("cc1101 send 4\r\n");
        }  
        else if(count >= MAX_FIFO_SIZE)  
        {  
            cc1101_read_tx_buf(buf_tmp, MAX_FIFO_SIZE);  
            cc1101_send_packet(buf_tmp, MAX_FIFO_SIZE);  
			rt_kprintf("cc1101 send 5\r\n");
        }  
  		rt_kprintf("cc1101 send 6\r\n");
        cc1101_set_tx_mode();  
    }  
	rt_kprintf("cc1101 send 7\r\n");
}  
void cc1101_send_write(void*_buf, unsigned short count)  
{  
    int n1, n2, i;  
    unsigned char *buf = (unsigned char *)_buf;  
  
    n1 = count / TX_BUF_SIZE;   
    n2 = count % TX_BUF_SIZE;  
  
    for(i = 0; i < n1; i++)  
    {  
        cc1101_send(buf, TX_BUF_SIZE);  
        buf += TX_BUF_SIZE;  
    }  
  
    cc1101_send(buf, n2);  
}  
int cc1101_receive_read(unsigned char *buf, int count)  
{   
    int i, real_read_count;  
    for(i = 0, real_read_count = 0; i < count; i++)  
    {  
        if(rf_dev.rx_rd == rf_dev.rx_wr)  
        {          	
    		//rt_kprintf("poll triger\r\n");
        	//if (real_read_count == 0)
			//	cc1101_gdo0_rx_it();
			/*g_retry++;
			if (g_retry >= 10) 
			{
				radio_intit2();
				g_retry=0;
			}*/
            return real_read_count;  
        }  
        else  
        {  
        	g_retry=0;
            buf[i] = rf_dev.rx_buf[rf_dev.rx_rd++];  
            rf_dev.rx_rd %= RX_BUF_SIZE;  
            real_read_count++;  
        }  
    }  
    return real_read_count;  
}  
void cc1101_isr(void)  
{  
	#if 1
    if(rf_dev.mode  == MODE_RX)  
    {  
    	rt_kprintf("got cc1101 data begin\r\n");
		cc1101_crash_cnt++;
        cc1101_gdo0_rx_it();  
    }  
    else if(rf_dev.mode == MODE_TX)     
    {  
    	rt_kprintf("send cc1101 data end\r\n");
        cc1101_gdo0_tx_it();  
    }  
    else  
    {  
        rf_dev.mode  = MODE_RX;  
    }
	#else
		cc1101_gdo0_rx_it();  
		cc1101_gdo0_tx_it();  
	#endif
}  
unsigned char set_rf_packet_length(unsigned char length) {
  unsigned char reg_value;

  /* make sure we are in fixed packet mode */
  reg_value = 0x04;
  trx8BitRegAccess(RADIO_WRITE_ACCESS, PKTCTRL0, &reg_value, 1);

  /* set the fixed packet length */
  trx8BitRegAccess(RADIO_WRITE_ACCESS, PKTLEN, &length, 1);

  return (0);
}
int radio_get_rssi(void) {
  int rssi;
  unsigned char cc_rssi;
  
  trx8BitRegAccess(RADIO_READ_ACCESS | RADIO_BURST_ACCESS, RSSI, &cc_rssi, 1);
  
  if (cc_rssi >= 128) {
    rssi = ((cc_rssi-256)>>1) - 72;
  } else {
    rssi = (cc_rssi>>1) - 72;
  }
  return rssi; 
}

char get_device_id(void) {
  unsigned char ret_partnum;
  unsigned char ret_version;
  
  trx8BitRegAccess(RADIO_READ_ACCESS+RADIO_BURST_ACCESS, VERSION, &ret_version, 1);
  trx8BitRegAccess(RADIO_READ_ACCESS+RADIO_BURST_ACCESS, PARTNUM, &ret_partnum, 1);
  rt_kprintf("version %d, partnum %d\r\n", ret_version,ret_partnum);
  switch (ret_partnum) {
  case 0:
    if(ret_version == 0x04) {
      return DEV_CC1101;
    } 
	if(ret_version == 0x14) {
		return DEV_CC1101;
	}
    if(ret_version == 0x07) {
      return DEV_CC1101;
    } 
    if(ret_version == 0x06) {
      return DEV_CC430x;
    }    
    if(ret_version == 0x00) {
      return DEV_CC1100;
    }
    break;
  case 128:
    if(ret_version == 0x03) {
      return DEV_CC2500;
    }
    break;
  default:
    break;
  }
  
  return DEV_UNKNOWN;
}

int radio_init(void)
{

	unsigned char i, writeByte, preferredSettings_length;
	registerSetting_t *preferredSettings;
	rt_sem_init(&(cc1101_rx_sem), "c11_rx", 0, 0);

	trxRfSpiInterfaceInit();	

	trxSpiCmdStrobe(RF_SRES);
	rt_kprintf("get here\r\n");
	if (DEV_CC1101 != get_device_id())
		return 0;
	rt_kprintf("rssi %d\r\n",radio_get_rssi());
	rt_thread_delay(100);
	preferredSettings_length = sizeof(preferredSettings_1200bps)/sizeof(registerSetting_t);	
	preferredSettings = (registerSetting_t *)preferredSettings_1200bps;	
	for(i = 0; i < preferredSettings_length; i++) {	
		writeByte = preferredSettings[i].data;
		trx8BitRegAccess(RADIO_WRITE_ACCESS, preferredSettings[i].addr, &writeByte, 1);	
	}

	for(i = 0; i < preferredSettings_length; i++) {
		uint8 readByte = 0;
		trx8BitRegAccess(RADIO_READ_ACCESS, preferredSettings[i].addr, &readByte, 1);
		if (readByte != preferredSettings[i].data)
		{
			rt_kprintf("rf reg set failed %d %x %x\r\n",i, preferredSettings[i].addr, readByte);
			return 0;
		}
	}
	trxRfSpiInterruptInit();
	cc1101_set_rx_mode();
	return 1;
}
void radio_intit2(void)
{
	unsigned char i, writeByte, preferredSettings_length;
	registerSetting_t *preferredSettings;
	trxRfDisableInt();
	trxRfSpiInterfaceInit2();
	trxSpiCmdStrobe(RF_SRES);
	rt_kprintf("get here2\r\n");
	if (DEV_CC1101 != get_device_id())
		return ;
	rt_kprintf("rssi2 %d\r\n",radio_get_rssi());
	rt_thread_delay(100);
	preferredSettings_length = sizeof(preferredSettings_1200bps)/sizeof(registerSetting_t);	
	preferredSettings = (registerSetting_t *)preferredSettings_1200bps;	
	for(i = 0; i < preferredSettings_length; i++) {	
		writeByte = preferredSettings[i].data;
		trx8BitRegAccess(RADIO_WRITE_ACCESS, preferredSettings[i].addr, &writeByte, 1);	
	}

	for(i = 0; i < preferredSettings_length; i++) {
		uint8 readByte = 0;
		trx8BitRegAccess(RADIO_READ_ACCESS, preferredSettings[i].addr, &readByte, 1);
		if (readByte != preferredSettings[i].data)
		{
			rt_kprintf("rf reg set failed %d %x %x\r\n",i, preferredSettings[i].addr, readByte);
			return;
		}
	}
	trxRfSpiInterruptInit();
	cc1101_set_rx_mode();
}
int radio_receive_on(void) {

	/* Range extender in RX mode */
#ifdef ENABLE_RANGE_EXTENDER
	range_extender_rxon();
#endif
	trxSpiCmdStrobe(RF_SFRX); 
	trxSpiCmdStrobe(RF_SIDLE); 
	trxSpiCmdStrobe(RF_SRX);                 // Change state to TX, initiating

	return(0);
}
int radio_send(unsigned char *payload, unsigned short payload_len) {
	//rt_kprintf("send %s, len %d\r\n",payload, payload_len);
	trx8BitRegAccess(RADIO_WRITE_ACCESS|RADIO_BURST_ACCESS, TXFIFO, payload, payload_len);
	/* Range extender in TX mode */
#ifdef ENABLE_RANGE_EXTENDER
	range_extender_txon();
#endif

	trxSpiCmdStrobe(RF_STX);               // Change state to TX, initiating
//	radio_wait_for_idle(0);
	
	//wait_int(1);
	//wait_int(0);
	radio_receive_on();
	return(0);
}
int radio_read(unsigned char *buf, unsigned short *buf_len) {
	unsigned char status;
	unsigned char pktLen;
	wait_int(1);
	wait_int(0);

	/* Read number of bytes in RX FIFO */
	trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_BURST_ACCESS, RXBYTES, &pktLen, 1);
	//rt_kprintf("pktLen %d\r\n", pktLen);
	pktLen = pktLen  & NUM_RXBYTES;
	/* make sure the packet size is appropriate, that is 1 -> buffer_size */
	if ((pktLen > 0) && (pktLen <= *buf_len)) {

		/* retrieve the FIFO content */
		trx8BitRegAccess(RADIO_READ_ACCESS|RADIO_BURST_ACCESS, RXFIFO, buf, pktLen);

		/* return the actual length of the FIFO */
		*buf_len = pktLen;

		/* retrieve the CRC status information */
		trx8BitRegAccess(RADIO_READ_ACCESS+RADIO_BURST_ACCESS, PKTSTATUS, &status, 1);

	} else {

		/* if the length returned by the transciever does not make sense, flush it */
		*buf_len = 0;                                // 0
		status = 0;
		//trxSpiCmdStrobe(RF_SFRX);	                 // Flush RXFIFO
	}  
	
	radio_receive_on(); 

	/* return status information, CRC OK or NOT OK */
	return (status & CRC_OK);
}
int radio_wait_for_idle(unsigned short max_hold) {

	unsigned int status = 0;
	unsigned char reg_status;

	/* check that we are still in RX mode before entering wait for RX end */
	trx8BitRegAccess(RADIO_READ_ACCESS+RADIO_BURST_ACCESS, MARCSTATE, &reg_status, 1);

	/* filter out only the status section of the register values */
	reg_status  = (reg_status & 0x1F);

	/* check for not idle mode */
	if(!(reg_status == MARCSTATE_IDLE)) {
#if 0
		rf_end_packet = 0;  // initialize global variable for use in this function

		RF_GDO_PxIES |= RF_GDO_PIN;       // Int on falling edge (end of pkt)
		RF_GDO_PxIFG &= ~RF_GDO_PIN;      // Clear flag


		/* wait for idle */
		if(max_hold > 0) {
			status = hal_timer_wait(max_hold);    // this will timeout either with GDO or timer
			if (status == 0) rf_end_packet = 1;
		} else {
			// wait for radio to interupt us to continue processing
			status = 0;

			while(!(RF_GDO_PxIFG & RF_GDO_PIN));
			// indicate that end of packet has been found
			rf_end_packet = 1;
			// clear the interrupt flag
			RF_GDO_PxIFG &= ~RF_GDO_PIN;
		}
#else
	wait_int(1);
	//wait_int(0);
#endif

	}
	/* Get timer values, however if we did not get a packet in time use 0      */
	if(rf_end_packet == 0) {
		status = max_hold;
	}

#ifdef ENABLE_RANGE_EXTENDER
	range_extender_idle();
#endif

	return status;
}
int radio_set_freq(unsigned long freq) {

	unsigned long freq_word;
	unsigned char freq_byte[3];
	float freq_float;

	// calculate the frequency word

	freq_float = freq*1000;
	freq_word = (unsigned long) (freq_float * 1/(float)SCALING_FREQ);
	
	/* return the frequency word */
	freq_byte[2] = ((uint8*)&freq_word)[0];
	freq_byte[1] = ((uint8*)&freq_word)[1];
	freq_byte[0] = ((uint8*)&freq_word)[2];
	rt_kprintf("set to %x %x %x\r\n",freq_byte[2],freq_byte[1],freq_byte[0]);

	// upload the frequency word to the transciver using a 3 byte write
	//trx8BitRegAccess(RADIO_WRITE_ACCESS | RADIO_BURST_ACCESS , FREQ2, freq_byte, 3);
	trx8BitRegAccess(RADIO_WRITE_ACCESS, FREQ2, &(freq_byte[2]), 1);
	trx8BitRegAccess(RADIO_WRITE_ACCESS, FREQ1, &(freq_byte[1]), 1);
	trx8BitRegAccess(RADIO_WRITE_ACCESS, FREQ0, &(freq_byte[0]), 1);

	return 0;
}
int radio_idle(void) {

	/* Idle range extender */
#ifdef ENABLE_RANGE_EXTENDER
	range_extender_idle();
#endif

	/* Idle range extender */
	trxSpiCmdStrobe(RF_SIDLE);

	/* Flush the FIFO's */
	trxSpiCmdStrobe(RF_SFRX);
	trxSpiCmdStrobe(RF_SFTX);
	return(0);
}

