#ifndef _MISC_H
#define _MISC_H
unsigned int CRC_check(unsigned char *Data,unsigned short Data_length);
char *add_item(char *old,char *id,char *text);
void gprs_at_cmd(rt_device_t dev, const char *cmd);
rt_bool_t have_str(const char *str, const char *magic);
#endif
