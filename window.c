#include "window.h"
#include "graphics.h"
#include "sheet.h"

void make_wtitle8(unsigned char* buf, int xsize, const char* title, char act) {
  static const char closebtn[14][16] = {
    "OOOOOOOOOOOOOOO@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQQQ@@QQQQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "O$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@",
  };
  unsigned char tc, tbc;
  if (act) {
    tc = COL8_WHITE;
    tbc = COL8_DARK_BLUE;
  } else {
    tc = COL8_GRAY;
    tbc = COL8_DARK_GRAY;
  }
  boxfill8(buf, xsize, tbc, 3, 3, xsize - 3, 21);
  putfonts8_asc(buf, xsize, 24, 4, tc, title);
  for (int y = 0; y < 14; ++y) {
    for (int x = 0; x < 16; ++x) {
      unsigned char c;
      switch (closebtn[y][x]) {
      case '@':  c = COL8_BLACK; break;
      case '$':  c = COL8_DARK_GRAY; break;
      case 'Q':  c = COL8_GRAY; break;
      default:   c = COL8_WHITE; break;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
    }
  }
}

void change_wtitle8(SHTCTL* shtctl, SHEET* sht, char act) {
  int tc_new, tbc_new, tc_old, tbc_old;
  if (act) {
    tc_new = COL8_WHITE;
    tbc_new = COL8_DARK_BLUE;
    tc_old = COL8_GRAY;
    tbc_old = COL8_DARK_GRAY;
  } else {
    tc_new = COL8_GRAY;
    tbc_new = COL8_DARK_GRAY;
    tc_old = COL8_WHITE;
    tbc_old = COL8_DARK_BLUE;
  }

  unsigned char* buf = sht->buf;
  int xsize = sht->bxsize;
  for (int y = 3; y < 20; ++y) {
    for (int x = 3; x < xsize - 3; ++x) {
      unsigned char c = buf[y * xsize + x];
      if (c == tc_old && x <= xsize - 22) {
        c = tc_new;
      } else if (c == tbc_old) {
        c = tbc_new;
      }
      buf[y * xsize + x] = c;
    }
  }
  sheet_refresh(shtctl, sht, 3, 3, xsize, 21);
}

void make_window8(unsigned char* buf, int xsize, int ysize, const char* title, char act) {
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, xsize, 1);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, xsize - 1, 2);
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, 1, ysize);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, 2, ysize - 1);
  boxfill8(buf, xsize, COL8_DARK_GRAY, xsize - 2, 1, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize, ysize);
  boxfill8(buf, xsize, COL8_GRAY, 2, 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_DARK_GRAY, 1, ysize - 2, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, 0, ysize - 1, xsize, ysize);
  make_wtitle8(buf, xsize, title, act);
}

void make_textbox8(SHEET* sht, int x0, int y0, int sx, int sy, int c) {
  int x1 = x0 + sx, y1 = y0 + sy;
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 2, y0 - 3, x1 + 2, y0 - 2);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 3, y0 - 3, x0 - 2, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_WHITE, x0 - 3, y1 + 2, x1 + 2, y1 + 3);
  boxfill8(sht->buf, sht->bxsize, COL8_WHITE, x1 + 2, y0 - 3, x1 + 3, y1 + 3);
  boxfill8(sht->buf, sht->bxsize, COL8_BLACK, x0 - 1, y0 - 2, x1 + 1, y0 - 1);
  boxfill8(sht->buf, sht->bxsize, COL8_BLACK, x0 - 2, y0 - 2, x0 - 1, y1 + 1);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 2, y1 + 1, x1 + 1, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x1 + 1, y0 - 2, x1 + 2, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
}

void putfonts8_asc_sht(SHTCTL* shtctl, SHEET* sht, int x, int y, int c, int b, const char* s, int l) {
  boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8, y + 16);
  putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
  sheet_refresh(shtctl, sht, x, y, x + l * 8, y + 16);
}
