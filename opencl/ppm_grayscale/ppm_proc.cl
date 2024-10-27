__kernel void ppm_proc(__global uchar *r, __global uchar *g, __global uchar *b,
                       __global uchar *result) {
  int i = get_global_id(0);
  result[i] = r[i] * 0.299 + g[i] * 0.587 + b[i] * 0.114;
}
