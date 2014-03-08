#include "setjmp.h"

int longjmp(jmp_buf env, int val) {
  __asm volatile("movl 0(%%edx), %%ecx\n\t"/* return address */
                 "movl 4(%%edx), %%ebx\n\t"
                 "movl 8(%%edx), %%esp\n\t"
                 "movl 12(%%edx), %%ebp\n\t"
                 "movl 16(%%edx), %%esi\n\t"
                 "movl 20(%%edx), %%edi\n\t"
                 "cmpl $0, %%eax\n\t" /* if val = 0 */
                 "jne  1f\n\t"
                 "movl $1, %%eax\n\t" /* then val = 1 */
                 "1: jmp *%%ecx"
                 :: "a" (val), "d" (env));
  /* NOTREACHED */
  return 0;
}
