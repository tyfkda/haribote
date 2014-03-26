#ifndef __CTYPE_H__
#define __CTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

int isdigit(int c);
int isspace(int c);

int toupper(int c);
int tolower(int c);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
