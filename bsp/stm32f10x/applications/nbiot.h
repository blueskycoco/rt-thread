#ifndef _NBIOT_
#define _NBIOT_
void nbiot_start(int index);
void nbiot_proc(void *last_data_ptr, rt_size_t data_size);
rt_uint8_t handle_nb_packet(rt_uint8_t *data, uint32_t len);
void send_nb(int type);
#endif
