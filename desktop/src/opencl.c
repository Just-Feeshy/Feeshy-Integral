#include <parallelism.h>
#include <config.h>
#include <assert.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_rwops.h>
#else
#include <SDL_log.h>
#include <SDL_rwops.h>
#endif

static struct opencl unigrid_opencl = {0};

static cl_program opencl_platform_make_source(cl_context context, const char* path) {
    SDL_RWops* file = SDL_RWFromFile(path, "rb");
    cl_program program = NULL;

    if (file == NULL) {
        SDL_Log("Failed to open file: %s\n", path);
        return NULL;
    }

    int64_t file_size = SDL_RWsize(file);
    uint8_t* file_buffer = (uint8_t*)SDL_malloc(file_size);

    if (file_buffer == NULL) {
        SDL_Log("Failed to allocate memory for file: %s\n", path);
        goto cleanup_platform;
    }

    if(SDL_RWread(file, file_buffer, 1, file_size) != file_size) {
        SDL_Log("Failed to read file: %s\n", path);
        goto cleanup_platform;
    }

    cl_int err;
    program = clCreateProgramWithSource(context, 1, (const char**)&file_buffer, (const size_t*)&file_size, &err);
    assert(err == CL_SUCCESS);

cleanup_platform:
    if(file) {
        SDL_RWclose(file);
    }

    if(file_buffer) {
        free(file_buffer);
    }

    return program;
}


static void best_device_opencl(cl_platform_id* platform_id, cl_device_id* device_id) {
    cl_uint num_platforms = 0;

    cl_int result = clGetPlatformIDs(0, NULL, &num_platforms);
    if (result != CL_SUCCESS || num_platforms == 0) {
        SDL_Log("[OpenCL] No platforms found");
        *platform_id = NULL;
        *device_id = NULL;
        return;
    }

    cl_platform_id* platforms = mem_alloca(num_platforms * sizeof(cl_platform_id));
    result = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (result != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to get platform IDs");
        *platform_id = NULL;
        *device_id = NULL;
        return;
    }

    for (cl_uint i = 0; i < num_platforms; i++) {
        cl_device_id device;
        if (clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device, NULL) == CL_SUCCESS) {
            *platform_id = platforms[i];
            *device_id = device;

            char name[DEFAULT_BUFFER_SIZE], vendor[DEFAULT_BUFFER_SIZE];
            clGetDeviceInfo(device, CL_DEVICE_NAME, DEFAULT_BUFFER_SIZE, name, NULL);
            clGetDeviceInfo(device, CL_DEVICE_VENDOR, DEFAULT_BUFFER_SIZE, vendor, NULL);
            SDL_Log("[OpenCL] Using GPU device: %s from %s", name, vendor);
            return;
        }
    }

    for (cl_uint i = 0; i < num_platforms; i++) {
        cl_device_id device;
        if (clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 1, &device, NULL) == CL_SUCCESS) {
            *platform_id = platforms[i];
            *device_id = device;

            char name[DEFAULT_BUFFER_SIZE], vendor[DEFAULT_BUFFER_SIZE];
            clGetDeviceInfo(device, CL_DEVICE_NAME, DEFAULT_BUFFER_SIZE, name, NULL);
            clGetDeviceInfo(device, CL_DEVICE_VENDOR, DEFAULT_BUFFER_SIZE, vendor, NULL);
            SDL_Log("[OpenCL] Using CPU device: %s from %s", name, vendor);
            return;
        }
    }

    SDL_Log("[OpenCL] No suitable device found");
    *platform_id = NULL;
    *device_id = NULL;
}

