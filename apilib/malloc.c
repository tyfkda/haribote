#include "stdio.h"
#include "apilib.h"

#define HEADER_SIZE  (16)

void* malloc(int size) {
  char* p = api_malloc(size + HEADER_SIZE);
  if (p != NULL) {
    *((int*)p) = size;
    p += HEADER_SIZE;
  }
  return p;
}

void free(void* p) {
  if (p == NULL)
    return;
  char* q = (char*)p - HEADER_SIZE;
  int size = *((int*)q);
  api_free(q, size + HEADER_SIZE);
}
