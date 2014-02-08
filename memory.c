#include "memory.h"
#include "bootpack.h"

#define CR0_CACHE_DISABLE  0x60000000
#define EFLAGS_AC_BIT      0x00040000

static unsigned int memtest_sub(unsigned int start, unsigned int end) {
  const unsigned int pat0 = 0xaa55aa55, pat1 = ~pat0;
  unsigned int i;
  for (i = start; i <= end; i += 0x1000) {
    volatile unsigned int* p = (unsigned int*)(i + 0xffc);
    unsigned int old = *p;
    *p = pat0;
    *p ^= 0xffffffff;
    if (*p != pat1) {
    not_memory:
      *p = old;
      break;
    }
    *p ^= 0xffffffff;
    if (*p != pat0)
      goto not_memory;
    *p = old;
  }
  return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
  char flg486 = 0;

  unsigned int eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT;  // AC-bit = 1
  io_store_eflags(eflg);
  eflg = io_load_eflags();
  if ((eflg & EFLAGS_AC_BIT) != 0)  // AC becomes 0 on 386.
    flg486 = 1;
  eflg &= ~EFLAGS_AC_BIT;  // AC-bit = 0
  io_store_eflags(eflg);

  if (flg486) {
    unsigned int cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;  // Disable cache.
    store_cr0(cr0);
  }

  unsigned int i = memtest_sub(start, end);
  if (flg486) {
    unsigned int cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;  // Enable cache.
    store_cr0(cr0);
  }
  return i;
}

void memman_init(struct MEMMAN* man) {
  man->frees = 0;
  man->maxfrees = 0;
  man->lostsize = 0;
  man->losts = 0;
}

unsigned int memman_total(struct MEMMAN* man) {
  unsigned int t = 0;
  for (int i = 0; i < man->frees; ++i) {
    t += man->free[i].size;
  }
  return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
  for (int i = 0; i < man->frees; ++i) {
    if (man->free[i].size >= size) {
      unsigned int a = man->free[i].addr;
      man->free[i].addr += size;
      man->free[i].size -= size;
      if (man->free[i].size == 0) {
        --man->frees;
        for (; i < man->frees; ++i)
          man->free[i] = man->free[i + 1];
      }
      return a;
    }
  }
  return 0;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
  int i;
  for (i = 0; i < man->frees; ++i)
    if (man->free[i].addr > addr)
      break;
  // free[i - 1].addr < addr < free[i].addr
  if (i > 0) {
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
      man->free[i - 1].size += size;
      if (i < man->frees) {
        if (addr + size == man->free[i].addr) {
          man->free[i - 1].size += man->free[i].size;
          --man->frees;
          for (; i < man->frees; ++i)
            man->free[i] = man->free[i + 1];
        }
      }
      return 0;
    }
  }
  if (i < man->frees) {
    if (addr + size == man->free[i].addr) {
      man->free[i].addr = addr;
      man->free[i].size += size;
      return 0;
    }
  }
  if (man->frees < MEMMAN_FREES) {
    for (int j = man->frees; j > i; --j)
      man->free[j] = man->free[j - 1];
    if (++man->frees > man->maxfrees)
      man->maxfrees = man->frees;
    man->free[i].addr = addr;
    man->free[i].size = size;
    return 0;
  }
  ++man->losts;
  man->lostsize += size;
  return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size) {
  size = (size + 0xfff) & ~0xfff;
  return memman_alloc(man, size);
}

int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size) {
  size = (size + 0xfff) & ~0xfff;
  return memman_free(man, addr, size);
}
