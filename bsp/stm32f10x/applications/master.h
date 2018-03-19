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
#define INFO_EVENT_DELAY_PROTECT_ON 		(1<<19)

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
void info_user(void *param);
rt_uint8_t handle_packet(rt_uint8_t *data);
#endif
