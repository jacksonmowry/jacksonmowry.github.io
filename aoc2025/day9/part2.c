#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef struct Point {
    long x;
    long y;
} Point;

typedef struct Line {
    Point start;
    Point end;
} Line;

int main() {
    Point points[600];
    Line lines[600];
    size_t num_points = 0;
    size_t num_lines = 0;
    char buf[256];

    while (fgets(buf, 1024, stdin)) {
        sscanf(buf, "%ld,%ld\n", &points[num_points].x, &points[num_points].y);
        num_points++;

        if (num_points > 1) {
            Point a = points[num_points - 2];
            Point b = points[num_points - 1];

            Point min = (Point){.x = MIN(a.x, b.x), .y = MIN(a.y, b.y)};
            Point max = (Point){.x = MAX(a.x, b.x), .y = MAX(a.y, b.y)};

            lines[num_lines++] = (Line){.start = min, .end = max};
        }
    }

    {
        Point a = points[0];
        Point b = points[num_points - 1];

        lines[num_lines++] = (Line){.start = a, .end = b};
    }

    unsigned long long largest_area = 0;
    for (size_t i = 0; i < num_points; i++) {
        for (size_t j = i + 1; j < num_points; j++) {
            unsigned long long x_len = labs(points[i].x - points[j].x) + 1;
            unsigned long long y_len = labs(points[i].y - points[j].y) + 1;
            unsigned long long area = x_len * y_len;
            if (area > largest_area) {
                // Now check if it's green/red inside
                Point a = points[i];
                Point b = points[j];

                for (size_t line = 0; line < num_lines; line++) {
                    long x1 = MIN(a.x, b.x);
                    long x2 = MAX(a.x, b.x);
                    long y1 = MIN(a.y, b.y);
                    long y2 = MAX(a.y, b.y);

                    Line l = lines[line];

                    printf("Checking against (%ld,%ld) (%ld,%ld)\n", l.start.x,
                           l.start.y, l.end.x, l.end.y);

                    if (l.end.x > x1 && l.start.x < x2 && l.end.y > y1 &&
                        l.start.y < y2) {
                        goto END;
                    }
                }

                // No intersections
                largest_area = area;
            }

        END:;

            /* largest_area = */
            /*     x_len * y_len > largest_area ? x_len * y_len : largest_area;
             */
        }
    }

    printf("Largest area: %llu\n", largest_area);
}
