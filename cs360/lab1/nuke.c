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
  return sqrt(((p2->x - p1->x) * (p2->x - p1->x)) +
              ((p2->y - p1->y) * (p2->y - p1->y)));
}

int main(int argc, char *argv[]) {
  // Error checking cli arguments, both for them being present, and that they
  // are the correct type
  if (argc != 5) {
    fprintf(stderr,
            "Usage: %s <blast strength> <attenuation> <blast x> <blast y>\n",
            argv[0]);
    return 1;
  }

  float initial_blast_strength;
  double attenuation_factor;
  int x_start;
  int y_start;

  if (sscanf(argv[1], "%f", &initial_blast_strength) != 1) {
    fprintf(stderr, "Error: unable to read blast strength.\n");
    return 1;
  }
  if (sscanf(argv[2], "%lf", &attenuation_factor) != 1) {
    fprintf(stderr, "Error: unable to read blast attenuation.\n");
    return 1;
  }
  if (sscanf(argv[3], "%d", &x_start) != 1) {
    fprintf(stderr, "Error: unable to read blast x coordinate.\n");
    return 1;
  }
  if (sscanf(argv[4], "%d", &y_start) != 1) {
    fprintf(stderr, "Error: unable to read blast y coordinate.\n");
    return 1;
  }

  // Create the "starting" point for our blast
  struct Point start = {.x = x_start, .y = y_start};

  int x;
  int y;
  char name[65] = {}; // 65th bit for the '\0' sentinal

  while (scanf("%d %d %64s", &x, &y, name) == 3) {
    // Read in line-by-line perform the calculation and print out the result
    double distance = calc(&start, &(struct Point){.x = x, .y = y});

    double dosage = pow(attenuation_factor, distance) * initial_blast_strength;

    printf("%-8s: %8.3lf Sv\n", name, dosage);
  }
}
