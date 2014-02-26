#include "sheet.h"
#include "memory.h"
#include "stdio.h"  // for NULL

enum SHEET_FLAGS {
  SHEET_FREE = 0,
  SHEET_USED = 1,
};

SHTCTL* shtctl_init(MEMMAN* memman, unsigned char* vram, int xsize, int ysize) {
  SHTCTL* ctl = (SHTCTL*)memman_alloc_4k(memman, sizeof(SHTCTL));
  if (ctl == NULL)
    return NULL;
  ctl->map = (unsigned char*)memman_alloc_4k(memman, xsize * ysize);
  if (ctl->map == NULL) {
    memman_free_4k(memman, ctl, sizeof(SHTCTL));
    return NULL;
  }
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1;  // No sheet.
  for (int i = 0; i < MAX_SHEETS; ++i)
    ctl->sheets0[i].flags = SHEET_FREE;
  return ctl;
}

SHEET* sheet_alloc(SHTCTL* ctl) {
  for (int i = 0; i < MAX_SHEETS; ++i) {
    SHEET* sheet = &ctl->sheets0[i];
    if (sheet->flags == SHEET_FREE) {
      sheet->flags = SHEET_USED;
      sheet->height = -1;
      sheet->task = NULL;
      return sheet;
    }
  }
  return NULL;
}

void sheet_setbuf(SHEET* sheet, unsigned char* buf, int xsize, int ysize, int col_inv) {
  sheet->buf = buf;
  sheet->bxsize = xsize;
  sheet->bysize = ysize;
  sheet->col_inv = col_inv;
}

static void sheet_refreshmap(SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
  unsigned char* map = ctl->map;
  int xsize = ctl->xsize, ysize = ctl->ysize;
  if (vx0 < 0)  vx0 = 0;
  if (vy0 < 0)  vy0 = 0;
  if (vx1 > xsize)  vx1 = xsize;
  if (vy1 > ysize)  vy1 = ysize;
  for (int h = h0; h <= ctl->top; ++h) {
    SHEET* sheet = ctl->sheets[h];
    unsigned char sid = sheet - ctl->sheets0;
    unsigned char* buf = sheet->buf;
    int bxsize = sheet->bxsize, bysize = sheet->bysize;
    int bx0 = vx0 - sheet->vx0;
    int by0 = vy0 - sheet->vy0;
    int bx1 = vx1 - sheet->vx0;
    int by1 = vy1 - sheet->vy0;
    if (bx0 < 0)  bx0 = 0;
    if (by0 < 0)  by0 = 0;
    if (bx1 > bxsize)  bx1 = bxsize;
    if (by1 > bysize)  by1 = bysize;
    if (sheet->col_inv == -1) {  // No transparent sheet.
      for (int by = by0; by < by1; ++by) {
        int vy = sheet->vy0 + by;
        for (int bx = bx0; bx < bx1; ++bx) {
          int vx = sheet->vx0 + bx;
          map[vy * xsize + vx] = sid;
        }
      }
    } else {
      unsigned char col_inv = sheet->col_inv;
      for (int by = by0; by < by1; ++by) {
        int vy = sheet->vy0 + by;
        for (int bx = bx0; bx < bx1; ++bx) {
          int vx = sheet->vx0 + bx;
          if (buf[by * bxsize + bx] != col_inv)
            map[vy * xsize + vx] = sid;
        }
      }
    }
  }
}

