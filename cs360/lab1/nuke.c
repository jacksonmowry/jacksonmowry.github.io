// Jackson Mowry
// Wed Aug 21 2024
// Nuke lab, lab 1 360
#include <math.h>
#include <stdio.h>

struct Point {
  int x;
  int y;
};

static double calc(const struct Point *p1, const struct Point *p2) {
  return sqrt(pow(p2->x - p1->x, 2) + pow(p2->y - p1->y, 2));
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "usage: ./nuke <initial_blast_strength> "
                    "<attenuation_factor> <start X> <start Y> < input.txt\n");
    return 1;
  }

  float initial_blast_strength;
  double attenuation_factor;
  int x_start;
  int y_start;

  if (sscanf(argv[1], "%f", &initial_blast_strength) != 1) {
    fprintf(stderr, "Error reading inital blast strength\n");
    return 1;
  }
  if (sscanf(argv[2], "%lf", &attenuation_factor) != 1) {
    fprintf(stderr, "Error reading attenuation factor\n");
    return 1;
  }
  if (sscanf(argv[3], "%d", &x_start) != 1) {
    fprintf(stderr, "Error reading inital X coord\n");
    return 1;
  }
  if (sscanf(argv[4], "%d", &y_start) != 1) {
    fprintf(stderr, "Error reading inital Y coord\n");
    return 1;
  }

  struct Point start = {.x = x_start, .y = y_start};

  int x;
  int y;
  char name[65] = {};

  while (scanf("%d %d %64s", &x, &y, name) != EOF) {
    double distance = calc(&start, &(struct Point){.x = x, .y = y});

    double dosage = pow(attenuation_factor, distance) * initial_blast_strength;

    printf("%-8s: %8.3f Sv\n", name, dosage);
  }
}
