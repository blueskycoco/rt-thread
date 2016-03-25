#ifndef _EPI_H
#define _EPI_H
void epi_w_thread(void* parameter);
int _epi_write(int index, const void *buffer, int size,unsigned char signal);
void bus_speed_test(void *param);
int epi_init(void);
#endif
