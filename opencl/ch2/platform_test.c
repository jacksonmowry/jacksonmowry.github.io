#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

int main() {
    cl_platform_id* platforms;
    cl_uint num_platforms;
    cl_int i = -1;
    cl_int err = -1;

    char* ext_data;
    size_t ext_size;

    err = clGetPlatformIDs(1, NULL, &num_platforms);
    if (err < 0) {
        perror("Couldn't find any platforms");
        exit(1);
    }

    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    printf("We have %u platforms\n", num_platforms);

    for (i = 0; i < (cl_int)num_platforms; i++) {
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 0, NULL,
                                &ext_size);
        if (err < 0) {
            perror("Couldn't read extension data.");
            exit(1);
        }

        ext_data = (char*)malloc(ext_size);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, ext_size,
                          ext_data, NULL);
        printf("Platform %d supports extensions: %s\n", i, ext_data);

        free(ext_data);

        cl_device_id device_id;
        cl_uint num_devices;
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 1, &device_id,
                             &num_devices);
        char buf[256];
        size_t buf_len;
        err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 8, buf, &buf_len);

        buf[buf_len] = 0;
        puts(buf);
    }

    free(platforms);
    return 0;
}
