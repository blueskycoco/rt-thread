#include <rtthread.h>
#include "stm32f0xx.h"
#define DEBUG rt_kprintf
 
uint8_t spi_send_rcv(uint8_t data);
int wait_int(int flag);
void cc1101_hw_init();
void reset_cs();
void cc1101_send_packet(uint8_t *txBuffer, uint8_t size);
uint8_t cc1101_rcv_packet(uint8_t *rxBuffer, uint8_t *length);
