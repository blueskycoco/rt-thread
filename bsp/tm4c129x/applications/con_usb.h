#ifndef _USB_H
#define _USB_H
rt_size_t _usb_init();
int _usb_write(int index, void *buffer, int size);
void usb_w_thread(void* parameter);
#endif