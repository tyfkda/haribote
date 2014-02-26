#ifndef _ICXXABI_H
#define _ICXXABI_H
 
#define ATEXIT_MAX_FUNCS  (32)  // (128)
 
#ifdef __cplusplus
extern "C" {
#endif
 
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
void __cxa_finalize(void *f);
 
#ifdef __cplusplus
};
#endif
 
#endif
