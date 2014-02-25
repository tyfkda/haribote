#include "stdint.h"
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
  void* __dso_handle;

  int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle) {
    //return printf("cxa_atexit: %p, %p, %p\n", func, arg, dso_handle);
    (void)func;
    (void)arg;
    (void)dso_handle;
    return 0;
  }

  void __cxa_pure_virtual() {
    // TODO: Raise exception.
  }
}
