#include "apilib.h"

int main() {
  TIMER* timer = api_alloctimer();
  api_inittimer(timer, 128);
  for (int i = 20000000; i >= 20000; i -= i / 100) {
    // 20KHz ~ 20Hz, 1% decreases
    api_beep(i);
    api_settimer(timer, 1);
    if (api_getkey(1) != 128)
      break;
  }
  api_beep(0);
  return 0;
}
