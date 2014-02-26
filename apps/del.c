#include "apilib.h"
#include "stdio.h"

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    const char* filename = argv[i];
    if (!api_delete(filename)) {
      printf("File not found: %s\n", filename);
      return 1;
    }
  }
  return 0;
}
