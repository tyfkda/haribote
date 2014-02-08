// Keyboard

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "fifo.h"

static const int PORT_KEYDAT = 0x0060;
static const int PORT_KEYCMD = 0x0064;

void wait_KBC_sendready(void);
void init_keyboard(FIFO* fifo, int data0);

#endif
