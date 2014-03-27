#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

struct FIFO;
struct SHEET;
struct SHTCTL;

typedef struct {
  char cyls;
  char bootDrive;
  char leds;  // Keyboard LED status
  char vmode;  // Video mode
  short scrnx, scrny;  // Screen resolution
  unsigned char* vram;
} BOOTINFO;

typedef struct {
  BOOTINFO* binfo;
  struct FIFO* fifo;
  struct SHTCTL* shtctl;
  struct SHEET* sht_back;
  struct SHEET* key_win;
  unsigned int memtotal;
  int key_mod;

  // Mouse.
  int mx, my;
  int mobtn;  // Old mouse button state.
  int mmx, mmy, new_mx, new_my, new_wx, new_wy;  // Mouse drag position.
  struct SHEET* sht_dragging;
  char drag_moved;
} OsInfo;

#define ADR_DISKIMG   0x00100000

const OsInfo* getOsInfo(void);

void set_active_window(struct SHEET* sheet);

#endif
