#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#define COL8_BLACK  (0)
#define COL8_RED  (1)
#define COL8_GREEN  (2)
#define COL8_YELLOW  (3)
#define COL8_BLUE  (4)
#define COL8_PURPLE  (5)
#define COL8_CYAN  (6)
#define COL8_WHITE  (7)
#define COL8_GRAY  (8)
#define COL8_DARK_RED  (9)
#define COL8_DARK_GREEN  (10)
#define COL8_DARK_YELLOW  (11)
#define COL8_DARK_BLUE  (12)
#define COL8_DARK_PURPLE  (13)
#define COL8_DARK_CYAN  (14)
#define COL8_DARK_GRAY  (15)

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xsize, unsigned char c,
              int x0, int y0, int x1, int y1);
void line8(unsigned char* vram, int xsize,
           int x0, int y0, int x1, int y1, unsigned char c);
void init_screen8(unsigned char* vram, int x, int y);
void putfont8(unsigned char* vram, int xsize, int x, int y, unsigned char c,
              const unsigned char* font);
void putfonts8_asc(unsigned char* vram, int xsize, int x, int y,
                   unsigned char c, const char* s);
void init_mouse_cursor8(unsigned char* mouse, unsigned char bc);
void putblock8_8(unsigned char* vram, int xsize, int pxsize, int pysize,
                 int px0, int py0, const unsigned char* buf, int bxsize);

#endif
