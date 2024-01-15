#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char **argv) {
  bool res = MAX(argc, 4);
}