void parallelism_init(struct GPU_MODULE* modul, const char* kernel_name, const char* path) {
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_context context;
    cl_command_queue queue;
    cl_kernel kernel;
    cl_int err;
    cl_program program;

    best_device_opencl(&platform_id, &device_id);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    assert(err == CL_SUCCESS);

    queue = clCreateCommandQueue(context, device_id, 0, &err);
    assert(err == CL_SUCCESS);

    program = opencl_platform_make_source(context, path);
    assert(program != NULL);

    err = clBuildProgram(program, 1, &device_id, "-cl-std=CL1.2 -D__OPENCL__ -Iinclude", NULL, NULL);
    if(err == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, DEFAULT_BUFFER_SIZE, NULL, &log_size);
        char* log = mem_alloca(log_size);
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        SDL_Log("[OpenCL] Build log:\n%s\n", log);
    }else if(err != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to build program\n");
    }

    kernel = clCreateKernel(program, kernel_name, &err);
    assert(err == CL_SUCCESS);

    modul->context = context;
    modul->queue = queue;
    modul->kernel = kernel;
    modul->device_id = device_id;
    modul->program = program;
}

void parallelism_alloc_MDF(struct GPU_MODULE* modul, AABB* aabb, Mesh* mesh, int32_t texture, GPU_MEM* data, size_t __size) {
    size_t global_size[3] = {UNIFORM_GRID_X, UNIFORM_GRID_Y, UNIFORM_GRID_Z};
    size_t local_size[3] = {8, 4, 2};

    cl_int err;

    cl_mem aabb_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(AABB), aabb, &err);
    assert(err == CL_SUCCESS);

    cl_mem vertex_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * mesh->vertex_count * 3, mesh->vertices, &err);
    assert(err == CL_SUCCESS);

    cl_mem normal_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * mesh->vertex_count * 3, mesh->normals, &err);
    assert(err == CL_SUCCESS);

    cl_mem index_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned short) * mesh->triangle_count * 3, mesh->indices, &err);

    cl_mem output_buffer = clCreateBuffer(modul->context, CL_MEM_WRITE_ONLY, sizeof(float) * __size, NULL, &err);
    assert(err == CL_SUCCESS);


    err |= clSetKernelArg(modul->kernel, 0, sizeof(cl_mem), &output_buffer);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 1, sizeof(cl_mem), &vertex_buffer);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 2, sizeof(cl_mem), &normal_buffer);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 3, sizeof(cl_mem), &index_buffer);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 4, sizeof(cl_int), &mesh->vertex_count);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 5, sizeof(cl_int), &mesh->triangle_count);
    assert(err == CL_SUCCESS);

    err |= clSetKernelArg(modul->kernel, 6, sizeof(cl_mem), &aabb_buffer);
    assert(err == CL_SUCCESS);

    (*data) = output_buffer;
    data[1] = vertex_buffer;
    data[2] = normal_buffer;
    data[3] = index_buffer;
    data[4] = aabb_buffer;
}

void parallelism_invoke_MDF(struct GPU_MODULE* modul, GPU_MEM data, float** buffer, size_t data_size) {
    size_t global_size[3] = {UNIFORM_GRID_X, UNIFORM_GRID_Y, UNIFORM_GRID_Z};

    (*buffer) = malloc(sizeof(float) * data_size);
    assert(*buffer != NULL);

    cl_int err = clEnqueueNDRangeKernel(modul->queue, modul->kernel, 3, NULL, global_size, NULL, 0, NULL, NULL);
    assert(err == CL_SUCCESS);

    err = clEnqueueReadBuffer(modul->queue, data, CL_TRUE, 0, sizeof(float) * data_size, *buffer, 0, NULL, NULL);
    assert(err == CL_SUCCESS);

    clFinish(modul->queue);
}

void parallelism_clean(struct GPU_MODULE* modul, GPU_MEM* data, size_t data_size) {
    if(!modul) {
        return;
    }

    for(size_t i=0; i<data_size; i++) {
        clReleaseMemObject((cl_mem)data[i]);
    }
    free(data);
}

void parallelism_destroy(struct GPU_MODULE* modul) {
    clReleaseKernel(modul->kernel);
    clReleaseCommandQueue(modul->queue);
    clReleaseContext(modul->context);
    clReleaseProgram(modul->program);
}
