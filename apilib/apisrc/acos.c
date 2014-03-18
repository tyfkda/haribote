#include "math.h"

double acos(double cost) {
  double sint = sqrt(1 - cost * cost);
  double tant = sint / cost;
  return atan(tant);
}
