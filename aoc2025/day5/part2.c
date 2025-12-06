#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

typedef struct Range {
    long unsigned lower;
    long unsigned upper;
} Range;

static int cmp(const void *a, const void *b) {
    Range *ar = (Range *)a;
    Range *br = (Range *)b;

    if (ar->upper < br->upper) {
        return -1;
    } else if (ar->upper > br->upper) {
        return 1;
    } else if (ar->lower < br->lower) {
        return -1;
    } else {
        return 0;
    }
}

int main() {
    Range ranges[4096] = {0};
    long num_ranges = 0;
    long fresh = 0;

    char buf[256];
    while (true) {
        fgets(buf, 256, stdin);
        if (isspace(buf[0])) {
            break;
        }

        sscanf(buf, "%ld-%ld", &ranges[num_ranges].lower,
               &ranges[num_ranges].upper);
        num_ranges++;
    }

    qsort(ranges, num_ranges, sizeof(Range), cmp);

    for (int i = num_ranges - 1; i > 0; i--) {

        if (ranges[i].lower <= ranges[i - 1].upper &&
            ranges[i - 1].lower <= ranges[i].upper) {
            // We have overlap
            Range tmp = ranges[i];
            printf("Merging [%lu,%lu] and [%lu,%lu] into ", tmp.lower,
                   tmp.upper, ranges[i - 1].lower, ranges[i - 1].upper);
            ranges[i - 1].lower = MIN(tmp.lower, ranges[i - 1].lower);
            ranges[i - 1].upper = MAX(tmp.upper, ranges[i - 1].upper);
            printf("[%lu,%lu]\n", ranges[i - 1].lower, ranges[i - 1].upper);
        } else {
            // No overlap
            fresh += (ranges[i].upper - ranges[i].lower + 1);
            printf("Range [%lu,%lu] = %lu\n", ranges[i].lower, ranges[i].upper,
                   ranges[i].upper - ranges[i].lower + 1);
        }
    }

    fresh += (ranges[0].upper - ranges[0].lower + 1);
    printf("Range [%lu,%lu] = %lu\n", ranges[0].lower, ranges[0].upper,
           ranges[0].upper - ranges[0].lower + 1);

    printf("Fresh Total: %ld\n", fresh);
}
