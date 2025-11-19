// TODO look into why this absolute pathing is needed
#include "../include/opencl.hpp"
#include <cstdio>
#include <sys/time.h>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s width height seed\n", argv[0]);
    return 1;
  }

  // Device device(
  //     select_device_with_most_flops());
  Device device(select_device_with_most_memory());

  int height;
  int width;
  int seed;
  sscanf(argv[1], "%d", &width);
  sscanf(argv[2], "%d", &height);
  sscanf(argv[3], "%d", &seed);

  srand(seed);

  const uint N = height * width; // size of vectors
  const uint M = 10;

  Memory<float> x(device, M); // allocate memory on both host and device
  Memory<float> y(device, M);
  Memory<float> r(device, M);
  Memory<uint32_t> color(device, M);
  Memory<bool> type(device, M);
  Memory<uint32_t> pixels(device, N);

  Kernel img_kernel(device, N, "img_kernel", width, height, M, x, y, r, type,
                    color, pixels);

  for (uint n = 0u; n < M; n++) {
    x[n] = (rand() / ((double)RAND_MAX / 2)) - 1; // initialize memory
    y[n] = (rand() / ((double)RAND_MAX / 2)) - 1; // initialize memory
    r[n] = (rand() / ((double)RAND_MAX)) / 4;
    type[n] = rand() % 2;
    color[n] = rand();
  }

  struct timeval begin;
  gettimeofday(&begin, NULL);

  x.write_to_device(); // copy data from host memory to device memory
  y.write_to_device();
  r.write_to_device();
  type.write_to_device();
  color.write_to_device();
  img_kernel.run();          // run add_kernel on the device
  pixels.read_from_device(); // copy data from device memory to host memory

  struct timeval end;
  gettimeofday(&end, NULL);

  double seconds = ((end.tv_usec + (1000000 * end.tv_sec)) -
                    (begin.tv_usec + (1000000 * begin.tv_sec))) /
                   1000000.0;

  fprintf(stderr, "Rendering took %f seconds\n", seconds);

  // printf("P3\n"); // PPM w/ ASCII format
  // printf("%u %u\n", width, height);
  // printf("255\n"); // Max value per channel of RGB

  // for (size_t i = 0; i < N; i++) {
  // printf("%d %d %d\n", pixels[i] & 0xFF, (pixels[i] >> 8) & 0xFF,
  //        (pixels[i] >> 16) & 0xFF);
  // }
}
