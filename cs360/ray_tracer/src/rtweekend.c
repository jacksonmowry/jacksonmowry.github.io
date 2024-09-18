#include "rtweekend.h"

#include <stdint.h>
#include <stdlib.h>

const double pi = 3.14159265358979323846;

double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }

double random_double() {
    // Generate a random double in the range [0, 1)
    static uint64_t state = 0.0;
    // Constants for a simple LCG
    const unsigned long long a = 6364136223846793005ULL;
    const unsigned long long c = 1;
    const unsigned long long m = 1ULL << 63; // 2^63

    // Update state
    state = (a * state + c) % m;

    // Convert state to double in the range [0, 1)
    return (double)state / (double)m;
}

double random_double_min_max(double min, double max) {
    return min + (max - min) * random_double();
}
