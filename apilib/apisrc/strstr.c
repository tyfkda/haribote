#include "string.h"
#include "stddef.h"

char *strstr(const char *haystack, const char *needle) {
  size_t len = strlen(needle);
  for (; *haystack != '\0'; ++haystack) {
    if (strncmp(haystack, needle, len) == 0)
      return (char*)haystack;
  }
  return NULL;
}
