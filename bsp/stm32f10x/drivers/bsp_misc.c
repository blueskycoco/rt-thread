#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <rtthread.h>
#include <rthw.h>
#include <dfs_posix.h>
#include "stm32f10x.h"
#include "bsp_misc.h"
#include "prop.h"
#include "lcd.h"
#include "led.h"
#define FLASH_PAGE_SIZE    ((uint16_t)0x800) 
#define WRITE_START_ADDR   ((uint32_t)0x08000000)
extern rt_uint8_t g_main_state;
extern rt_uint32_t g_server_addr_bak;
extern rt_uint32_t g_server_port_bak;
extern rt_uint8_t battery_insert();
unsigned int CRC_check(unsigned char *Data,unsigned short Data_length)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned int CRC1=0xFFFF;
	while(Data_length)
	{
		CRC1=((unsigned int)(Data[Data_index]))^CRC1;
		for(times=0;times<8;times++)
		{
			mid=CRC1;
			CRC1=CRC1>>1;
			if(mid & 0x0001)
			{
				CRC1=CRC1^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC1;
}
unsigned int CRC_check_file(unsigned char *file)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned char *Data = RT_NULL;
	unsigned short Data_length=0,Data_length1=0;
	unsigned int CRC1=0xFFFF;

	int fd = open(file, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("CRC check file ,open %s failed\n", file);
		return 0;
	}
	Data = (unsigned char *)rt_malloc(1024*sizeof(unsigned char));
	if (Data == RT_NULL) {
		rt_kprintf("CRC check file , malloc failed\r\n");
		show_memory_info();
		return 0;
	}

	do {
		Data_length1 = Data_length = read(fd, Data, 1024);
		//rt_kprintf("Data length %d\r\n", Data_length);
		if (Data_length > 0) {
			Data_index=0;
			while (Data_length) {
				CRC1=Data[Data_index]^CRC1;
				for(times=0;times<8;times++)
				{
					mid=CRC1;
					CRC1=CRC1>>1;
					if(mid & 0x0001)
					{
						CRC1=CRC1^0xA001;
					}
				}
				Data_index++;
				Data_length--;
			}
		}
		else
		{
			rt_kprintf("read %s done %04x\r\n", file,CRC1);
		}

	}while(Data_length1 == 1024);
	close(fd);
	rt_free(Data);
	rt_kprintf("CRC check file crc is %04x\r\n", CRC1);
	return CRC1;
}
void cat_file(unsigned char *file)
{	
	unsigned char *Data = RT_NULL;
	int i;
	unsigned short Data_length=0;
	int fd = open(file, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("CRC check file ,open %s failed\n");
		return ;
	}
	Data = (unsigned char *)rt_malloc(1024*sizeof(unsigned char));
	if (Data == RT_NULL) {
		rt_kprintf("cat file , malloc failed\r\n");
		show_memory_info();
		return ;
	}
	rt_kprintf("%s \r\n",file);
	do {
		Data_length = read(fd, Data, 1024);
		if (Data_length>0)
		{
			for(i=0;i<Data_length;i++)
				rt_kprintf("%02x", Data[i]);
			rt_kprintf("\r\n");
		}		
	}while(Data_length >0);
	rt_free(Data);
	close(fd);
}
char doit_ack(char *text,const char *item_str)
{
	char result=0;cJSON *json,*item_json;

	json=cJSON_Parse(text);
	if (!json) {rt_kprintf("Error ack before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		item_json=cJSON_GetObjectItem(json,item_str);
		if((item_json->type & 255) ==cJSON_True)
		{
			result=1;
		}
		cJSON_Delete(json);
	}
	return result;
}
char *doit(char *text,const char *item_str)
{
	char *out=NULL;cJSON *json,*item_json;

	json=cJSON_Parse(text);
	if (!json) {rt_kprintf("Error it before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		item_json = cJSON_GetObjectItem(json, item_str);
		if (item_json)
		{
			cJSON *data;
			data=cJSON_GetObjectItem(item_json,item_str);
			if(data)
			{
				int nLen = strlen(data->valuestring);
				out=(char *)malloc(nLen+1);
				memset(out,'\0',nLen+1);
				memcpy(out,data->valuestring,nLen);
			}
		}
		cJSON_Delete(json);
	}
	return out;
}
char *doit_data(char *text,char *item_str)
{
	char *out=NULL;cJSON *item_json;

	item_json=cJSON_Parse(text);
	if (!item_json) {rt_kprintf("Error data before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{

		cJSON *data;
		data=cJSON_GetObjectItem(item_json,item_str);
		if(data)
		{
			int nLen = strlen(data->valuestring);
			out=(char *)malloc(nLen+1);
			memset(out,'\0',nLen+1);
			memcpy(out,data->valuestring,nLen);
		}
		cJSON_Delete(item_json);	
	}
	return out;
}

char *add_item(char *old,char *id,char *text)
{
	cJSON *root;
	char *out;
	if(old!=NULL)
		root=cJSON_Parse(old);
	else
		root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, id, cJSON_CreateString(text));
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	if(old)
		rt_free(old);
	return out;
}
char *rm_item(char *old,char *id)
{
	cJSON *root;
	char *out;
	if(old!=NULL)
		root=cJSON_Parse(old);
	else
		root=cJSON_CreateObject();	
	cJSON *data;
	data=cJSON_GetObjectItem(root,id);
	if(data)
	{
		cJSON_DeleteItemFromObject(root, id);
	}
	out=cJSON_PrintUnformatted(root);	
	cJSON_Delete(root);
	if(old)
		rt_free(old);
	return out;
}
char *add_obj(char *old,char *id,char *pad)
{
	cJSON *root,*fmt;
	char *out;
	root=cJSON_Parse(old);
	fmt=cJSON_Parse(pad);
	cJSON_AddItemToObject(root, id, fmt);
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	cJSON_Delete(fmt);
	rt_free(pad);
	return out;
}
void gprs_at_cmd(rt_device_t dev, const char *cmd)
{
	if (!have_str(cmd, "CSQ"))
		rt_kprintf("=> %s",cmd);
	rt_device_write(dev, 0, (void *)cmd, rt_strlen(cmd));
}
rt_bool_t have_str(const char *str, const char *magic)
{
	if (strstr(str, magic) != RT_NULL)
		return RT_TRUE;

	return RT_FALSE;
}
void show_signal(int csq)
{
	int level = 0;
	if (csq <= 2 || csq == 99) 
		level = 0;	
	else if (csq >= 25) 
		level = 5;	
	else if (csq >= 20)	
		level = 4;	
	else if (csq >= 13)	
		level = 3;	
	else if (csq >= 6)	
		level = 2;	
	else 
		level = 1;
	SetSignalIco(level);
}
void show_battery(int v)
{
	static rt_bool_t blink = RT_FALSE;
	int level = 4;	
	if (!battery_insert())
	{
		SetBatteryIco(0);
		return ;
	}
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
	if (v>=1258) {
		//GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		level=4;
	}
	else if (v>1225)
		level=3;
	else if (v>1208)
		level=2;
	else if (v>1169)
		level=1;
	else
		level=5;
	//rt_kprintf("battery %d level %d\r\n", v,level);
	if (level == 5) {
		//if (battery_insert()) {
		if (blink) {
			blink = RT_FALSE;
			SetBatteryIco(level);
		} else {
			blink = RT_TRUE;
			SetBatteryIco(1);
		}
		//} else {
		//	SetBatteryIco(level);
		//}
	} else 
		SetBatteryIco(level);
}
void Adc_Init(void)
{

	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;//ADC_Channel_14;
	ADC_Init(ADC1, &ADC_InitStructure);


	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);

	while(ADC_GetResetCalibrationStatus(ADC1));


	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));
}
//uint16_t g_data1 = 0;
rt_uint16_t Get_val(rt_uint8_t ch)
{
	rt_uint16_t DataValue,Data1Value;
#if 0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_1Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	Data1Value = ADC_GetConversionValue(ADC1); 
	
	//return DataValue; 
#endif
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	DataValue = ADC_GetConversionValue(ADC1); 

	rt_kprintf("DataValue %d\r\n", DataValue);
	//g_data1 += Data1Value;
	//DataValue = (int)((float)DataValue/(float)Data1Value)*12;
	return DataValue; 
} 

