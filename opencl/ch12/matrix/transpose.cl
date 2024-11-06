__kernel void transpose(__global float4* g_mat, __local float4* l_mat, uint size) {
    __global float4* src;
    __global float4* dst;

    int col = get_global_id(0);
    int row = 0;
    while (col >= size) {
        col -= size--;
        row++;
    }
    col += row;
    size += row;

    src = g_mat + row * size * 4 + col;
    l_mat += get_local_id(0)*8;
    l_mat[0] = src[0];
    l_mat[1] = src[size];
    l_mat[2] = src[size*2];
    l_mat[3] = src[size*3];

    if (row == col) {
        src[0] = (float4)(l_mat[0].x, l_mat[1].x,
                          l_mat[2].x, l_mat[3].x);
        src[size] = (float4)(l_mat[0].y, l_mat[1].y,
                          l_mat[2].y, l_mat[3].y);
        src[size*2] = (float4)(l_mat[0].z, l_mat[1].z,
                          l_mat[2].z, l_mat[3].z);
        src[size*3] = (float4)(l_mat[0].w, l_mat[1].w,
                          l_mat[2].w, l_mat[3].w);
    } else {
        dst = g_mat + col * size * 4 + row;
        l_mat[4] = dst[0];
        l_mat[5] = dst[size];
        l_mat[6] = dst[size*2];
        l_mat[7] = dst[size*3];

        dst[0] = (float4)(l_mat[0].x, l_mat[1].x,
                          l_mat[2].x, l_mat[3].x);
        dst[size] = (float4)(l_mat[0].y, l_mat[1].y,
                          l_mat[2].y, l_mat[3].y);
        dst[size*2] = (float4)(l_mat[0].z, l_mat[1].z,
                          l_mat[2].z, l_mat[3].z);
        dst[size*3] = (float4)(l_mat[0].w, l_mat[1].w,
                          l_mat[2].w, l_mat[3].w);

        src[0] = (float4)(l_mat[4].x, l_mat[5].x,
                          l_mat[6].x, l_mat[7].x);
        src[size] = (float4)(l_mat[4].y, l_mat[5].y,
                          l_mat[6].y, l_mat[7].y);
        src[size*2] = (float4)(l_mat[4].z, l_mat[5].z,
                          l_mat[6].z, l_mat[7].z);
        src[size*3] = (float4)(l_mat[4].w, l_mat[5].w,
                          l_mat[6].w, l_mat[7].w);
    }
}
