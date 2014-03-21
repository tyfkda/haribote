#include "math.h"

double ceil(double x) {
  if (x < 0) {
    long l = (long)(-x);
    return (double)(-l);
  } else {
    long l = (long)x;
    if (x > l)
      return (double)(x + 1);
    else
      return (double)l;
  }
}
