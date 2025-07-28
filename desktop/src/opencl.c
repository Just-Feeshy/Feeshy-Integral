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
    SDL_RWops* file = NULL;
    cl_program program = NULL;
    uint8_t* file_buffer = NULL;

    file = SDL_RWFromFile(path, "rb");
    if (file == NULL) {
        SDL_Log("Failed to open file: %s\n", path);
        return NULL;
    }

    int64_t file_size = SDL_RWsize(file);
    file_buffer = (uint8_t*)SDL_malloc(file_size);

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
        SDL_free(file_buffer);
        file_buffer = NULL;
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
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_kernel kernel = NULL;
    cl_int err;
    cl_program program = NULL;

    // Initialize module to zero
    memset(modul, 0, sizeof(struct GPU_MODULE));

    best_device_opencl(&platform_id, &device_id);
    if (!platform_id || !device_id) {
        SDL_Log("[OpenCL] No suitable OpenCL device found\n");
        return;
    }
    
    // Debug: Print device info
    char device_name[256];
    char vendor_name[256];
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, NULL);
    SDL_Log("[OpenCL] Using device: %s from %s\n", device_name, vendor_name);

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to create context: %d\n", err);
        return;
    }

    queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (err != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to create command queue: %d\n", err);
        clReleaseContext(context);
        return;
    }

    program = opencl_platform_make_source(context, path);
    if (!program) {
        SDL_Log("[OpenCL] Failed to create program from source\n");
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    err = clBuildProgram(program, 1, &device_id, "-cl-std=CL1.2 -D__OPENCL__ -Iinclude", NULL, NULL);
    if(err == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size + 1);
        if (log) {
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
            log[log_size] = '\0';
            SDL_Log("[OpenCL] Build log:\n%s\n", log);
            free(log);
        }
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    } else if(err != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to build program: %d\n", err);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    kernel = clCreateKernel(program, kernel_name, &err);
    if (err != CL_SUCCESS) {
        SDL_Log("[OpenCL] Failed to create kernel '%s': %d\n", kernel_name, err);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

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
    
    // Debug: Print mesh and AABB info
    SDL_Log("[DFAO Debug] Mesh: vertices=%d, triangles=%d", mesh->vertex_count, mesh->triangle_count);  
    SDL_Log("[DFAO Debug] AABB: min=(%.3f,%.3f,%.3f) max=(%.3f,%.3f,%.3f)", 
            aabb->min[0], aabb->min[1], aabb->min[2],
            aabb->max[0], aabb->max[1], aabb->max[2]);
    SDL_Log("[DFAO Debug] Grid size: %dx%dx%d = %zu", UNIFORM_GRID_X, UNIFORM_GRID_Y, UNIFORM_GRID_Z, __size);

    cl_mem aabb_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(AABB), aabb, &err);
    if (err != CL_SUCCESS) {
        SDL_Log("[DFAO Debug] Failed to create AABB buffer: %d", err);
        return;
    }

    cl_mem vertex_buffer = clCreateBuffer(modul->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * mesh->vertex_count * 3, mesh->vertices, &err);
    if (err != CL_SUCCESS) {
        SDL_Log("[DFAO Debug] Failed to create vertex buffer: %d", err);
        return;
    }

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
    
    SDL_Log("[DFAO Debug] Executing kernel with global size: %zux%zux%zu", global_size[0], global_size[1], global_size[2]);

    (*buffer) = malloc(sizeof(float) * data_size);
    if (!*buffer) {
        SDL_Log("[DFAO Debug] Failed to allocate output buffer of size %zu", data_size);
        return;
    }

    cl_int err = clEnqueueNDRangeKernel(modul->queue, modul->kernel, 3, NULL, global_size, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        SDL_Log("[DFAO Debug] Failed to enqueue kernel: %d", err);
        free(*buffer);
        *buffer = NULL;
        return;
    }

    err = clEnqueueReadBuffer(modul->queue, data, CL_TRUE, 0, sizeof(float) * data_size, *buffer, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        SDL_Log("[DFAO Debug] Failed to read buffer: %d", err);
        free(*buffer);
        *buffer = NULL;
        return;
    }

    clFinish(modul->queue);
    
    // Debug: Sample first few values to check for corruption
    SDL_Log("[DFAO Debug] First 10 distance values: %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f", 
            (*buffer)[0], (*buffer)[1], (*buffer)[2], (*buffer)[3], (*buffer)[4],
            (*buffer)[5], (*buffer)[6], (*buffer)[7], (*buffer)[8], (*buffer)[9]);
            
    // Check for NaN/Inf values
    int nan_count = 0, inf_count = 0, negative_count = 0;
    for (size_t i = 0; i < (data_size > 1000 ? 1000 : data_size); i++) {
        if (isnan((*buffer)[i])) nan_count++;
        if (isinf((*buffer)[i])) inf_count++;
        if ((*buffer)[i] < 0.0f) negative_count++;
    }
    SDL_Log("[DFAO Debug] Data validation (first 1000): NaN=%d, Inf=%d, Negative=%d", nan_count, inf_count, negative_count);
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
    if (!modul) return;
    
    if (modul->kernel) {
        clReleaseKernel(modul->kernel);
        modul->kernel = NULL;
    }
    if (modul->queue) {
        clReleaseCommandQueue(modul->queue);
        modul->queue = NULL;
    }
    if (modul->context) {
        clReleaseContext(modul->context);
        modul->context = NULL;
    }
    if (modul->program) {
        clReleaseProgram(modul->program);
        modul->program = NULL;
    }
}
