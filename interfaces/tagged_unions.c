#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)

typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

#define WHITE ((RGB){.r = 255, .b = 255, .g = 255})
#define RED ((RGB){.r = 255, .b = 0, .g = 0})
#define GREEN ((RGB){.r = 0, .b = 0, .g = 255})
#define BLUE ((RGB){.r = 0, .b = 255, .g = 0})
#define MISS_COLOR ((RGB){.r = 160, .g = 200, .b = 255})

// Whiteish at top of screen
// Blue at bottom
RGB miss_color(double x, double y) {
    assert(x >= -1 && x <= 1);
    assert(y >= -1 && y <= 1);

    RGB color = MISS_COLOR;

    double height_fraction = (y - -1) / 2;

    color.r += (255 - 160) * height_fraction;
    color.g += (255 - 200) * height_fraction;

    return color;
}

typedef struct Square {
    double x;
    double y;
    double radius; // Half side length

    RGB color;
} Square;

typedef struct Circle {
    double x;
    double y;
    double radius;

    RGB color;
} Circle;

typedef struct Shape {
    enum { SQUARE, CIRCLE } tag;
    union {
        Square s;
        Circle c;
    } shape;
} Shape;

typedef struct HitRecord {
    RGB color;
    bool hit;
} HitRecord;

Square square_init(double x1, double y1, double radius, RGB color) {
    assert(x1 >= -1 && x1 <= 1);
    assert(y1 >= -1 && y1 <= 1);

    return (Square){
        .x = x1,
        .y = y1,
        .radius = radius,

        .color = color,
    };
}

HitRecord square_hit(const Square* r, double aspect_ratio, double x, double y) {
    assert(x >= -1 && x <= 1);
    assert(y >= -1 && y <= 1);

    HitRecord hr = {0};

    double x1 = r->x - r->radius * aspect_ratio;
    double x2 = r->x + r->radius * aspect_ratio;
    double y1 = r->y - r->radius;
    double y2 = r->y + r->radius;

    if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
        hr.color = r->color;
        hr.hit = true;
    } else {
        hr.color = miss_color(x, y);
        hr.hit = false;
    }

    return hr;
}

Circle circle_init(double x, double y, double r, RGB color) {
    assert(x >= -1 && x <= 1);
    assert(y >= -1 && y <= 1);

    return (Circle){
        .x = x,
        .y = y,
        .radius = r,

        .color = color,
    };
}

HitRecord circle_hit(const Circle* c, double aspect_ratio, double x, double y) {
    assert(x >= -1 && x <= 1);
    assert(y >= -1 && y <= 1);

    HitRecord hr = {0};

    // Uses the elipse collision formula
    // \frac{(x-h)^2}{r_x^2} + \frac{(y-k)^2}{r_y^2} \leq 1
    double lhs = (pow(x - c->x, 2) / pow(c->radius * aspect_ratio, 2)) +
                 (pow(y - c->y, 2) / pow(c->radius, 2));

    if (lhs <= 1) {
        hr.color = c->color;
        hr.hit = true;
    } else {
        hr.color = miss_color(x, y);
        hr.hit = false;
    }

    return hr;
}

HitRecord hit(const Shape* s, double aspect_ratio, double x, double y) {
    switch (s->tag) {
    case SQUARE:
        return square_hit(&s->shape.s, aspect_ratio, x, y);
    case CIRCLE:
        return circle_hit(&s->shape.c, aspect_ratio, x, y);
    }
}

Shape shape_random() {
    if (rand() / (double)RAND_MAX < 0.5) {
        // Make circle
        return (Shape){
            .tag = CIRCLE,
            .shape.c = circle_init(
                (rand() / (RAND_MAX / 2.0)) - 1.0,
                (rand() / (RAND_MAX / 2.0)) - 1.0,
                rand() / (double)RAND_MAX / 4,
                (RGB){.r = rand() % 255, .g = rand() % 255, .b = rand() % 255}),
        };
    } else {
        // Make square
        return (Shape){
            .tag = SQUARE,
            .shape.s = square_init(
                (rand() / (RAND_MAX / 2.0)) - 1.0,
                (rand() / (RAND_MAX / 2.0)) - 1.0,
                rand() / (double)RAND_MAX / 4,
                (RGB){.r = rand() % 255, .g = rand() % 255, .b = rand() % 255}),
        };
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s width height random_seed\n", argv[0]);
        return 1;
    }

    // Grab width and height from argv
    size_t width;
    size_t height;

    int scanned = sscanf(argv[1], "%zu", &width);
    assert(scanned == 1);

    scanned = sscanf(argv[2], "%zu", &height);
    assert(scanned == 1);

    double aspect_ratio = (double)height / (double)width;

    // Grab random seed
    int seed;
    scanned = sscanf(argv[3], "%d", &seed);
    assert(scanned == 1);
    srand(seed);

    Shape scene[10];
    const size_t scene_size = 10;
    for (size_t i = 0; i < scene_size; i++) {
        scene[i] = shape_random();
    }

    // RENDERING SCENE

    // Start by writing the PPM image header before writing each pixel
    printf("P3\n"); // PPM w/ ASCII format
    printf("%zu %zu\n", width, height);
    printf("255\n"); // Max value per channel of RGB

    struct timeval begin;
    gettimeofday(&begin, NULL);

    // Now loop through each pixel, printing it's values as we go
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            HitRecord hr;
            for (int depth = scene_size - 1; depth >= 0; depth--) {
                // Calculate x/y pixel coords, y is flipped so we can render 0,0
                // as the top left corner
                double x = ((double)col / ((double)width / 2) - 1);
                double y = -1 * ((double)row / ((double)height / 2) - 1);

                hr = hit(&scene[depth], aspect_ratio, x, y);
                if (hr.hit) {
                    // We hit something, therefore we can avoid checking any
                    // other object
                    break;
                }
            }

            // We've either checked all object and hit nothing, or we broke out
            // because we hit something Either way we now need to write the
            // pixel color to our image
            printf("%hhu %hhu %hhu\n", hr.color.r, hr.color.g, hr.color.b);
        }
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    double seconds = ((end.tv_usec + (1000000 * end.tv_sec)) -
                      (begin.tv_usec + (1000000 * begin.tv_sec))) /
                     1000000.0;

    fprintf(stderr, "Rendering took %f seconds\n", seconds);
};
