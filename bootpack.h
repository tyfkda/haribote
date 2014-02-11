#ifndef __BOOTPACK_H__
#define __BOOTPACK_H__

typedef struct {
  char cyls;
  char leds;  // Keyboard LED status
  char vmode;  // Video mode
  char reserve;
  short scrnx, scrny;  // Screen resolution
  unsigned char* vram;
} BOOTINFO;

#define ADR_BOOTINFO  0x00000ff0;

void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
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

#endif