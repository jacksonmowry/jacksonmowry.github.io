#include "../include/kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(
string opencl_c_container() {
  return R( // ########################## begin of OpenCL C code
            // ####################################################################

      kernel void img_kernel(uint width, uint height, uint num_shapes,
                             global float *x, global float *y, global float *r,
                             global bool *type, global uchar4 *color,
                             global uchar4 *pixels) {
        const uint n = get_global_id(0);
        const float aspect_ratio = (float)height / (float)width;

        const uint local_x = n % width;
        const uint local_y = n / width;

        const float x_coord = ((float)local_x / ((float)width / 2) - 1);
        const float y_coord = -1 * ((float)local_y / ((float)height / 2) - 1);

        uchar4 pixel_color;
        pixel_color.x = 160;
        pixel_color.y = 200;
        pixel_color.z = 255;

        float height_fraction = (y_coord + 1) / 2;

        pixel_color.x += (255 - 160) * height_fraction;
        pixel_color.y += (255 - 200) * height_fraction;

        for (int depth = num_shapes - 1; depth >= 0; depth--) {
          if (type[depth]) {
            // Square
            if (fabs(x_coord - x[depth]) <= r[depth] * aspect_ratio &&
                fabs(y_coord - y[depth]) <= r[depth]) {
              pixel_color = color[depth];
              break;
            }
          } else {
            // Circle
            float lhs =
                (pow(x_coord - x[depth], 2) / pow(r[depth] * aspect_ratio, 2)) +
                (pow(y_coord - y[depth], 2) / pow(r[depth], 2));

            if (lhs <= 1) {
              pixel_color = color[depth];
              break;
            }
          }
        }

        pixels[n] = pixel_color;
      }

      kernel void neuron_leak_kernel(
          global float *neuron_charge_vector,
          global float *neuron_threshold_vector,
          global float *neuron_min_potential_vector) {
        const uint o = get_group_id(0); // Which neuron

        neuron_charge_vector[o] =
            max(neuron_min_potential_vector[o], neuron_charge_vector[o]);
      }

      kernel void synapse_kernel(global uint * firing_matrix,
                                 global float *weight_matrix,
                                 global ushort *incoming_synapses_vector,
                                 global float *neuron_charge_vector) {
        const uint o = get_group_id(0); // Which neuron
        const uint m = get_local_id(0); // Which synapse of the neuron
        const uint max_incoming = get_local_size(0);

        if (m > incoming_synapses_vector[(o * max_incoming) + m]) {
          return;
        }

        firing_matrix[(o * max_incoming) + m] >>= 1;
        const float charge = (firing_matrix[(o * max_incoming) + m] & 1)
                                 ? weight_matrix[(o * max_incoming) + m]
                                 : 0;

        const float charge_update = work_group_reduce_add(charge);

        if (m == 0) {
          neuron_charge_vector[o] += charge_update;
        }
      }

      // The super kernel uses a charge table, and must be run after the leak
      // kernel
      // kernel void neuron_super_kernel(global float *neuron_charge_vector,
      //                                 global float *neuron_threshold_vector,
      //                                 global ushort *synapse_delay_matrix,
      //                                 global uint *synapse_firing_matrix,
      //                                 global ushort
      //                                 *incoming_synapses_vector, global
      //                                 ushort *neuron_fire_count_vector) {
      //   const uint o = get_group_id(0); // Which neuron

      //   for (uint i = 0; i < incoming_synapses_vector[o]; i++) {
      //     if (neuron_charge_matrix[()])
      //   }

      //   if (neuron_charge_vector[o] < neuron_threshold_vector[o]) {
      //     return;
      //   }

      //   synapse_firing_matrix[idx] |= 1 << synapse_delay_matrix[idx];

      //   if (m == 0) {
      //     neuron_fire_count_vector[o]++;
      //     neuron_charge_vector[o] = 0;
      //   }
      // }

      kernel void neuron_firing_kernel(
          global float *neuron_charge_vector,
          global float *neuron_threshold_vector,
          global ushort *synapse_delay_matrix,
          global uint *synapse_firing_matrix,
          global ushort *incoming_synapses_vector,
          global ushort *neuron_fire_count_vector) {
        const uint o = get_group_id(0); // Which neuron
        const uint m = get_local_id(0); // Which synapse of the neuron
        const uint max_incoming = get_local_size(0);
        const uint idx = (o * max_incoming) + m;

        if (m > incoming_synapses_vector[idx]) {
          return;
        }

        if (neuron_charge_vector[o] < neuron_threshold_vector[o]) {
          return;
        }

        synapse_firing_matrix[idx] |= 1 << synapse_delay_matrix[idx];

        if (m == 0) {
          neuron_fire_count_vector[o]++;
          neuron_charge_vector[o] = 0;
        }
      }

  );
} // ############################################################### end of
  // OpenCL C code
  // #####################################################################
