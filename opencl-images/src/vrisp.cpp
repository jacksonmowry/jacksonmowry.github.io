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

  Memory<float> synapse_weight_matrix(device, synapse_matrix_size);
  Memory<uint16_t> synapse_delay_matrix(device, synapse_matrix_size);
  Memory<uint16_t> synapse_from_matrix(device, synapse_matrix_size);

  Memory<uint16_t> incoming_synapses_vector(device, neurons);
  Memory<float> neuron_charge_matrix(
      device, charge_matrix_size); // Rows are timesteps, cols are neurons
  Memory<float> neuron_threshold_vector(device, neurons);
  Memory<float> neuron_min_potential_vector(device, neurons);
  Memory<uint32_t> neuron_fire_count_vector(device, neurons);
  Memory<bool> neuron_fired_vector(device, neurons);
  Memory<bool> neuron_leak_vector(device, neurons);

  Memory<uint32_t> current_timestep(device, 1);
  current_timestep[0] = 0;

  Kernel neuron_leak_fired_kernel(
      device, neurons, "neuron_leak_fired_kernel", tracked_timesteps, neurons,
      current_timestep, neuron_charge_matrix, neuron_threshold_vector,
      neuron_min_potential_vector, neuron_fired_vector,
      neuron_fire_count_vector, neuron_leak_vector);
  Kernel neuron_super_kernel(
      device, neurons, "neuron_super_kernel", current_timestep, neurons,
      tracked_timesteps, max_incoming, neuron_charge_matrix,
      synapse_weight_matrix, synapse_delay_matrix, synapse_from_matrix,
      incoming_synapses_vector, neuron_fired_vector);
  Kernel increment_timestep_kernel(device, 1, "increment_timestep",
                                   current_timestep);

  for (size_t i = 0; i < neurons; i++) {
    incoming_synapses_vector[i] = rand() % max_incoming;
    neuron_threshold_vector[i] = rand() / (double)RAND_MAX;
    neuron_min_potential_vector[i] = 0;
    neuron_fire_count_vector[i] = 0;
    neuron_leak_vector[i] = 0;

    for (size_t j = 0; j < incoming_synapses_vector[i]; j++) {
      synapse_weight_matrix[(i * max_incoming) + j] =
          rand() / (double)RAND_MAX / 4;
      synapse_weight_matrix[(i * max_incoming) + j] = 1;
      synapse_delay_matrix[(i * max_incoming) + j] = 1;
      // rand() % (tracked_timesteps - 1) + 1;
      synapse_from_matrix[(i * max_incoming) + j] = rand() % neurons;
      synapse_from_matrix[(i * max_incoming) + j] = 1;
    }

    neuron_charge_matrix[((0) * neurons) + i] = rand() / (double)RAND_MAX;
  }

  printf("Incoming\n");
  for (size_t neuron = 0; neuron < neurons; neuron++) {
    printf("%hu\n", incoming_synapses_vector[neuron]);
  }
  printf("\n");

  printf("Weights\n");
  for (size_t neuron = 0; neuron < neurons; neuron++) {
    for (size_t synapse = 0; synapse < max_incoming; synapse++) {
      printf("%4.3f ",
             synapse_weight_matrix[(neuron * max_incoming) + synapse]);
    }
    printf("\n");
  }
  printf("\n");

  printf("Delays\n");
  for (size_t neuron = 0; neuron < neurons; neuron++) {
    for (size_t synapse = 0; synapse < max_incoming; synapse++) {
      printf("%hu ", synapse_delay_matrix[(neuron * max_incoming) + synapse]);
    }
    printf("\n");
  }
  printf("\n");

  printf("From\n");
  for (size_t neuron = 0; neuron < neurons; neuron++) {
    for (size_t synapse = 0; synapse < max_incoming; synapse++) {
      printf("%hu ", synapse_from_matrix[(neuron * max_incoming) + synapse]);
    }
    printf("\n");
  }
  printf("\n");

  printf("Thresholds\n");
  for (size_t neuron = 0; neuron < neurons; neuron++) {
    printf("%4.3f ", neuron_threshold_vector[neuron]);
  }
  printf("\n\n");

  printf("Neuron charge matrix:\n");
  for (size_t i = 0; i < tracked_timesteps; i++) {
    for (size_t j = 0; j < neurons; j++) {
      const uint32_t idx = (i * neurons) + j;
      printf("%f ", neuron_charge_matrix[idx]);
    }
    puts("");
  }
  printf("\n");

  struct timeval begin;
  gettimeofday(&begin, NULL);

  synapse_weight_matrix.write_to_device();
  synapse_delay_matrix.write_to_device();
  synapse_from_matrix.write_to_device();
  incoming_synapses_vector.write_to_device();
  neuron_charge_matrix.write_to_device();
  neuron_threshold_vector.write_to_device();
  neuron_min_potential_vector.write_to_device();
  neuron_fire_count_vector.write_to_device();
  neuron_fired_vector.write_to_device();
  neuron_leak_vector.write_to_device();
  current_timestep.write_to_device();

  for (size_t i = 0; i < 100000; i++) {
    neuron_leak_fired_kernel.run();
    neuron_super_kernel.run();       // run add_kernel on the device
    increment_timestep_kernel.run(); // run add_kernel on the device
  }

  neuron_charge_matrix
      .read_from_device(); // copy data from device memory to host memory
  neuron_fire_count_vector
      .read_from_device(); // copy data from device memory to host memory
  current_timestep.read_from_device();

  struct timeval end;
  gettimeofday(&end, NULL);

  double seconds = ((end.tv_usec + (1000000 * end.tv_sec)) -
                    (begin.tv_usec + (1000000 * begin.tv_sec))) /
                   1000000.0;

  fprintf(stderr, "Run took %f seconds\n", seconds);

  printf("We processed %u timesteps\n\n", current_timestep[0]);

  printf("Neuron fire counts: \n");
  for (size_t i = 0; i < neuron_fire_count_vector.length(); i++) {
    printf("%u ", neuron_fire_count_vector[i]);
  }
  printf("\n\n");

  printf("Neuron charge matrix:\n");
  for (size_t i = 0; i < tracked_timesteps; i++) {
    for (size_t j = 0; j < neurons; j++) {
      const uint32_t idx = (i * neurons) + j;
      printf("%f ", neuron_charge_matrix[idx]);
    }
    puts("");
  }
  printf("\n");
}
