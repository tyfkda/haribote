#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

#include "mouse.h"

struct FIFO;
struct SHEET;
struct SHTCTL;

typedef struct {
  char cyls;
  char leds;  // Keyboard LED status
  char vmode;  // Video mode
  char reserve;
  short scrnx, scrny;  // Screen resolution
  unsigned char* vram;
} BOOTINFO;

typedef struct {
  BOOTINFO* binfo;
  struct FIFO* fifo;
  struct SHTCTL* shtctl;
  struct SHEET* sht_back;
  struct SHEET* key_win;
  MOUSE_DEC mdec;
  unsigned int memtotal;
  int key_mod;

  // Mouse.
  int mx, my;
  int mobtn;  // Old mouse button state.
  int mmx, mmy, new_mx, new_my, new_wx, new_wy;  // Mouse drag position.
  struct SHEET* sht_dragging;
  char drag_moved;
} OsInfo;

#define ADR_BOOTINFO  0x00000ff0
#define ADR_DISKIMG   0x00100000

const OsInfo* getOsInfo(void);

void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_clts(void);
void io_fnsave(int* addr);
void io_frstor(int* addr);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void start_app(int eip, int cs, int esp, int ds, int* tss_esp0);

#endif
