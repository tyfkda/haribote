#ifndef __STDARG_H__
#define __STDARG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC__
#error stdarg is not implemented
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef __builtin_va_list  va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_end(ap)  __builtin_va_end(ap)
#define va_arg(ap, type)  __builtin_va_arg(ap, type)
#define va_copy(dst, src)  __builtin_va_copy(dst, src)

int vsprintf(char *str, const char *fmt, va_list ap);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
