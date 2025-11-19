// TODO look into why this absolute pathing is needed
#include "../include/opencl.hpp"
#include <cstddef>
#include <cstdio>
#include <sys/time.h>

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "usage: %s neurons max_incoming seed tracked_timesteps\n",
            argv[0]);
    return 1;
  }

  // Device device(
  //     select_device_with_most_flops());
  Device device(select_device_with_most_flops());

  int neurons;
  int max_incoming;
  int seed;
  int tracked_timesteps;
  sscanf(argv[1], "%d", &neurons);
  sscanf(argv[2], "%d", &max_incoming);
  sscanf(argv[3], "%d", &seed);
  sscanf(argv[4], "%d", &tracked_timesteps);

  srand(seed);

  const uint synapse_matrix_size = neurons * max_incoming; // size of vectors
  const uint charge_matrix_size =
      neurons * tracked_timesteps; // size of vectors

  Memory<uint32_t> synapse_firing_matrix(
      device, synapse_matrix_size); // allocate memory on both host and device
  Memory<float> synapse_weight_matrix(device, synapse_matrix_size);
  Memory<uint16_t> synapse_delay_matrix(device, synapse_matrix_size);

  Memory<uint16_t> incoming_synapses_vector(device, neurons);
  Memory<float> neuron_charge_vector(device, neurons);
  Memory<float> neuron_charge_matrix(
      device, charge_matrix_size); // Rows are timesteps, cols are neurons
  Memory<float> neuron_threshold_vector(device, neurons);
  Memory<float> neuron_min_potential_vector(device, neurons);
  Memory<uint16_t> neuron_fire_count_vector(device, neurons);
  Memory<float> neuron_fired_vector(device, neurons);

  Kernel neuron_leak_kernel(device, neurons, "neuron_leak_kernel",
                            neuron_charge_vector, neuron_threshold_vector,
                            neuron_min_potential_vector);
  Kernel synapse_kernel(device, synapse_matrix_size, max_incoming,
                        "synapse_kernel", synapse_firing_matrix,
                        synapse_weight_matrix, incoming_synapses_vector,
                        neuron_charge_vector);
  Kernel neuron_firing_kernel(device, synapse_matrix_size, max_incoming,
                              "neuron_firing_kernel", neuron_charge_vector,
                              neuron_threshold_vector, synapse_delay_matrix,
                              synapse_firing_matrix, incoming_synapses_vector,
                              neuron_fire_count_vector);

  for (size_t i = 0; i < neurons; i++) {
    incoming_synapses_vector[i] = rand() % max_incoming;
    neuron_threshold_vector[i] = 0.1;
    neuron_min_potential_vector[i] = 0;

    for (size_t j = 0; j < incoming_synapses_vector[i]; j++) {
      synapse_weight_matrix[(i * max_incoming) + j] = rand() / (double)RAND_MAX;
      synapse_delay_matrix[(i * max_incoming) + j] = rand() % 16;
      synapse_firing_matrix[(i * max_incoming) + j] = rand();
    }
  }

  // printf("Weights\n");
  // for (size_t neuron = 0; neuron < neurons; neuron++) {
  //   for (size_t synapse = 0; synapse < max_incoming; synapse++) {
  //     printf("%4.3f ",
  //            synapse_weight_matrix[(neuron * max_incoming) + synapse]);
  //   }
  //   printf("\n");
  // }
  // printf("\n");

  // printf("Firing Mask\n");
  // for (size_t neuron = 0; neuron < neurons; neuron++) {
  //   for (size_t synapse = 0; synapse < max_incoming; synapse++) {
  //     printf("%0X ", synapse_firing_matrix[(neuron * max_incoming) +
  //     synapse]);
  //   }
  //   printf("\n");
  // }
  // printf("\n");

  struct timeval begin;
  gettimeofday(&begin, NULL);

  synapse_firing_matrix.write_to_device();
  synapse_weight_matrix.write_to_device();
  synapse_delay_matrix.write_to_device();
  incoming_synapses_vector.write_to_device();
  neuron_charge_vector.write_to_device();
  neuron_threshold_vector.write_to_device();
  neuron_min_potential_vector.write_to_device();
  neuron_fire_count_vector.write_to_device();

  for (size_t i = 0; i < 100000; i++) {
    neuron_leak_kernel.run();
    synapse_kernel.run(); // run add_kernel on the device
    neuron_firing_kernel.run();
  }

  neuron_charge_vector
      .read_from_device(); // copy data from device memory to host memory
  neuron_fire_count_vector
      .read_from_device(); // copy data from device memory to host memory

  struct timeval end;
  gettimeofday(&end, NULL);

  double seconds = ((end.tv_usec + (1000000 * end.tv_sec)) -
                    (begin.tv_usec + (1000000 * begin.tv_sec))) /
                   1000000.0;

  fprintf(stderr, "Run took %f seconds\n", seconds);

  for (size_t i = 0; i < neuron_fire_count_vector.length(); i++) {
    printf("%hu ", neuron_fire_count_vector[i]);
  }
  printf("\n");
  for (size_t i = 0; i < neuron_charge_vector.length(); i++) {
    printf("%f ", neuron_charge_vector[i]);
  }
  printf("\n");
}
