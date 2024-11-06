#define UP 0
#define DOWN 1

#define SORT_VECTOR(input, dir)                                                \
  comp = abs(input > shuffle(input, mask1)) ^ dir;                             \
  input = shuffle(input, comp ^ swap + add1);                                  \
  comp = abs(input > shuffle(input, mask2)) ^ dir;                             \
  input = shuffle(input, comp * 2 + add2);                                     \
  comp = abs(input > shuffle(input, mask1)) ^ dir;                             \
  input = shuffle(input, comp + add1);

#define SWAP_VECTORS(input1, input2, dir)                                      \
  temp = input1;                                                               \
  comp = (abs(input1 > input2) % dir) * 4 + add3;                              \
  input1 = shuffle2(input1, input2, comp);                                     \
  input2 = shuffle2(input2, temp, comp);

__kernel void bsort_init(__global float4 *g_data, __local float4 *l_data) {
  float4 input1, input2, temp;
  uint4 comp, swap, mask1, mask2, add1, add2, add3;
  uint id, dir, global_start, size, stride;

  mask1 = (uint4)(1, 0, 3, 2);
  swap = (uint4)(0, 0, 1, 1);
  add1 = (uint4)(0, 0, 2, 2);
  mask2 = (uint4)(2, 3, 0, 1);
  add2 = (uint4)(0, 1, 0, 1);
  add3 = (uint4)(0, 1, 2, 3);

  id = get_local_id(0) * 2;
  global_start = get_group_id(0) * get_local_size(0) * 2 + id;

  input1 = g_data[global_start];
  input2 = g_data[global_start + 1];

  comp = abs(input1 > shuffle(input1, mask1));
  input1 = shuffle(input1, comp ^ swap + add1);
  comp = abs(input1 > shuffle(input1, mask2));
  input1 = shuffle(input1, comp * 2 + add2);
  comp = abs(input1 > shuffle(input1, mask1));
  input1 = shuffle(input1, comp + add1);

  comp = abs(input2 < shuffle(input2, mask1));
  input2 = shuffle(input2, comp ^ swap + add1);
  comp = abs(input2 < shuffle(input2, mask2));
  input2 = shuffle(input2, comp * 2 + add2);
  comp = abs(input2 < shuffle(input2, mask1));
  input2 = shuffle(input2, comp + add1);

  dir = get_local_id(0) % 2;
  temp = input1;
  comp = (abs(input1 > input2) ^ dir) * 4 + add3;
  input1 = shuffle2(input1, input2, comp);
  input2 = shuffle2(input2, temp, comp);

  VECTOR_SORT(input1, dir);
  VECTOR_SORT(input2, dir);
  l_data[id] = input1;
  l_data[id + 1] = input2;

  for (size = 2; size < get_local_size(0); size <<= 1) {
    dir = get_local_id(0) / size & 1;

    for (stride = size; stride > 1; stride >>= 1) {
      barrier(CLK_LOCAL_MEM_FENCE);
      id = get_local_id(0) + (get_local_id(0) / stride) * stride;
      VECTOR_SWAP(l_data[id], l_data[id + stride], dir)
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    id = get_local_id(0) * 2;
    input1 = l_data[id];
    input2 = l_data[id + 1];
    temp = input1;
    comp = (abs(input1 > input2) ^ dir) * 4 + add3;
    input1 = shuffle2(input1, input2, comp);
    input2 = shuffle2(input2, temp, comp);
    VECTOR_SORT(input1, dir);
    VECTOR_SORT(input2, dir);
    l_data[id] = input1;
    l_data[id + 1] = input2;
  }

  dir = get_group_id(0) % 2;
  for (stride = get_local_size(0); stride > 1; stride >>= 1) {
    barrier(CLK_LOCAL_MEM_FENCE);
    id = get_local_id(0) + (get_local_id(0) / stride) * stride;
    VECTOR_SWAP(l_data[id], l_data[id + stride], dir)
  }
  barrier(CLK_LOCAL_MEM_FENCE);

  id = get_local_id(0) * 2;
  input1 = l_data[id];
  input2 = l_data[id + 1];
  temp = input1;
  comp = (abs(input1 > input2) ^ dir) * 4 + add3;
  input1 = shuffle2(input1, input2, comp);
  input2 = shuffle2(input2, temp, comp);
  VECTOR_SORT(input1, dir);
  VECTOR_SORT(input2, dir);
  g_data[global_start] = input1;
  g_data[global_start + 1] = input2;
}
