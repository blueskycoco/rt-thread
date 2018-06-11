#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <rtthread.h>
#include "stm32f10x.h"
#include "bsp_misc.h"
#include "prop.h"
#include "lcd.h"
extern rt_uint8_t g_main_state;
extern rt_uint32_t g_server_addr_bak;
extern rt_uint32_t g_server_port_bak;
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
		rt_kprintf("CRC check file ,open %s failed\n");
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
			rt_kprintf("read %s failed %d\r\n", file,CRC1);
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
	if (v>1180)
		level=4;
	else if (v>1070)
		level=3;
	else if (v>960)
		level=2;
	else if (v>850)
		level=1;
	else
		level=5;
	//rt_kprintf("battery %d level %d\r\n", v,level);
	if (level == 5) {
		if (blink) {
			blink = RT_FALSE;
			SetBatteryIco(level);
		} else {
			blink = RT_TRUE;
			SetBatteryIco(1);
		}
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
	ADC_InitStructure.ADC_NbrOfChannel = ADC_Channel_14;
	ADC_Init(ADC1, &ADC_InitStructure);


	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);

	while(ADC_GetResetCalibrationStatus(ADC1));


	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));
}

rt_uint16_t Get_val(rt_uint8_t ch)
{
	rt_uint16_t DataValue;
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	DataValue = ADC_GetConversionValue(ADC1); 
	return DataValue; 
} 

rt_uint16_t ADC_Get_aveg(void) 
{ 
	rt_uint32_t ad_sum = 0; 
	rt_uint8_t i; 
	for(i=0;i<10;i++) 
	{ 
		ad_sum += Get_val(ADC_Channel_14);
		rt_thread_delay(1); 
	} 
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
		rt_kprintf("malloc failed local ip\r\n");
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
