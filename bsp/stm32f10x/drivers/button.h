#ifndef _BUTTON_
#define _BUTTON_
void button_init(void);
rt_uint16_t get_bat(void);
void battery_init();
void buzzer_ctl(int level);

void bell_ctl(int level);
#endif
