#ifndef __SETJMP_H__
#define  __SETJMP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef long jmp_buf[6];

int setjmp(jmp_buf env);
int longjmp(jmp_buf env, int val);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
