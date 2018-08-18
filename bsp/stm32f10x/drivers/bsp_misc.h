#ifndef _MISC_H
#define _MISC_H
unsigned int CRC_check(unsigned char *Data,unsigned short Data_length);
char *add_item(char *old,char *id,char *text);
void gprs_at_cmd(rt_device_t dev, const char *cmd);
rt_bool_t have_str(const char *str, const char *magic);
void Adc_Init();
void show_battery(int v);
void led_blink(int times);
rt_int32_t match_bin(rt_uint8_t *ptr1,int len1, rt_uint8_t *ptr2,rt_size_t len2);
void adjust_time(rt_uint8_t *server_time);
void update_ip_list(rt_uint8_t *ip, rt_uint8_t len);
int get_len(rt_uint8_t *pos, rt_uint16_t len);
void cat_file(unsigned char *file);
unsigned int CRC_check_file(unsigned char *file);
void show_signal(int csq);
rt_uint8_t con_rssi(rt_uint8_t cc_rssi);
void net_flow(void);
void alarm_flow(void);
void show_memory_info(void);
void print_ts(rt_uint8_t *ptr);
#endif
