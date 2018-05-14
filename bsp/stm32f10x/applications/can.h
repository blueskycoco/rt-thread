#ifndef _CAN_H
#define _CAN_H
void can_init();
int can_send(unsigned short id, unsigned char *payload, unsigned char payload_len);
int can_read(unsigned char *buf, unsigned char *buf_len);
void set_id(unsigned short id);
int poll_can();
#endif
