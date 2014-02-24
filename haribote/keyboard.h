// Keyboard

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "fifo.h"

#define PORT_KEYDAT  (0x0060)
#define PORT_KEYCMD  (0x0064)

#define KEY_UP       (0x80)
#define KEY_LEFT     (0x81)
#define KEY_RIGHT    (0x82)
#define KEY_DOWN     (0x83)
#define KEY_F1       (0x91)
#define KEY_F2       (0x92)
#define KEY_F3       (0x93)
#define KEY_F4       (0x94)
#define KEY_F5       (0x95)
#define KEY_F6       (0x96)
#define KEY_F7       (0x97)
#define KEY_F8       (0x98)
#define KEY_F9       (0x99)
#define KEY_F10      (0x9a)
#define KEY_F11      (0x9b)
#define KEY_F12      (0x9c)

void wait_KBC_sendready(void);
void init_keyboard(FIFO* fifo, int data0);

#endif
