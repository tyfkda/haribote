#include "stdlib.h"
#include "stddef.h"
#include "string.h"

void* calloc(size_t nmemb, size_t size) {
  size_t totalSize = nmemb * size;
  void* newptr = malloc(totalSize);
  if (newptr == NULL)
    return newptr;

  memset(newptr, 0x00, totalSize);
  return newptr;
}
