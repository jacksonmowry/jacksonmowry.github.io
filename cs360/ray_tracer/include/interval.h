#pragma once

#include <stdbool.h>

typedef struct {
    double min;
    double max;
} Interval;

double interval_size(const Interval i);
bool interval_contains(const Interval i, double x);
bool interval_surrounds(const Interval i, double x);
double interval_clamp(Interval i, double x);
