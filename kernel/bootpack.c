#include "apilib.h"
#include "bootpack.h"
#include "console.h"
#include "dsctbl.h"
#include "fd.h"
#include "fifo.h"
#include "graphics.h"
#include "int.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "timer.h"
#include "window.h"

#define ADR_BOOTINFO  (0x00000ff0)

#define MOD_LSHIFT    (1 << 0)
#define MOD_RSHIFT    (1 << 1)
#define MOD_LCONTROL  (1 << 2)
#define MOD_RCONTROL  (1 << 3)
#define MOD_SHIFT_MASK    (MOD_LSHIFT | MOD_RSHIFT)
#define MOD_CONTROL_MASK  (MOD_LCONTROL | MOD_RCONTROL)

static OsInfo s_osinfo;
const OsInfo* getOsInfo(void)  { return &s_osinfo; }

int toupper(int c) {
  if ('a' <= c && c <= 'z')
    return c + ('A' - 'a');
  return c;
}

int tolower(int c) {
  if ('A' <= c && c <= 'Z')
    return c + ('a' - 'A');
  return c;
}

static void keywin_off(SHTCTL* shtctl, SHEET* key_win) {
  change_wtitle8(shtctl, key_win, FALSE);
  if ((key_win->flags & 0x20) != 0)
    fifo_put(&key_win->task->fifo, 3);  // Send hide cursor message.
}

static void keywin_on(SHTCTL* shtctl, SHEET* key_win) {
  change_wtitle8(shtctl, key_win, TRUE);
  if ((key_win->flags & 0x20) != 0)
    fifo_put(&key_win->task->fifo, 2);  // Send show cursor message.
}

