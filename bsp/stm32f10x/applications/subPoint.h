#ifndef SUB_POINT_
#define SUB_POINT_
#include "prop.h"
void set_fq_on(struct FangQu *list, int len);
void handleSub(rt_uint8_t *data);
void edit_fq(rt_uint8_t index, rt_uint8_t param0,rt_uint8_t param1,
		rt_uint8_t param2,rt_uint32_t param3,rt_uint16_t param4,
		rt_uint32_t param5);
void delete_fq(rt_uint8_t index, rt_uint8_t type);
void proc_fq(rt_uint8_t *fq, rt_uint8_t len, rt_uint8_t code);
void proc_detail_fq(rt_uint8_t index, rt_uint8_t code);
char *cmd_sub_type(rt_uint8_t type);
char *cmd_dev_type(rt_uint8_t type);
void handle_protect_on();
void handle_protect_off();
void get_infrar_normal_mode();
char get_addr(rt_uint32_t subId, struct FangQu *list, int len);
void proc_wire_fq(rt_uint8_t *fq, rt_uint8_t len, rt_uint8_t code);
void set_fq_on81(struct FangQu *list, int len);
void set_fq_off81(struct FangQu *list, int len);
#endif
