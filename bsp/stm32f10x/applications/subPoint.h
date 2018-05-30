#ifndef SUB_POINT_
#define SUB_POINT_
void set_fq_on(struct FangQu *list, int len);
void handleSub(rt_uint8_t *data);
void edit_fq(rt_uint8_t index, rt_uint8_t param0,rt_uint8_t param1);
void delete_fq(rt_uint8_t index, rt_uint8_t type);
void proc_fq(rt_uint8_t *fq, rt_uint8_t len, rt_uint8_t code);
void proc_detail_fq(rt_uint8_t index, rt_uint8_t code);
char *cmd_sub_type(rt_uint8_t type);
#endif
