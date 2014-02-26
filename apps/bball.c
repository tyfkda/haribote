#include "apilib.h"
#include "stdlib.h"

void HariMain(void) {
  struct POINT {
    int x, y;
  };
  static const struct POINT table[16] = {
    { 204, 129 }, { 195,  90 }, { 172,  58 }, { 137,  38 }, {  98,  34 },
    {  61,  46 }, {  31,  73 }, {  15, 110 }, {  15, 148 }, {  31, 185 },
    {  61, 212 }, {  98, 224 }, { 137, 220 }, { 172, 200 }, { 195, 168 },
    { 204, 129 },
  };

  unsigned char buf[216 * 237];
  int win = api_openwin(buf, 216, 237, -1, "bball");
  api_boxfilwin(win, 8, 29, 207, 228, 0);
  for (int i = 0; i < 15; ++i) {
    for (int j = i; ++j < 16; ) {
      int dis = j - i;
      if (dis >= 8)
        dis = 15 - dis;
      if (dis != 0)
        api_linewin(win, table[i].x, table[i].y, table[j].x, table[j].y, 8 - dis);
    }
  }
  api_refreshwin(win, 0, 0, 216, 237);

  for (;;)
    if (api_getkey(1) == 0x0a)
      break;
  exit(0);
}
