#ifndef _CON_COMMON_H
#define _CON_COMMON_H
#include "con_socket.h"
void set_config(rt_uint8_t *data,rt_int32_t ipv6_len,rt_int32_t dev);
void print_config(config g);
rt_int8_t *send_out(rt_int32_t dev,rt_int32_t cmd,rt_int32_t *lenout);
rt_int32_t common_init(rt_int32_t dev);
rt_bool_t need_reconfig(rt_int32_t dev);
void all_cut();
#endif
