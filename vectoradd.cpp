// Start compile.sh to compile, or enter:
// > g++ -c -Wall -I /home/christophe/tools/AMD-APP-SDK-v2.8-RC-lnx32/include vectoradd.cpp -o vectoradd.o
// > g++ vectoradd.o -o vectoradd -L /home/christophe/tools/AMD-APP-SDK-v2.8-RC-lnx32/lib/x86 -l OpenCL

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

int main(int argc, char *argv[]) {
    
    // Create the input vector.
    const int UNIT_SIZE = 64; // Divide work into units of 64.
    const int VECTOR_SIZE = UNIT_SIZE * 20;
    const int VECTOR_MEM_SIZE = VECTOR_SIZE * sizeof(int);
    
    int *a = (int*)malloc(VECTOR_MEM_SIZE);
    
    for (int i = 0; i < VECTOR_SIZE; i++) {
        
        a[i] = i + 1;
    }
    
    // Load kernel code into array source_str.
    FILE *fp;
    char *source_str;
    size_t source_size;
    
    fp = fopen("vector_add_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    
    // Get platform and device info.
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
    
    // Create OpenCL context.
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    
    // Create command queue.
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    
    // Create memory buffers on the device for each vector.
    cl_mem a_mem_obj = clCreateBuffer(
        context, CL_MEM_READ_ONLY, VECTOR_MEM_SIZE, NULL, &ret);
    cl_mem result_mem_obj = clCreateBuffer(
        context, CL_MEM_READ_ONLY, VECTOR_MEM_SIZE, NULL, &ret);
    
    // Copy the vector onto the buffer.
    ret = clEnqueueWriteBuffer(
        command_queue, a_mem_obj, CL_TRUE, 0, VECTOR_MEM_SIZE, a, 0, NULL, NULL);
    
    // Create the program from the kernel source.
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    
    // Build the program.
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    
    if (ret != CL_SUCCESS) {
        printf("%d: Kernel did not compile.", ret);
        exit(1);
    }
    
    // Create the kernel.
    cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
    
    // Set the arguments of the kernel.
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&result_mem_obj);
    
    // Execute the kernel.
    size_t global_item_size = VECTOR_SIZE;
    size_t local_item_size = UNIT_SIZE;
    
    ret = clEnqueueNDRangeKernel(
        command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    
    // Read the memory buffer result.
    int *result = (int *)malloc(VECTOR_MEM_SIZE);
    ret = clEnqueueReadBuffer(
        command_queue, result_mem_obj, CL_TRUE, 0, VECTOR_MEM_SIZE, result, 0, NULL, NULL);
    
    // Display result.
    int maximum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        
        if (result[i] > maximum) {
            
            printf("%5d %15d\n", result[i], a[i]);
            maximum = result[i];
        }
    }
    
    // Clean up.
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(result_mem_obj);
    
    ret = clReleaseCommandQueue(command_queue);
    
    ret = clReleaseContext(context);
    
    free(a);
    free(result);
        
    return 0;
}
