#ifndef __STDDEF_H__
#define __STDDEF_H__

#ifndef FALSE
#define FALSE  (0)
#define TRUE   (1)
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL   (0)
#else
#define NULL   ((void*)0)
#endif
#endif


typedef int  ptrdiff_t;

#endif
