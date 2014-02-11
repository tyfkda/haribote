#include "sheet.h"
#include "memory.h"
#include "stdio.h"  // for NULL

#define SHEET_USE  (1)

SHTCTL* shtctl_init(MEMMAN* memman, unsigned char* vram, int xsize, int ysize) {
  SHTCTL* ctl = (SHTCTL*)memman_alloc_4k(memman, sizeof(SHTCTL));
  if (ctl == NULL)
    return NULL;
  ctl->map = (unsigned char*)memman_alloc_4k(memman, xsize * ysize);
  if (ctl->map == NULL) {
    memman_free_4k(memman, (int)ctl, sizeof(SHTCTL));
    return NULL;
  }
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1;  // No sheet.
  for (int i = 0; i < MAX_SHEETS; ++i)
    ctl->sheets0[i].flags = 0;  // Not used.
  return ctl;
}

SHEET* sheet_alloc(SHTCTL* ctl) {
  for (int i = 0; i < MAX_SHEETS; ++i) {
    SHEET* sht = &ctl->sheets0[i];
    if (sht->flags == 0) {
      sht->flags = SHEET_USE;
      sht->height = -1;
      return sht;
    }
  }
  return NULL;
}

void sheet_setbuf(SHEET* sht, unsigned char* buf, int xsize, int ysize, int col_inv) {
  sht->buf = buf;
  sht->bxsize = xsize;
  sht->bysize = ysize;
  sht->col_inv = col_inv;
}

static void sheet_refreshmap(SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
  unsigned char* map = ctl->map;
  int xsize = ctl->xsize, ysize = ctl->ysize;
  if (vx0 < 0)  vx0 = 0;
  if (vy0 < 0)  vy0 = 0;
  if (vx1 > xsize)  vx1 = xsize;
  if (vy1 > ysize)  vy1 = ysize;
  for (int h = h0; h <= ctl->top; ++h) {
    SHEET* sht = ctl->sheets[h];
    unsigned char sid = sht - ctl->sheets0;
    unsigned char* buf = sht->buf;
    int bxsize = sht->bxsize, bysize = sht->bysize;
    unsigned char col_inv = sht->col_inv;
    int bx0 = vx0 - sht->vx0;
    int by0 = vy0 - sht->vy0;
    int bx1 = vx1 - sht->vx0;
    int by1 = vy1 - sht->vy0;
    if (bx0 < 0)  bx0 = 0;
    if (by0 < 0)  by0 = 0;
    if (bx1 > bxsize)  bx1 = bxsize;
    if (by1 > bysize)  by1 = bysize;
    for (int by = by0; by < by1; ++by) {
      int vy = sht->vy0 + by;
      for (int bx = bx0; bx < bx1; ++bx) {
        int vx = sht->vx0 + bx;
        if (buf[by * bxsize + bx] != col_inv)
          map[vy * xsize + vx] = sid;
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
    SHEET* sht = ctl->sheets[h];
    unsigned char sid = sht - ctl->sheets0;
    unsigned char* buf = sht->buf;
    int bxsize = sht->bxsize, bysize = sht->bysize;
    int bx0 = vx0 - sht->vx0;
    int by0 = vy0 - sht->vy0;
    int bx1 = vx1 - sht->vx0;
    int by1 = vy1 - sht->vy0;
    if (bx0 < 0)  bx0 = 0;
    if (by0 < 0)  by0 = 0;
    if (bx1 > bxsize)  bx1 = bxsize;
    if (by1 > bysize)  by1 = bysize;
    for (int by = by0; by < by1; ++by) {
      int vy = sht->vy0 + by;
      for (int bx = bx0; bx < bx1; ++bx) {
        int vx = sht->vx0 + bx;
        if (map[vy * xsize + vx] == sid)
          vram[vy * xsize + vx] = buf[by * bxsize + bx];
      }
    }
  }
}

void sheet_updown(SHTCTL* ctl, SHEET* sht, int height) {
  int old = sht->height;

  if (height > ctl->top + 1)
    height = ctl->top + 1;
  if (height < -1)
    height = -1;
  sht->height = height;

  if (old > height) {
    if (height >= 0) {
      for (int h = old; h > height; --h) {
        ctl->sheets[h] = ctl->sheets[h - 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
    } else {
      if (ctl->top > old) {
        for (int h = old; h < ctl->top; ++h) {
          ctl->sheets[h] = ctl->sheets[h + 1];
          ctl->sheets[h]->height = h;
        }
      }
      --ctl->top;
      sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
      sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
    }
  } else if (old < height) {
    if (old >= 0) {
      for (int h = old; h < height; ++h) {
        ctl->sheets[h] = ctl->sheets[h + 1];
        ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {
      for (int h = ctl->top; h >= height; --h) {
        ctl->sheets[h + 1] = ctl->sheets[h];
        ctl->sheets[h + 1]->height = h + 1;
      }
      ctl->sheets[height] = sht;
      ++ctl->top;
    }
    sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
    sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
  }
}

void sheet_refresh(SHTCTL* ctl, SHEET* sht, int bx0, int by0, int bx1, int by1) {
  if (sht->height >= 0)
    sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
}

void sheet_slide(SHTCTL* ctl, SHEET* sht, int vx0, int vy0) {
  int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (sht->height >= 0) {
    sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
    sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
    sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
    sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
  }
}

void sheet_free(SHTCTL* ctl, SHEET* sht) {
  if (sht->height >= 0)
    sheet_updown(ctl, sht, -1);
  sht->flags = 0;
}
