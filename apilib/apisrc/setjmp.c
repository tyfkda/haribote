#include "setjmp.h"

int setjmp(jmp_buf env) {
  void *return_addr = __builtin_return_address(0);
  __asm volatile("movl %%edx, 0(%%ecx)\n\t"/* return address */
                 "movl %%ebx, 4(%%ecx)\n\t"
                 "movl %%esp, 8(%%ecx)\n\t"
                 "movl %%ebp, 12(%%ecx)\n\t"
                 "movl %%esi, 16(%%ecx)\n\t"
                 "movl %%edi, 20(%%ecx)"
                 :: "d" (return_addr), "c" (env));
  return 0;
}
