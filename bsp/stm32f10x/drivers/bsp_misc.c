#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <rtthread.h>
#include "stm32f10x.h"
#include "bsp_misc.h"
unsigned int CRC_check(unsigned char *Data,unsigned short Data_length)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned int CRC1=0xFFFF;
	while(Data_length)
	{
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
	return CRC1;
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
	else if (csq >= 12) 
		level = 5;	
	else if (csq >= 8)	
		level = 4;	
	else if (csq >= 5)	
		level = 3;	
	else 
		level = 2;
	SetSignalIco(level);
}
void show_battery(int v)
{
	int level = 4;
	if (v>1200)
		level=4;
	else if (v>1170)
		level=3;
	else if (v>1120)
		level=2;
	else if (v>1050)
		level=1;
	else
		level=5;

	SetBatteryIco(level);
}
void Adc_Init(void)
{

	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = ADC_Channel_8;
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
		ad_sum += Get_val(ADC_Channel_8);
		rt_thread_delay(1); 
	} 
	return (ad_sum / 10);
}