static const char keytable[2][0x80] = {
#if 1  // English keyboard mapping.
  {  // Normal.
      0,0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0,'\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   KEY_F1,   KEY_F2,   KEY_F3,   KEY_F4,   KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,   0,   0, '7', KEY_UP, '9', '-', KEY_LEFT, '5', KEY_RIGHT, '+', '1',
    KEY_DOWN, '3', '0', '.',   0,   0,   0, KEY_F11, KEY_F12,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0,
  },
  {  // Shift.
      0,0x1b, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',   0,   KEY_F1,   KEY_F2,   KEY_F3,   KEY_F4,   KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,   0,   0, '7', KEY_UP, '9', '-', KEY_LEFT, '5', KEY_RIGHT, '+', '1',
    KEY_DOWN, '3', '0', '.',   0,   0,   0, KEY_F11, KEY_F12,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, '_',   0,   0,   0,   0,   0,   0,   0,   0,   0, '|',   0,   0,
  },
#else  // Japanese keyboard mapping.
  {  // Normal.
      0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   KEY_F1,   KEY_F2,   KEY_F3,   KEY_F4,   KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,   0,   0, '7', KEY_UP, '9', '-', KEY_LEFT, '5', KEY_RIGHT, '+', '1',
    KEY_DOWN, '3', '0', '.',   0,   0,   0, KEY_F11, KEY_F12,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0,
  },
  {  // Shift.
      0,   0, '!', '"', '#', '$', '%', '&', '*', '(', ')',   0, '=', '~',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{',0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*',   0,   0, '}', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',   0,   KEY_F1,   KEY_F2,   KEY_F3,   KEY_F4,   KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,   0,   0, '7', KEY_UP, '9', '-', KEY_LEFT, '5', KEY_RIGHT, '+', '1',
    KEY_DOWN, '3', '0', '.',   0,   0,   0, KEY_F11, KEY_F12,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, '_',   0,   0,   0,   0,   0,   0,   0,   0,   0, '|',   0,   0,
  },
#endif
};

void set_active_window(SHEET* sheet) {
  OsInfo* osinfo = &s_osinfo;
  if (osinfo->key_win != sheet) {
    keywin_off(osinfo->shtctl, osinfo->key_win);
    keywin_on(osinfo->shtctl, osinfo->key_win = sheet);
  }
  sheet_updown(osinfo->shtctl, sheet, osinfo->shtctl->top - 1);
}

static void handle_key_event(OsInfo* osinfo, int keycode) {
  switch (keycode) {
  case 0x0f:  // Tab.
    if (osinfo->key_win != NULL && osinfo->shtctl->top > 2)
      // Move old bottom to the top.
      set_active_window(osinfo->shtctl->sheets[1]);
    break;
  case 0x2a:  // Left shift on.
    osinfo->key_mod |= MOD_LSHIFT;
    break;
  case 0x36:  // Right shift on.
    osinfo->key_mod |= MOD_RSHIFT;
    break;
  case 0xaa:  // Left shift off.
    osinfo->key_mod &= ~MOD_LSHIFT;
    break;
  case 0xb6:  // Right shift off.
    osinfo->key_mod &= ~MOD_RSHIFT;
    break;
  case 0x1d:  // Left control on.
  case 0x3a:  // Caps lock: treat as left control key.
    osinfo->key_mod |= MOD_LCONTROL;
    break;
  case 0x9d:  // Left control off.
  case 0xba:  // Caps lock: treat as left control key.
    osinfo->key_mod &= ~MOD_LCONTROL;
    break;
  case 0x3b:  // F1
    if ((osinfo->key_mod & MOD_SHIFT_MASK) != 0) {  // Shift + F1
      TASK* task = osinfo->key_win->task;
      if (task != NULL && task->tss.ss0 != 0) {
        io_cli();
        task->tss.eax = (int)&(task->tss.esp0);
        task->tss.eip = (int)asm_end_app;
        io_sti();
        task_run(task, -1, 0);  // Wake to execute termination.
      }
    }
    break;
  case 0x3c:  // F2
    if ((osinfo->key_mod & MOD_SHIFT_MASK) != 0) {  // Shift + F2 : Create console.
      if (osinfo->key_win != NULL)
        keywin_off(osinfo->shtctl, osinfo->key_win);
      osinfo->key_win = open_console(osinfo->shtctl);
      sheet_slide(osinfo->shtctl, osinfo->key_win, 32, 4);
      sheet_updown(osinfo->shtctl, osinfo->key_win, osinfo->shtctl->top);
      keywin_on(osinfo->shtctl, osinfo->key_win);
    }
    break;
  default:
    if (keycode < 0x80) {  // Normal character.
      int shift = (osinfo->key_mod & MOD_SHIFT_MASK) != 0;
      unsigned char key = keytable[shift][keycode];
      if (key != 0 && osinfo->key_win != NULL) {  // Normal character.
        if (!shift)
          key = tolower(key);
        if ((osinfo->key_mod & MOD_CONTROL_MASK) != 0)
          key = toupper(key) - ('A' - 1);
        fifo_put(&osinfo->key_win->task->fifo, key + 256);
      }
    }
    break;
  }
}

static int close_button_clicked(SHEET* sheet, int x, int y) {
  return sheet->bxsize - 21 <= x && x <= sheet->bxsize - 5 && 5 <= 5 && y < 19;
}
static int title_bar_clicked(SHEET* sheet, int x, int y) {
  return 3 <= x && x < sheet->bxsize - 3 && 3 <= y && y < 21;
}

static SHEET* get_window_at(OsInfo* osinfo, int mx, int my) {
  for (int j = osinfo->shtctl->top; --j > 0; ) {
    SHEET* sheet = osinfo->shtctl->sheets[j];
    int x = mx - sheet->vx0;
    int y = my - sheet->vy0;
    if (0 <= x && x < sheet->bxsize && 0 <= y && y < sheet->bysize &&
        sheet->buf[y * sheet->bxsize + x] != sheet->col_inv)
      return sheet;
  }
  return NULL;
}

static void mouse_left_clicked(OsInfo* osinfo) {
  SHEET* sheet = get_window_at(osinfo, osinfo->mx, osinfo->my);
  if (sheet == NULL)
    return;

  // Activate this sheet.
  sheet_updown(osinfo->shtctl, sheet, osinfo->shtctl->top - 1);
  int x = osinfo->mx - sheet->vx0;
  int y = osinfo->my - sheet->vy0;
  if (close_button_clicked(sheet, x, y)) {
    // Close button clicked.
    if ((sheet->flags & 0x10) != 0) {  // Window created by application.
      TASK* task = sheet->task;
      io_cli();
      task->tss.eax = (int)&(task->tss.esp0);
      task->tss.eip = (int)asm_end_app;
      io_sti();
      task_run(task, -1, 0);  // Wake to execute termination.
    } else {  // Console window.
      TASK* task = sheet->task;
      sheet_updown(osinfo->shtctl, sheet, -1);
      if (sheet == osinfo->key_win) {
        keywin_off(osinfo->shtctl, osinfo->key_win);
        keywin_on(osinfo->shtctl, osinfo->key_win = osinfo->shtctl->sheets[osinfo->shtctl->top - 1]);
      }
      io_cli();
      fifo_put(&task->fifo, 4);
      io_sti();
    }
    return;
  }
  if (sheet != osinfo->key_win) {
    keywin_off(osinfo->shtctl, osinfo->key_win);
    osinfo->key_win = sheet;
    keywin_on(osinfo->shtctl, osinfo->key_win);
  }
  if (title_bar_clicked(sheet, x, y)) {
    osinfo->mmx = osinfo->mx;  // Go to drag mode.
    osinfo->mmy = osinfo->my;
    osinfo->new_wx = sheet->vx0;
    osinfo->new_wy = sheet->vy0;
    osinfo->sht_dragging = sheet;
  }
}

static void handle_mouse_event(OsInfo* osinfo, MOUSE_DEC* mdec, int code) {
  if (mouse_decode(mdec, code) == 0)
    return;

  int mtrg = mdec->btn & ~osinfo->mobtn;
  osinfo->mobtn = mdec->btn;
  // Move mouse cursor.
  osinfo->mx += mdec->dx;
  osinfo->my += mdec->dy;
  if (osinfo->mx < 0)  osinfo->mx = 0;
  if (osinfo->my < 0)  osinfo->my = 0;
  if (osinfo->mx >= osinfo->binfo->scrnx - 1)  osinfo->mx = osinfo->binfo->scrnx - 1;
  if (osinfo->my >= osinfo->binfo->scrny - 1)  osinfo->my = osinfo->binfo->scrny - 1;

  osinfo->new_mx = osinfo->mx;
  osinfo->new_my = osinfo->my;
  if (mtrg & MOUSE_LBUTTON)
    mouse_left_clicked(osinfo);
  if (osinfo->mmx >= 0) {  // Drag mode.
    if ((mdec->btn & MOUSE_LBUTTON) == 0) {  // Mouse left button released.
      osinfo->mmx = -1;  // Drag end.
    } else {
      int dx = osinfo->mx - osinfo->mmx;
      int dy = osinfo->my - osinfo->mmy;
      osinfo->new_wx += dx;
      osinfo->new_wy += dy;
      osinfo->mmx = osinfo->mx;
      osinfo->mmy = osinfo->my;
      osinfo->drag_moved = TRUE;
    }
  }
}

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  FIFO fifo;
  int fifobuf[128];
  fifo_init(&fifo, 128, fifobuf, NULL);
  OsInfo* osinfo = &s_osinfo;
  osinfo->fifo = &fifo;
  init_pit();
  init_keyboard(&fifo, 256);
  MOUSE_DEC mdec;
  enable_mouse(&fifo, 512, &mdec);
  io_out8(PIC0_IMR, 0xb8);  // Enable PIT, PIC1, keyboard and FDC.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  osinfo->memtotal = memtest(0x00400000, 0xbfffffff);
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  memman_init(memman);
  memman_free(memman, (void*)0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
  memman_free(memman, (void*)0x00400000, osinfo->memtotal - 0x00400000);

  init_palette();

  osinfo->binfo = (BOOTINFO*)ADR_BOOTINFO;
  osinfo->shtctl = shtctl_init(memman, osinfo->binfo->vram, osinfo->binfo->scrnx, osinfo->binfo->scrny);

  init_fdc();

  TASK* task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 2);

  // sht_back
  osinfo->sht_back = sheet_alloc(osinfo->shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, osinfo->binfo->scrnx * osinfo->binfo->scrny);
  sheet_setbuf(osinfo->sht_back, buf_back, osinfo->binfo->scrnx, osinfo->binfo->scrny, -1);
  init_screen8(buf_back, osinfo->binfo->scrnx, osinfo->binfo->scrny);

  // sht_cons
  osinfo->key_win = open_console(osinfo->shtctl);

  // sht_mouse
  SHEET* sht_mouse = sheet_alloc(osinfo->shtctl);
  unsigned char buf_mouse[16 * 16];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8(buf_mouse, 99);
  osinfo->mx = (osinfo->binfo->scrnx - 16) / 2;
  osinfo->my = (osinfo->binfo->scrny - 28 - 16) / 2;
  osinfo->mmx = osinfo->mmy = osinfo->new_mx = -1;
  osinfo->new_my = osinfo->new_wx = osinfo->new_wy = 0;
  osinfo->mobtn = 0;
  osinfo->drag_moved = FALSE;
  osinfo->sht_dragging = NULL;

  sheet_slide(osinfo->shtctl, osinfo->sht_back, 0, 0);
  sheet_slide(osinfo->shtctl, osinfo->key_win, 8, 2);  // console
  sheet_slide(osinfo->shtctl, sht_mouse, osinfo->mx, osinfo->my);
  sheet_updown(osinfo->shtctl, osinfo->sht_back, 0);
  sheet_updown(osinfo->shtctl, osinfo->key_win, 1);
  sheet_updown(osinfo->shtctl, sht_mouse, 2);

  osinfo->key_mod = 0;
  keywin_on(osinfo->shtctl, osinfo->key_win);

  for (;;) {
    io_cli();

    if (fifo_empty(&fifo)) {
      if (osinfo->new_mx >= 0) {
        io_sti();
        sheet_slide(osinfo->shtctl, sht_mouse, osinfo->new_mx, osinfo->new_my);
        osinfo->new_mx = -1;
      }
      if (osinfo->drag_moved) {
        io_sti();
        sheet_slide(osinfo->shtctl, osinfo->sht_dragging, osinfo->new_wx, osinfo->new_wy);
        osinfo->drag_moved = FALSE;
        if (osinfo->mmx < 0)  // Drag released.
          osinfo->sht_dragging = NULL;
      }
      task_sleep(task_a);
      io_sti();
      continue;
    }

    int i = fifo_get(&fifo);
    io_sti();

    if (osinfo->key_win != NULL && osinfo->key_win->flags == 0) {  // Console window closed.
      if (osinfo->shtctl->top == 1) {  // No window, only mouse and background.
        osinfo->key_win = NULL;
      } else {
        keywin_on(osinfo->shtctl, osinfo->key_win = osinfo->shtctl->sheets[osinfo->shtctl->top - 1]);
      }
    }
    if (256 <= i && i < 512) {  // Keyboard data.
      handle_key_event(osinfo, i - 256);
    } else if (512 <= i && i < 768) {  // Mouse data.
      handle_mouse_event(osinfo, &mdec, i - 512);
    } else if (768 <= i && i < 1024) {  // Close console request.
      close_console(osinfo->shtctl, osinfo->shtctl->sheets0 + (i - 768));
    } else if (1024 <= i && i < 2024) {  // Close console task request.
      close_constask(taskctl->tasks0 + (i - 1024));
    } else if (2024 <= i && i < 2280) {  // Close console only.
      SHEET* sheet2 = osinfo->shtctl->sheets0 + (i - 2024);
      memman_free_4k(memman, sheet2->buf, 256 * 165);
      sheet_free(osinfo->shtctl, sheet2);
    }
  }
}
