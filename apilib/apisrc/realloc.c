#include "stdlib.h"
#include "stddef.h"
#include "string.h"

void* realloc(void* ptr, size_t size) {
  void* newptr = malloc(size);
  if (newptr == NULL || ptr == NULL)
    return newptr;

  memcpy(newptr, ptr, size);
  free(ptr);
  return newptr;
}
