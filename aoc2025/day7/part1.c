#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Point {
    int x;
    int y;
} Point;

char map[1024][1024] = {0};
int height = 0;
int width = 0;

Point splits[4096] = {0};
int num_splits = 0;

int sort(const void *a, const void *b) {
    Point *pa = (Point *)a;
    Point *pb = (Point *)b;

    if (pa->x < pb->x) {
        return -1;
    } else if (pa->x > pb->x) {
        return 1;
    } else if (pa->y < pb->y) {
        return -1;
    } else if (pa->y > pb->y) {
        return 1;
    } else {
        return 0;
    }
}

void rec(int x, int y) {
    while (map[y][x] != '^' && y < height) {
        /* if (y != 0) { */
        /*     map[y][x] = '|'; */
        /* } */
        y++;
    }

    if (y >= height) {
        return;
    }

    for (int i = 0; i < num_splits; i++) {
        if (splits[i].x == x && splits[i].y == y) {
            return;
        }
    }
    /* printf("I am spitting at (%d, %d)\n", x, y); */
    splits[num_splits++] = (Point){.x = x, .y = y};
    if (x - 1 >= 0) {
        rec(x - 1, y);
    }
    if (x + 1 < width) {
        rec(x + 1, y);
    }
}

void start_rec() {
    int x = strcspn(map[0], "S");
    int y = 0;

    rec(x, y);
}

int main() {

    while (fgets(map[height], 1024, stdin)) {
        map[height][strcspn(map[height], "\n")] = '\0';
        height++;
    }

    width = strlen(map[0]);

    start_rec();

    qsort(splits, num_splits, sizeof(Point), sort);

    Point tmp = splits[0];
    int count = 1;
    for (int i = 1; i < num_splits; i++) {
        if (splits[i].x != tmp.x || splits[i].y != tmp.y) {
            tmp = splits[i];
            count++;
        }
    }

    printf("Splits: %d\n", count);

    /* printf("Map looks like: \n"); */

    /* for (int i = 0; i < height; i++) { */
    /*     puts(map[i]); */
    /* } */
}
