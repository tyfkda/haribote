#include "stdlib.h"
#include "apilib.h"
#include "stddef.h"
#include "stdio.h"

#define HEADER_SIZE  (16)

void* malloc(size_t size) {
  char* p = api_malloc(size + HEADER_SIZE);
  if (p != NULL) {
    *((int*)p) = size;
    p += HEADER_SIZE;
  } else {
    fprintf(stderr, "malloc failed! (size=%ud)\n", size);
    exit(1);
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
