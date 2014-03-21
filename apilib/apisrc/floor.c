#include "math.h"

double floor(double x) {
  if (x >= 0) {
    long l = (long)x;
    return (double)l;
  } else {
    double y = -x;
    long l = (long)y;
    if (y > l)
      return (double)(-l - 1);
    else
      return (double)(-l);
  }
}
