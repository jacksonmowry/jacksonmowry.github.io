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

__kernel void bsort8(__global float4* data, int dir) {
    __local float4 input1 = data[0];
    __local float4 input2 = data[1];
    __local float4 temp;

    __local uint4 comp;
    __local uint4 swap = (uint4)(0,0,1,1);
    __local uint4 mask1 = (uint4)(1,0,3,2);
    __local uint4 mask2 = (uint4)(2,3,0,1);
    __local uint4 add1 = (uint4)(0,0,2,2);
    __local uint4 add2 = (uint4)(0,1,0,1);
    __local uint4 add3 = (uint4)(0,1,2,3);
    
    SORT_VECTOR(input1, UP)
    SORT_VECTOR(input2, DOWN)

    SWAP_VECTORS(input1, input2, dir)
    SORT_VECTOR(input1, dir)
    SORT_VECTOR(input2, dir)

    data[0] = input1;
    data[1] = input2;
}
