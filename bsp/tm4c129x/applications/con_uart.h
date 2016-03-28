#ifndef _UART_H
#define _UART_H
void uart_w_thread(void* parameter);
rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size);
#endif
