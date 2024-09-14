#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <stdbool.h>

typedef struct {
    double min;
    double max;
} interval;

double interval_size(const interval i);
bool interval_contains(const interval i, double x);
bool interval_surrounds(const interval i, double x);
double interval_clamp(interval i, double x);

#endif // INTERVAL_H_
