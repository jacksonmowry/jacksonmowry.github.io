#ifndef RTWEEKEND_H_
#define RTWEEKEND_H_

#include <stdlib.h>
const double pi = 3.14159265358979323846;

double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }

double random_double() { return rand() / (RAND_MAX + 1.0); }

double rnadom_double_min_max(double min, double max) {
    return min + (max - min) * random_double();
}

#endif // RTWEEKEND_H_
