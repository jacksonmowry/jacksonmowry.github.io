#include <CL/cl_platform.h>
#include <stdarg.h>
#define PROGRAM_FILE "vec_reflect.cl"
#define KERNEL_FUNC "vec_reflect"

#include <CL/cl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

typedef struct {
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_int err;

  cl_program program;
  FILE *program_handle;
  char *program_buffer;
  char *program_log;
  size_t program_size;
  size_t log_size;
  cl_kernel kernel;
  size_t work_units_per_kernel;

  cl_mem *kernel_args;
  size_t num_kernel_args;
} cl_context_wrapper;

// `cl_init_kernel` creates all of the context/kernel objects needed for the
// provided function. Returns `true` on success, `false` on failure
bool cl_init_kernel(cl_context_wrapper *ccw, const char *program_file,
                    const char *kernel_func, cl_device_type device_type,
                    size_t num_kernel_args) {
  ccw->err = clGetPlatformIDs(1, &ccw->platform, NULL);
  if (ccw->err < 0) {
    perror("Couldn't find any platforms");
    return false;
  }

  ccw->err = clGetDeviceIDs(ccw->platform, device_type, 1, &ccw->device, NULL);
  if (ccw->err < 0) {
    perror("Couldn't find any devices");
    return false;
  }

  ccw->context = clCreateContext(NULL, 1, &ccw->device, NULL, NULL, &ccw->err);
  if (ccw->err < 0) {
    perror("Couldn't create a context");
    return false;
  }

  ccw->program_handle = fopen(program_file, "r");
  fseek(ccw->program_handle, 0, SEEK_END);
  ccw->program_size = ftell(ccw->program_handle);
  fseek(ccw->program_handle, 0, SEEK_SET);
  ccw->program_buffer = malloc(ccw->program_size + 1);
  fread(ccw->program_buffer, ccw->program_size, sizeof(char),
        ccw->program_handle);
  ccw->program_buffer[ccw->program_size] = 0;
  fclose(ccw->program_handle);

  ccw->program = clCreateProgramWithSource(ccw->context, 1,
                                           (const char **)&ccw->program_buffer,
                                           &ccw->program_size, &ccw->err);
  if (ccw->err < 0) {
    perror("Couldn't create the program");
    return false;
  }

  free(ccw->program_buffer);

  ccw->err = clBuildProgram(ccw->program, 0, NULL, NULL, NULL, NULL);
  if (ccw->err < 0) {
    perror("Couldn't build program");

    clGetProgramBuildInfo(ccw->program, ccw->device, CL_PROGRAM_BUILD_LOG, 0,
                          NULL, &ccw->log_size);
    ccw->program_log = malloc(ccw->log_size + 1);
    ccw->program_log[ccw->log_size] = '\0';
    clGetProgramBuildInfo(ccw->program, ccw->device, CL_PROGRAM_BUILD_LOG,
                          ccw->log_size + 1, ccw->program_log, NULL);
    printf("%s\n", ccw->program_log);
    free(ccw->program_log);

    return false;
  }

  ccw->kernel = clCreateKernel(ccw->program, kernel_func, &ccw->err);
  if (ccw->err < 0) {
    perror("Couldn't create kernel");
    return false;
  }

  ccw->queue = clCreateCommandQueueWithProperties(ccw->context, ccw->device,
                                                  NULL, &ccw->err);
  if (ccw->err < 0) {
    perror("Couldn't create command queue");
    return false;
  }

  ccw->kernel_args = calloc(num_kernel_args, sizeof(cl_mem));
  ccw->num_kernel_args = num_kernel_args;

  return true;
}

void cl_free_kernel(cl_context_wrapper *ccw) {
  clReleaseKernel(ccw->kernel);
  clReleaseCommandQueue(ccw->queue);
  clReleaseProgram(ccw->program);
  clReleaseContext(ccw->context);

  for (size_t i = 0; i < ccw->num_kernel_args; i++) {
    clReleaseMemObject(ccw->kernel_args[i]);
  }

  free(ccw->kernel_args);
}

bool cl_create_and_bind(cl_context_wrapper *ccw, cl_mem_flags flags,
                        size_t size, void *host_ptr, size_t kernel_arg_pos) {
  ccw->kernel_args[kernel_arg_pos] =
      clCreateBuffer(ccw->context, flags, size, host_ptr, &ccw->err);
  if (ccw->err < 0) {
    perror("Unable to create CL Buffer");
    return false;
  }

  ccw->err = clSetKernelArg(ccw->kernel, kernel_arg_pos, sizeof(cl_mem),
                            &ccw->kernel_args[kernel_arg_pos]);
  if (ccw->err < 0) {
    perror("Unable to set kernel arg");
    return false;
  }

  return true;
}

int main() {
  cl_context_wrapper ccw;
  cl_init_kernel(&ccw, PROGRAM_FILE, KERNEL_FUNC, CL_DEVICE_TYPE_DEFAULT, 3);

  float reflect[4];
  cl_mem reflect_buffer;
  float x[4] = {1, 2, 3, 4};
  float u[4] = {0, 5, 0, 0};

  bool bound =
      cl_create_and_bind(&ccw, CL_MEM_WRITE_ONLY, 4 * sizeof(float), NULL, 2);
  cl_uint err = clSetKernelArg(ccw.kernel, 0, sizeof(x), x);
  err |= clSetKernelArg(ccw.kernel, 1, sizeof(u), x);

  clEnqueueReadBuffer(ccw.queue, reflect_buffer, CL_TRUE, 0, sizeof(reflect),
                      reflect, 0, NULL, NULL);
  printf("\nResult: %f %f %f %f\n", reflect[0], reflect[1], reflect[2],
         reflect[3]);

  cl_free_kernel(&ccw);
}
