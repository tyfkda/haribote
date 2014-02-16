// Memory management

#ifndef __MEMORY_H__
#define __MEMORY_H__

#define MEMMAN_FREES  (4090)
#define MEMMAN_ADDR   (0x003c0000)

typedef struct {
  void* addr;
  unsigned int size;
} FREEINFO;

typedef struct MEMMAN {
  int frees, maxfrees, lostsize, losts;
  FREEINFO free[MEMMAN_FREES];
} MEMMAN;

unsigned int memtest(unsigned int start, unsigned int end);

void memman_init(MEMMAN* man);
unsigned int memman_total(MEMMAN* man);
void* memman_alloc(MEMMAN *man, unsigned int size);
int memman_free(MEMMAN *man, void* addr, unsigned int size);
void* memman_alloc_4k(MEMMAN* man, unsigned int size);
int memman_free_4k(MEMMAN* man, void* addr, unsigned int size);

#endif
