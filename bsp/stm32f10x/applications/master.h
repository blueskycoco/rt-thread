#ifndef _MASTER_H
#define _MASTER_H

#define INFO_EVENT_PCIE_NULL 				(1<<0)
#define INFO_EVENT_CODING					(1<<1)
#define INFO_EVENT_FACTORY_RESET			(1<<2)
#define INFO_EVENT_NORMAL	 				(1<<3)

struct IPAddress
{
	rt_uint8_t IP[4];
	rt_uint16_t port;
};

struct DomainAddress
{
	rt_uint8_t domain[32];
	rt_uint16_t port;
};

struct ReadOnlyProperty
{
	rt_uint8_t sn[6];
	rt_uint32_t model;
	rt_uint8_t CAPTCHA[6];	
};

struct EventLog
{
	rt_uint32_t address;
	rt_uint16_t length;
};

struct MachineProperty
{
   	struct ReadOnlyProperty 	roProperty;
	struct IPAddress			socketAddress[10];
	struct DomainAddress   	socketDomainAddress;
	struct IPAddress			updateAddress;
	struct DomainAddress   	updateDomainAddress;
	rt_uint16_t 			socketAddressVersion;
	rt_uint16_t 			socketDomainVersion;
	rt_uint16_t 			updateAddressVersion;
	rt_uint16_t 			updateDomainVersion;
	rt_uint16_t 			appVersion;
	rt_uint16_t 			appCRC;
	rt_uint16_t 			appLength;
	rt_uint16_t 			firmVersion;
	rt_uint16_t 			firmCRC;
	rt_uint16_t 			firmLength;
	rt_uint8_t  			status;
};
struct FangQu
{
	rt_uint8_t  	index;
	rt_uint8_t  	type;
	rt_uint8_t  	operationType;
	rt_uint8_t  	voiceType;
	rt_uint8_t  	alarmType;
	rt_uint8_t  	isBypass;
	rt_uint8_t  	status;
	rt_uint32_t 	slave_sn;
	rt_uint8_t  	slave_type;
	rt_uint32_t   		slave_model;
	rt_uint32_t 	slave_batch;
};

struct FangQuProperty
{   	
	rt_uint8_t   delay_in;
	rt_uint8_t   delya_out;
	rt_uint8_t   delay_bufang;
	rt_uint16_t  auto_bufang[3];
	rt_uint16_t  auto_chefang[3];
	rt_uint8_t	  alarm_voice;
	rt_uint8_t	  is_alarm_voice;
	rt_uint8_t   is_check_AC;
	rt_uint8_t   is_check_DC;
	rt_uint8_t	  PGM;
	rt_uint8_t	  is_lamp;
};
struct FangQu    fangquList[140];
struct FangQuProperty fqp;
struct MachineProperty mp;
int load_param();
void save_param(int type);
void info_user(void *param);
#endif
