#include "math.h"

double asin(double sint) {
  double cost = sqrt(1 - sint * sint);
  double tant = sint / cost;
  return atan(tant);
}
