#define PROGRAM_FILE "matvec.cl"
#define KERNEL_FUNC "matvec_mult"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <CL/cl.h>

int main() {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_int i;
    cl_int err;

    cl_program program;
    FILE* program_handle;
    char* program_buffer;
    char* program_log;
    size_t program_size;
    size_t log_size;
    cl_kernel kernel;
    size_t work_units_per_kernel;

    float mat[16];
    float vec[4];
    float result[4];
    float correct[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    cl_mem mat_buff;
    cl_mem vec_buff;
    cl_mem res_buff;

    for (i = 0; i < 16; i++) {
        mat[i] = i * 2.0f;
    }

    for (i = 0; i < 4; i++) {
        vec[i] = i * 3.0f;
        correct[0] += mat[i] * vec[i];
        correct[1] += mat[i + 4] * vec[i];
        correct[2] += mat[i + 8] * vec[i];
        correct[3] += mat[i + 12] * vec[i];
    }

    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0) {
        perror("Couldn't find any platforms");
        exit(1);
    }
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    if (err < 0) {
        perror("Couldn't find any devices");
        exit(1);
    }
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err < 0) {
        perror("Couldn't create a context");
        exit(1);
    }

    program_handle = fopen(PROGRAM_FILE, "r");
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    fseek(program_handle, 0, SEEK_SET);
    program_buffer = (char*)malloc(program_size + 1);
    program_buffer[program_size] = 0;
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    program = clCreateProgramWithSource(
        context, 1, (const char**)&program_buffer, &program_size, &err);
    if (err < 0) {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0) {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL,
                              &log_size);
        program_log = (char*)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    kernel = clCreateKernel(program, KERNEL_FUNC, &err);
    if (err < 0) {
        perror("Couldn't create the kernel");
        exit(1);
    }
    queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    if (err < 0) {
        perror("Couldn't create the command queue");
        exit(1);
    }

    mat_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              sizeof(float) * 16, mat, &err);
    vec_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              sizeof(float) * 4, vec, &err);
    res_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                              sizeof(float) * 4, result, &err);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &mat_buff);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &vec_buff);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &res_buff);

    work_units_per_kernel = 4;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL,
                           0, NULL, NULL);

    clEnqueueReadBuffer(queue, res_buff, CL_TRUE, 0, sizeof(float) * 4, result,
                        0, NULL, NULL);
    if ((result[0] == correct[0]) && (result[1] == correct[1]) &&
        (result[2] == correct[2]) && (result[3] == correct[3])) {
        printf("matvec mul correct\n");
    } else {
        printf("%f %f %f %f\n%f %f %f %f\n", result[0], result[1], result[2],
               result[3], correct[0], correct[1], correct[2], correct[3]);

        printf("matvec mul incorrect\n");
    }

    clReleaseMemObject(mat_buff);
    clReleaseMemObject(vec_buff);
    clReleaseMemObject(res_buff);

    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    return 0;
}
