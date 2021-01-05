#ifndef _MEM_LIST_H
#define _MEM_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_D2H	0x01
#define TYPE_H2D	0x00
void rt_memlist_init();
rt_bool_t insert_mem(rt_uint8_t type, rt_uint8_t* data, rt_uint16_t len);
void remove_mem(rt_uint8_t type, rt_uint8_t **out, rt_uint16_t *len);
rt_bool_t host2device_list_isempty();
rt_bool_t device2host_list_isempty();
void notify_d2h();
void notify_h2d();

#ifdef __cplusplus
}
#endif

#endif
