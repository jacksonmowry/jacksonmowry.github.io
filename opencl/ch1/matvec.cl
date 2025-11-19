#define CL_TAGET_OPENCL_VERSION 301
__kernel void matvec_mult(__global float4* matrix,
                          __global float4* vector,
                          __global float* result) {
    int i = get_global_id(0);
    printf("Hi from thread %d\n", i);
    result[i] = dot(matrix[i], vector[0]);
}
