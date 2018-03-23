#ifndef _PROP_H
#define _PROP_H
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
	rt_uint8_t		status;
	rt_uint8_t   	delay_in;
	rt_uint8_t   	delya_out;
	rt_uint32_t  	auto_bufang;
	rt_uint32_t  	auto_chefang;
	rt_uint8_t	  	alarm_voice_time;	//play alarm time
	rt_uint8_t	  	is_alarm_voice;	//open or not alarm voice
	rt_uint8_t   	is_check_AC;
	rt_uint8_t   	is_check_DC;
	rt_uint8_t	  	is_lamp;
	rt_uint8_t	  	PGM0;
	rt_uint8_t	  	PGM1;
};

#define WIRELESS_MAX	50
#define WIRE_MAX		30
struct FangQu    fangqu_wire[30];
struct FangQu    fangqu_wireless[50];
struct FangQuProperty fqp;
struct MachineProperty mp;
#define TYPE_MP 		0
#define TYPE_FQP 		1

#define TYPE_WIRE				0x00
#define TYPE_WIRELESS			0x01
#define TYPE_NOW				0x00
#define TYPE_DELAY				0x01
#define TYPE_24					0x02
#define TYPE_VOICE_Y			0x00
#define TYPE_VOICE_N			0x01
#define TYPE_ALARM_00			0x00
#define TYPE_ALARM_01			0x01
#define TYPE_ALARM_02			0x02
#define TYPE_ALARM_03			0x03
#define TYPE_ALARM_04			0x04
#define TYPE_BYPASS_Y			0x01
#define TYPE_BYPASS_N			0x00
#define TYPE_STAY_Y				0x01
#define TYPE_STAY_N				0x00
#define TYPE_PROTECT_ON			0x02
#define TYPE_PROTECT_OFF		0x01
#define TYPE_SLAVE_MODE_DELAY	0x01
#define TYPE_SLAVE_MODE_NODELAY	0x00

#define DEFAULT_DELAY_OUT		30
#define DEFAULT_DELAY_IN		30
#define DEFAULT_VOICE_TIME		60
#define DEFAULT_CHECK_AC		1
#define DEFAULT_CHECK_DC		1
#define DEFAULT_ALARM_VOICE		0
#define PGM12_00				0x00
#define PGM12_01				0x01
#define PGM12_02				0x02
#define PGM12_03				0x03
#define PGM12_04				0x04
#define PGM34_00				0x00
#define PGM34_01				0x01
#define PGM34_02				0x02

#define FQP_FILE		"/fqp.dat"
#define MP_FILE			"/mp.dat"
#define DEFAULT_DOMAIN 	"kjfslkjflskdjfj"
void dump_mp(struct MachineProperty v);
void dump_fqp(struct FangQuProperty v1, struct FangQu *v2,struct FangQu *v3);
int load_param();
void save_param(int type);
#endif