static void sheet_refreshsub(SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
  unsigned char* vram = ctl->vram;
  unsigned char* map = ctl->map;
  int xsize = ctl->xsize, ysize = ctl->ysize;
  if (vx0 < 0)  vx0 = 0;
  if (vy0 < 0)  vy0 = 0;
  if (vx1 > xsize)  vx1 = xsize;
  if (vy1 > ysize)  vy1 = ysize;
  for (int h = h0; h <= h1; ++h) {
    SHEET* sheet = ctl->sheets[h];
    unsigned char sid = sheet - ctl->sheets0;
    unsigned char* buf = sheet->buf;
    int bxsize = sheet->bxsize, bysize = sheet->bysize;
    int bx0 = vx0 - sheet->vx0;
    int by0 = vy0 - sheet->vy0;
    int bx1 = vx1 - sheet->vx0;
    int by1 = vy1 - sheet->vy0;
    if (bx0 < 0)  bx0 = 0;
    if (by0 < 0)  by0 = 0;
    if (bx1 > bxsize)  bx1 = bxsize;
    if (by1 > bysize)  by1 = bysize;
    for (int by = by0; by < by1; ++by) {
      int vy = sheet->vy0 + by;
      for (int bx = bx0; bx < bx1; ++bx) {
        int vx = sheet->vx0 + bx;
        if (map[vy * xsize + vx] == sid)
          vram[vy * xsize + vx] = buf[by * bxsize + bx];
      }
    }
  }
}

void sheet_updown(SHTCTL* ctl, SHEET* sheet, int height) {
  int old = sheet->height;

  if (height > ctl->top + 1)
    height = ctl->top + 1;
  if (height < -1)
    height = -1;
  sheet->height = height;

  if (old > height) {
    if (height >= 0) {
      for (int h = old; h > height; --h) {
        ctl->sheets[h] = ctl->sheets[h - 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sheet;
      sheet_refreshmap(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height + 1);
      sheet_refreshsub(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height + 1, old);
    } else {
      if (ctl->top > old) {
        for (int h = old; h < ctl->top; ++h) {
          ctl->sheets[h] = ctl->sheets[h + 1];
          ctl->sheets[h]->height = h;
        }
      }
      --ctl->top;
      sheet_refreshmap(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, 0);
      sheet_refreshsub(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, 0, old - 1);
    }
  } else if (old < height) {
    if (old >= 0) {
      for (int h = old; h < height; ++h) {
        ctl->sheets[h] = ctl->sheets[h + 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sheet;
    } else {
      for (int h = ctl->top; h >= height; --h) {
        ctl->sheets[h + 1] = ctl->sheets[h];
        ctl->sheets[h + 1]->height = h + 1;
      }
      ctl->sheets[height] = sheet;
      ++ctl->top;
    }
    sheet_refreshmap(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height);
    sheet_refreshsub(ctl, sheet->vx0, sheet->vy0, sheet->vx0 + sheet->bxsize, sheet->vy0 + sheet->bysize, height, height);
  }
}

void sheet_refresh(SHTCTL* ctl, SHEET* sheet, int bx0, int by0, int bx1, int by1) {
  if (sheet->height >= 0)
    sheet_refreshsub(ctl, sheet->vx0 + bx0, sheet->vy0 + by0, sheet->vx0 + bx1, sheet->vy0 + by1, sheet->height, sheet->height);
}

void sheet_slide(SHTCTL* ctl, SHEET* sheet, int vx0, int vy0) {
  int old_vx0 = sheet->vx0, old_vy0 = sheet->vy0;
  sheet->vx0 = vx0;
  sheet->vy0 = vy0;
  if (sheet->height >= 0) {
    sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sheet->bxsize, old_vy0 + sheet->bysize, 0);
    sheet_refreshmap(ctl, vx0, vy0, vx0 + sheet->bxsize, vy0 + sheet->bysize, sheet->height);
    sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sheet->bxsize, old_vy0 + sheet->bysize, 0, sheet->height - 1);
    sheet_refreshsub(ctl, vx0, vy0, vx0 + sheet->bxsize, vy0 + sheet->bysize, sheet->height, sheet->height);
  }
}

void sheet_free(SHTCTL* ctl, SHEET* sheet) {
  if (sheet->height >= 0)
    sheet_updown(ctl, sheet, -1);
  sheet->flags = SHEET_FREE;
}
