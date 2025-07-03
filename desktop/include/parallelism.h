#pragma once

#include <utils.h>
#include <mesh.h>
#include <aabb.h>

#ifdef USE_OPENCL

#if defined(MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

struct opencl {
    cl_program program;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
    cl_kernel kernel;
};

#define GPU_MODULE opencl
#define GPU_MEM cl_mem
#endif

void parallelism_init(struct GPU_MODULE* modul, const char* kernel_name, const char* path);
void parallelism_alloc_MDF(struct GPU_MODULE* modul, AABB* aabb, Mesh* mesh, int32_t texture, GPU_MEM* data, size_t _size);
void parallelism_invoke_MDF(struct GPU_MODULE* modul, GPU_MEM data, float** buffer, size_t data_size);
void parallelism_clean(struct GPU_MODULE* modul, GPU_MEM* data, size_t data_size);
void parallelism_destroy(struct GPU_MODULE* modul);
