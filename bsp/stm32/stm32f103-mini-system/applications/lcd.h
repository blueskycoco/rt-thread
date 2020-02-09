#ifndef _LCD_H
#define _LCD_H
rt_uint8_t st7585_init(void);
void ST7585_Write_String(rt_uint8_t X,rt_uint8_t Y,rt_uint8_t *S);
#endif
