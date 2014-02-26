#ifndef __WINDOW_H__
#define __WINDOW_H__

struct SHTCTL;
struct SHEET;

void make_wtitle8(unsigned char* buf, int xsize, const char* title, char act);
void change_wtitle8(struct SHTCTL* shtctl, struct SHEET* sheet, char act);
void make_window8(unsigned char* buf, int xsize, int ysize, const char* title, char act);
void make_textbox8(struct SHEET* sheet, int x0, int y0, int sx, int sy, int c);
void putfonts8_asc_sht(struct SHTCTL* shtctl, struct SHEET* sheet, int x, int y, int c, int b, const char* s, int l);

#endif
