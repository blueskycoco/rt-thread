#ifndef _ILI9325_
#define _ILI9325_
#define WHITE 0xFFFF
#define BLACK 0X0000
#define BLUE  0x001F  
#define GREEN 0x07E0
#define RED   0xF800
rt_err_t stm32_lcd_init();
void draw_hline(const rt_uint16_t pixel, int x1, int x2, int y);
void draw_vline(const rt_uint16_t pixel, int x, int y1, int y2);
void fb_clr(uint16_t color);
void put_cross(int x, int y);
void clr_cross(int x, int y);
#endif
