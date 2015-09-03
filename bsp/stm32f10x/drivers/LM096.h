#ifndef __LM096_H
#define __LM096_H

#ifdef __cplusplus
 extern "C" {
#endif
/*ssd1306芯片的初始化*/
void ssd1306_init(void);
//void drawstring(uint8_t x, uint8_t line, char *c);
/*打开oled显示*/
void display(void);
/*清除显示缓冲区*/
void clear(void);
/*绘图电池图标和数字信息，bat1是第一个电池的电量，bat2是第二个电池的电量，从0到100,c是数字串，总共6个数字，一个.*/
void draw(uint8_t bat1,uint8_t bat2,char *c);
#ifdef __cplusplus
}
#endif

#endif
