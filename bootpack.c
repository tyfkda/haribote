#include "bootpack.h"
#include "dsctbl.h"
#include "graphic.h"
#include "int.h"
#include "stdio.h"

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  io_out8(PIC0_IMR, 0xf9);  // Enable PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  struct BOOTINFO* binfo = (struct BOOTINFO*)ADR_BOOTINFO;
  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

  unsigned char mcursor[256];
  init_mouse_cursor8(mcursor, COL8_DARK_CYAN);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  char s[40];
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

  for (;;) {
    io_cli();
    if (keybuf.flag == 0) {
      io_stihlt();
    } else {
      int i = keybuf.data;
      keybuf.flag = 0;
      io_sti();

      char s[4];
      sprintf(s, "%02x", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 16, 32);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
    }
  }
}
