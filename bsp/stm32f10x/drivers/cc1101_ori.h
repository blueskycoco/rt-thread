#include <rtthread.h>
#include "stm32f10x.h"
#define DEBUG rt_kprintf
#define CC1101_RCV 1
uint8_t spi_send_rcv(uint8_t data);
int wait_int(int flag);
void cc1101_hw_init();
void reset_cs();
void cc1101_send_packet(uint8_t *txBuffer, uint8_t size);
uint8_t cc1101_rcv_packet(uint8_t *rxBuffer, uint8_t *length);
uint8_t read_cc1101(uint8_t addr,uint8_t *buf,uint8_t len,uint8_t type);
void write_cc1101(uint8_t addr,uint8_t* buf,uint8_t len,uint8_t type);
int cc1101_init();
void cs(int type);
int miso();
