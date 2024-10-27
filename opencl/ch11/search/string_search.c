/* #define _CRT_SECURE_NO_WARNINGS */
#define PROGRAM_FILE "string_search.cl"
#define KERNEL_FUNC "string_search"
#define TEXT_FILE "kafka.txt"

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  /* Host/device data structures */
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_int err;

  /* Program/kernel data structures */
  cl_program program;
  FILE *program_handle;
  char *program_buffer, *program_log;
  size_t program_size, log_size;
  cl_kernel kernel;
  size_t offset = 0;
  size_t global_size, local_size;

  /* Data and buffers */
  char pattern[16] = "thatwithhavefrom";
  FILE *text_handle;
  char *text;
  size_t text_size;
  int chars_per_item;
  int result[4] = {0, 0, 0, 0};
  cl_mem text_buffer, result_buffer;

  err = clGetPlatformIDs(1, &platform, NULL);
  if (err < 0) {
    perror("Couldn't identify a platform");
    exit(1);
  }

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  if (err < 0) {
    perror("Couldn't access any devices");
    exit(1);
  }

  clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(global_size),
                  &global_size, NULL);
  clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(local_size),
                  &local_size, NULL);

  global_size *= local_size;

  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

  program_handle = fopen(PROGRAM_FILE, "r");
  fseek(program_handle, 0, SEEK_END);
  program_size = ftell(program_handle);
  fseek(program_handle, 0, SEEK_SET);
  program_buffer = calloc(program_size + 1, 1);
  fread(program_buffer, 1, program_size, program_handle);
  fclose(program_handle);

  text_handle = fopen(TEXT_FILE, "r");
  fseek(text_handle, 0, SEEK_END);
  text_size = ftell(text_handle) - 1;
  rewind(text_handle);
  text = (char *)calloc(text_size, sizeof(char));
  fread(text, sizeof(char), text_size, text_handle);
  fclose(text_handle);
  chars_per_item = text_size / global_size + 1;

  program = clCreateProgramWithSource(
      context, 1, (const char **)&program_buffer, &program_size, &err);
  free(program_buffer);

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  kernel = clCreateKernel(program, KERNEL_FUNC, &err);

  text_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               text_size, text, &err);

  result_buffer =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     sizeof(result), result, NULL);

  err = clSetKernelArg(kernel, 0, sizeof(pattern), pattern);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &text_buffer);
  err |= clSetKernelArg(kernel, 2, sizeof(chars_per_item), &chars_per_item);
  err |= clSetKernelArg(kernel, 3, sizeof(int), NULL);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &result_buffer);

  queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);

  err = clEnqueueNDRangeKernel(queue, kernel, 1, &offset, &global_size,
                               &local_size, 0, NULL, NULL);

  err = clEnqueueReadBuffer(queue, result_buffer, CL_TRUE, 0, sizeof(result),
                            &result, 0, NULL, NULL);

  printf("\nResults: \n");
  printf("Number of occurrences of 'that': %d\n", result[0]);
  printf("Number of occurrences of 'with': %d\n", result[1]);
  printf("Number of occurrences of 'have': %d\n", result[2]);
  printf("Number of occurrences of 'from': %d\n", result[3]);

  /* Deallocate resources */
  clReleaseMemObject(result_buffer);
  clReleaseMemObject(text_buffer);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(queue);
  clReleaseProgram(program);
  clReleaseContext(context);
  return 0;
}
