#ifndef __LM096_H
#define __LM096_H

#ifdef __cplusplus
 extern "C" {
#endif
/*ssd1306оƬ�ĳ�ʼ��*/
void ssd1306_init(void);
//void drawstring(uint8_t x, uint8_t line, char *c);
/*��oled��ʾ*/
void display(void);
/*�����ʾ������*/
void clear(void);
/*��ͼ���ͼ���������Ϣ��bat1�ǵ�һ����صĵ�����bat2�ǵڶ�����صĵ�������0��100,c�����ִ����ܹ�6�����֣�һ��.*/
void draw(char *co2,char *co1);

#ifdef __cplusplus
}
#endif

#endif
