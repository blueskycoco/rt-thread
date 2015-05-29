/*
 * brife: at88sc_config.h , config at88sc and lowlevel gpio control
 */ 
#include <rtthread.h>
#include <string.h>
#ifndef BOOL
#define BOOL rt_bool_t
#endif
#ifndef TRUE
#define TRUE RT_TRUE
#endif
#ifndef FALSE
#define FALSE RT_FALSE
#endif
#ifndef NULL
#define NULL RT_NULL
#endif
#define AT88SC_ATR		0x00	//Answer To Reset
#define AT88SC_FAB		0x08	//Fab Code
#define AT88SC_MTZ		0x0a	//MTZ
#define AT88SC_CMC		0x0c	//Card Manufacturer Code
#define AT88SC_LHC		0x10	//Lot History Code
#define AT88SC_DCR		0x18	//DCR
#define AT88SC_INN		0x19	//Identfication Number Nc
#define AT88SC_AR0		0x20	//Access regiter0
#define AT88SC_PR0		0x21	//Password register control 0
#define AT88SC_AR1		0x22	//Access regiter1
#define AT88SC_PR1		0x23	//Password register control 1
#define AT88SC_AR2		0x24	//Access regiter2
#define AT88SC_PR2		0x25	//Password register control 2
#define AT88SC_AR3		0x26	//Access regiter3
#define AT88SC_PR3		0x27	//Password register control 3
#define AT88SC_IC		0x40	//Issuer Code
#define AT88SC_AAC0		0x50	//Authification attempt count 0
#define AT88SC_CI0		0x51	//Cryptogram Ci0
#define AT88SC_SK0		0x58	//Session Encryption Key Sk0
#define AT88SC_AAC1		0x60	//Authification attempt count 1
#define AT88SC_CI1		0x61	//Cryptogram Ci1
#define AT88SC_SK1		0x68	//Session Encryption Key Sk1
#define AT88SC_AAC2		0x70	//Authification attempt count 2
#define AT88SC_CI2		0x71	//Cryptogram Ci2
#define AT88SC_SK2		0x78	//Session Encryption Key Sk2
#define AT88SC_AAC3		0x80	//Authification attempt count 3
#define AT88SC_CI3		0x81	//Cryptogram Ci3
#define AT88SC_SK3		0x88	//Session Encryption Key Sk3
#define AT88SC_G0		0x90	//Secert Seed G0
#define AT88SC_G1		0x98	//Secert Seed G1
#define AT88SC_G2		0xa0	//Secert Seed G2
#define AT88SC_G3		0xa8	//Secert Seed G3
#define AT88SC_PACW0	0xB0	//PAC of Write password attempt 0
#define AT88SC_W0		0xB1	//Write Password register 0
#define AT88SC_PACR0	0xB4	//PAC of Read password attempt 0
#define AT88SC_R0		0xB5	//Read Password register 0
#define AT88SC_PACW1	0xB8	//PAC of Write password attempt 1
#define AT88SC_W1		0xB9	//Write Password register 1
#define AT88SC_PACR1	0xBc	//PAC of Read password attempt 1
#define AT88SC_R1		0xBd	//Read Password register 1
#define AT88SC_PACW2	0xC0	//PAC of Write password attempt 2
#define AT88SC_W2		0xC1	//Write Password register 2
#define AT88SC_PACR2	0xC4	//PAC of Read password attempt 2
#define AT88SC_R2		0xC5	//Read Password register 2
#define AT88SC_PACW3	0xC8	//PAC of Write password attempt 3
#define AT88SC_W3		0xC9	//Write Password register 3
#define AT88SC_PACR3	0xCC	//PAC of Read password attempt 3
#define AT88SC_R3		0xCD	//Read Password register 3
#define AT88SC_PACW4	0xD0	//PAC of Write password attempt 4
#define AT88SC_W4		0xD1	//Write Password register 4
#define AT88SC_PACR4	0xD4	//PAC of Read password attempt 4
#define AT88SC_R4		0xD5	//Read Password register 4
#define AT88SC_PACW5	0xD8	//PAC of Write password attempt 5
#define AT88SC_W5		0xD9	//Write Password register 5
#define AT88SC_PACR5	0xDc	//PAC of Read password attempt 5
#define AT88SC_R5		0xDd	//Read Password register 5
#define AT88SC_PACW6	0xE0	//PAC of Write password attempt 6
#define AT88SC_W6		0xE1	//Write Password register 6
#define AT88SC_PACR6	0xE4	//PAC of Read password attempt 6
#define AT88SC_R6		0xE5	//Read Password register 6
#define AT88SC_PACW7	0xE8	//PAC of Write password attempt 7
#define AT88SC_W7		0xE9	//Write Password register 7
#define AT88SC_PACR7	0xEC	//PAC of Read password attempt 7
#define AT88SC_R7		0xED	//Read Password register 7
#define BYTES_MAX   	256
#define DEFAULT_ADDRESS 0x0B
#define debug 1
#if debug
#define AT88DBG	rt_kprintf
#else
#define AT88DBG 
#endif
void i2c_init(void);
void i2c_scl_set(unsigned char level);
void i2c_sda_set(unsigned char level);
unsigned char i2c_sda_get(void);
void i2c_sda_input(void);
void i2c_sda_output(void);
void sleep_ms(unsigned long n);
typedef void (*callback_t)(void);
#define USER_ZONE_SIZE 32
#define BURN 0
#define READ 1
typedef struct {
	unsigned char New_SecureCode[7];
	unsigned char ar[2];
	unsigned char ci[7];
	unsigned char g[8];
	unsigned char pw[7];
	unsigned char id[7];
	unsigned char fuse;
	unsigned char num_ar;
}pe,*ppe;


typedef struct {
	unsigned char *user_zone;
	unsigned char g[8];
	unsigned char pw[3];
	unsigned char use_g;
	unsigned char use_pw;
	unsigned char zone_index;
	int len;
	int addr;
	int page_size;
}ge,*pge;

BOOL auth(pge p,callback_t cb);
BOOL read_userzone(pge p);
