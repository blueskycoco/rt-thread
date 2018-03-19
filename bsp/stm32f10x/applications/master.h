#ifndef _MASTER_H
#define _MASTER_H

#define INFO_EVENT_PCIE_NULL 				(1<<0)
#define INFO_EVENT_CODING					(1<<1)
#define INFO_EVENT_FACTORY_RESET			(1<<2)
#define INFO_EVENT_NORMAL	 				(1<<3)
#define INFO_EVENT_ALARM	 				(1<<4)
#define INFO_EVENT_PROTECT_ON 				(1<<5)
#define INFO_EVENT_PROTECT_OFF 				(1<<6)
#define INFO_EVENT_STM32_ERROR 				(1<<7)
#define INFO_EVENT_USE_GPRS	 				(1<<8)
#define INFO_EVENT_USE_WIRE	 				(1<<9)
#define INFO_EVENT_WIFI_SIGNAL 				(1<<10)
#define INFO_EVENT_POWER_SWITCH				(1<<11)
#define INFO_EVENT_USE_XG	 				(1<<12)
#define INFO_EVENT_GPRS_SIGNAL 				(1<<13)
#define INFO_EVENT_SHOW_NUM	 				(1<<14)
#define INFO_EVENT_BATTERY_LEVEL			(1<<15)
#define INFO_EVENT_SAVE_FANGQU				(1<<16)
#define INFO_EVENT_SAVE_MAIN				(1<<17)
#define INFO_EVENT_MUTE						(1<<18)
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
	rt_uint8_t 				qccid[10];
};
struct FangQu
{
	rt_uint8_t  	index;			//fq index
	rt_uint8_t  	type;			//wire 1, wireless 2 ro
	
	rt_uint8_t  	operationType; 	//0 now, 1 delay ,2 24 hour
	rt_uint8_t  	voiceType;		// 0 audio , 1 no audio
	rt_uint8_t  	alarmType;		//0,1,2,3,4
	rt_uint8_t  	isBypass;		//0 noBypass ,1 Bypass
	rt_uint8_t		isStay;			//0 nostay ,1 stay protect off
	rt_uint8_t  	status;			//0 unprotect , 1 protect
	rt_uint8_t  	slave_delay;	//0 normal , 1 delay T mins
	
	rt_uint32_t 	slave_sn;		//ro
	rt_uint8_t  	slave_type;		//ro
	rt_uint16_t   	slave_model;	//ro
	rt_uint32_t 	slave_batch;	//ro
};

struct FangQuProperty
{   	
	rt_uint8_t   	delay_in;
	rt_uint8_t   	delya_out;
	rt_uint16_t  	auto_bufang[2];
	rt_uint16_t  	auto_chefang[2];
	rt_uint8_t	  	alarm_voice;	//play alarm time
	rt_uint8_t	  	is_alarm_voice;	//open or not alarm voice
	rt_uint8_t   	is_check_AC;
	rt_uint8_t   	is_check_DC;
	rt_uint8_t	  	PGM[4];
};

#define WIRELESS_MAX	50
#define WIRE_MAX		30
struct FangQu    fangqu_wire[30];
struct FangQu    fangqu_wireless[50];
struct FangQuProperty fqp;
struct MachineProperty mp;
#define LOGING			0x0001
#define HEART_BEAT		0x0002
#define T_LOGOUT		0x0003
#define ALARM_TRAP		0x0004
#define GET_ADDRESS		0x0005
#define LOGIN_ACK 		0x8001
#define HEART_BEAT_ACK	0x8002
#define T_LOGOUT_ACK	0x8003
#define ALARM_TRAP_ACK	0x8004
#define GET_ADDRESS_ACK	0x8005
int load_param();
void save_param(int type);
void info_user(void *param);
rt_uint8_t handle_packet(rt_uint8_t *data);
#endif