rt_uint16_t ADC_Get_aveg(void) 
{ 
	rt_uint32_t ad_sum = 0; 
	rt_uint8_t i; 
	if (!battery_insert())
		return 2000;
	for(i=0;i<10;i++) 
	{ 
		ad_sum += Get_val(ADC_Channel_14);
		rt_thread_delay(1); 
	} 
	rt_kprintf("battery is %d \r\n", (int)(ad_sum/10));
	//rt_kprintf("verfintern is %d \r\n", (int)(g_data1/10));
	//g_data1=0;
	return (ad_sum / 10);
}
void led_blink(int times)
{
	int i=0;
	for (i=0;i<times;i++)
	{
		rt_hw_led_on(0);
		rt_thread_delay(RT_TICK_PER_SECOND);
		rt_hw_led_off(0);
		rt_thread_delay(RT_TICK_PER_SECOND);
		if (g_main_state==2)
			rt_hw_led_on(0);
	}
}
rt_int32_t match_bin(rt_uint8_t *ptr1,int len1, rt_uint8_t *ptr2,rt_size_t len2)
{
	int i,j;
	for (i=0;i<len1;i++)
		if (ptr1[i] == ptr2[0] && ((i+len2) < len1))
		{				
			for (j=1;j<len2;j++)
			{
				if (ptr1[i+j] != ptr2[j])
					break;
			}
			if (j==len2)
				return i;
		}
	return -1;
}
void adjust_time(rt_uint8_t *server_time)
{
	rt_time_t local_time,server_ts;
	struct tm *to;
	time(&local_time);
#if 0
	server_ts = time2ts((server_time[0]<<8)|server_time[1],server_time[2],
			server_time[3],server_time[4],server_time[5],server_time[6]);

#else
	server_ts = (server_time[0] << 24) | (server_time[1] << 16) | (server_time[2] << 8) | (server_time[3] << 0);
#endif
	if (abs(local_time-server_ts) >= 30) {		
		rt_kprintf("local time \t%d \r\nserver time \t%d\r\n", local_time, server_ts);
		rt_device_t device = rt_device_find("rtc");
		if (device == RT_NULL)
		{
			rt_kprintf("can not find rtc device\r\n");
			return ;
		}		
		rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &server_ts);
		time(&local_time);
		rt_kprintf("new time \t%s\r\n",ctime(&local_time));
	}
}
void strip2hex(char *str,struct IPAddress *ip)
{
	int i,j=0,inx=0,count=0;
	char tmpBuf[6]={0};
	for(i=0;i<=strlen(str);i++)
	{ 
		if (str[i] != '.' && str[i] != ':') { 
			memcpy(&tmpBuf[i-inx], &str[i], 1); 
		} 
		else { 
			inx = i + 1; 
			ip->IP[j++] = strtol(tmpBuf, NULL, 10);
			memset(tmpBuf, 0, sizeof(tmpBuf)); 
			count++; 
		}
		if(count==4)
		{
			ip->port = strtol(tmpBuf, NULL, 10); 
		}
	}
	//	rt_kprintf("%d.%d.%d.%d:%04d\n",ip->IP[0],ip->IP[1],ip->IP[2],ip->IP[3],ip->port);
}
void update_ip_list(rt_uint8_t *ip, rt_uint8_t len)
{
	rt_uint8_t i = MAX_IP_LIST-1;
	rt_uint8_t newip[25]={0};
	rt_uint8_t *local_ip = (rt_uint8_t *)rt_malloc(len+1);
	if (local_ip == RT_NULL)
	{
		rt_kprintf("malloc failed local ip %d\r\n",len);
		show_memory_info();
		return;
	}
	local_ip[len]='\0';
	rt_memcpy(local_ip,ip,len);
	for (i=MAX_IP_LIST-1; i>0; i--)
	{
		rt_memcpy(&(mp.socketAddress[i]),&(mp.socketAddress[i-1]),sizeof(struct IPAddress));
	}
	strip2hex(local_ip,&(mp.socketAddress[0]));
	rt_free(local_ip);	
	g_server_addr_bak = (mp.socketAddress[0].IP[0] << 24)|
		(mp.socketAddress[0].IP[1] << 16)|
		(mp.socketAddress[0].IP[2] <<  8)|
		(mp.socketAddress[0].IP[3] <<  0);
	g_server_port_bak = mp.socketAddress[0].port;
}
int get_len(rt_uint8_t *pos, rt_uint16_t len)
{
	int i=0;
	int rlen=0;
	while (pos[i] != '\r' && pos[i+1] != '\n' && i<len)
	{
		rlen = rlen*10 + pos[i] - '0';
		i++;
	}
	return rlen;
}
rt_uint8_t con_rssi(rt_uint8_t cc_rssi)
{	
	rt_uint8_t rssi;
	if (cc_rssi >= 128) {
		rssi = 100+((cc_rssi-256)>>1) - 72;
	} else {
		rssi = 100+(cc_rssi>>1) - 72;
	}
	return rssi;
}
void net_flow(void)
{
	int i;
	for (i=0; i<2; i++) {
		SetStateIco(8,ICO_ON);
		SetStateIco(9,ICO_OFF);
		rt_thread_delay(40);
		SetStateIco(8,ICO_OFF);
		SetStateIco(9,ICO_ON);
		rt_thread_delay(40);
	}
	SetStateIco(8,ICO_OFF);
	SetStateIco(9,ICO_OFF);
}
void alarm_flow(void)
{
	static int flag = 0;
	if (flag) {
		SetStateIco(2,ICO_ON);
		flag = 0;
	} else {
		SetStateIco(2,ICO_OFF);
		flag = 1;
	}
}
void show_memory_info(void)
{
	rt_uint32_t total,used,maxused;
	rt_memory_info(&total,&used,&maxused);
	rt_kprintf("++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	rt_kprintf("total memory: %d\n", total);
	rt_kprintf("used memory : %d\n", used);
	rt_kprintf("maximum allocated memory: %d\n", maxused);	
	rt_kprintf("++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
}
void print_ts(rt_uint8_t *ptr)
{
	rt_time_t cur_time = time(RT_NULL);
	rt_kprintf(" %s %s\r\n",ptr,ctime(&cur_time));
}
rt_uint8_t write_flash(rt_uint32_t start_addr, rt_uint8_t *file, rt_uint32_t len)
{
	rt_uint32_t EraseCounter = 0x00, Address = 0x00;
	rt_uint8_t *Data = NULL;
	rt_uint32_t NbrOfPage = 0x00;
	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
	int fd = open(file, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("write flash ,open %s failed\n", file);
		return 0;
	}

	NbrOfPage = len / FLASH_PAGE_SIZE;
	if ((len % FLASH_PAGE_SIZE) !=0)
		NbrOfPage += 1;

	rt_base_t level = rt_hw_interrupt_disable();
	FLASH_Unlock();	
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
	{
		FLASHStatus = FLASH_ErasePage(start_addr + (FLASH_PAGE_SIZE * EraseCounter));
	}
	if (EraseCounter != NbrOfPage)
		rt_kprintf("erase boot area failed\r\n");
	else {
		rt_kprintf("erase boot area ok\r\n");
		Data = (rt_uint8_t *)rt_malloc(2048*sizeof(rt_uint8_t));
		while (1) {
			rt_uint32_t length = read(fd, Data, 2048);
			rt_uint32_t i =0;
			rt_kprintf("begin to prog %08x\r\n", start_addr+Address);
			if (length > 0) {
				while((Address < len) && (FLASHStatus == FLASH_COMPLETE))
				{
					rt_uint32_t tmp_dat = (Data[i+3]<<24)|(Data[i+2]<<16)|(Data[i+1]<<8)|Data[i+0];
					FLASHStatus = FLASH_ProgramWord(start_addr+Address, tmp_dat);
					Address = Address + 4;
					i+=4;
					if (i>=length)
						break;
				}	
				if (i>=length && length != 2048)
					break;
			} else
				break;
		}
	}
	close(fd);
	free(Data);
	rt_hw_interrupt_enable(level);
	return 1;
}

int parse_at_resp(const char *buf, const char *format, ...)
{
	va_list args;
	int resp_args_num = 0;

	RT_ASSERT(format);
	
	va_start(args, format);

    resp_args_num = vsscanf(buf, format, args);

    va_end(args);

    return resp_args_num;
}
static const uint16_t crc16tab[256]= {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
uint16_t CRC16_check(const char *buf, int len) {
    int counter;
    uint16_t crc = 0;
    for (counter = 0; counter < len; counter++)
		crc = (crc >> 8) ^ crc16tab[(crc ^ *(buf++)) & 0xFF];
    return crc;
}
