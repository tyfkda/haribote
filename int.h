// Interrupt

#ifndef __INT_H__
#define __INT_H__

#define PIC0_ICW1  0x0020
#define PIC0_OCW2  0x0020
#define PIC0_IMR   0x0021
#define PIC0_ICW2  0x0021
#define PIC0_ICW3  0x0021
#define PIC0_ICW4  0x0021
#define PIC1_ICW1  0x00a0
#define PIC1_OCW2  0x00a0
#define PIC1_IMR   0x00a1
#define PIC1_ICW2  0x00a1
#define PIC1_ICW3  0x00a1
#define PIC1_ICW4  0x00a1

void init_pic(void);

void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler2c(void);
void asm_hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);

#endif
