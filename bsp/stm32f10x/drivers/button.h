#ifndef _BUTTON_
#define _BUTTON_
#define BUZZER_OK	0
#define BUZZER_ERROR 1
#define BUZZER_INFO	2
void button_init(void);
rt_uint16_t get_bat(void);
void battery_init();
void buzzer_ctl(int level);
rt_uint8_t check_ac();
void bell_ctl(int level);
#endif
