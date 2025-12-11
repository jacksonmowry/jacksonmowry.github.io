#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Point {
    long x;
    long y;
} Point;

int main() {
    Point points[4096];
    size_t num_points = 0;
    char buf[1024];

    while (fgets(buf, 1024, stdin)) {
        sscanf(buf, "%ld,%ld\n", &points[num_points].x, &points[num_points].y);
        num_points++;
    }

    unsigned long long largest_area = 0;
    for (size_t i = 0; i < num_points; i++) {
        for (size_t j = i + 1; j < num_points; j++) {
            unsigned long long x_len = labs(points[i].x - points[j].x) + 1;
            unsigned long long y_len = labs(points[i].y - points[j].y) + 1;

            largest_area =
                x_len * y_len > largest_area ? x_len * y_len : largest_area;
        }
    }

    printf("Largest area: %llu\n", largest_area);
}
