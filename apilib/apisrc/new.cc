#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

void* operator new(size_t size) {
  return malloc(size);
}

void operator delete(void* ptr) {
  free(ptr);
}

void* operator new[](size_t size) {
  return malloc(size);
}

void operator delete[](void* ptr) {
  free(ptr);
}

extern "C" {
  void __cxa_pure_virtual() {
    // TODO: Raise exception.
  }
}
