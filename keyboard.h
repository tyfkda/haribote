// Keyboard

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

static const int PORT_KEYDAT = 0x0060;
static const int PORT_KEYCMD = 0x0064;

extern struct FIFO8 keyfifo;

void wait_KBC_sendready(void);
void init_keyboard(void);

#endif
