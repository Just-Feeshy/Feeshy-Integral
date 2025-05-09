#define CGLTF_IMPLEMENTATION

#include <model.h>
#include <cglm/vec3.h>
#include <opengl.h>
#include <cgltf.h>

#ifdef EMSCRIPTEN
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_log.h>
#else
#include <SDL_rwops.h>
#include <SDL_stdinc.h>
#include <SDL_log.h>
#endif

#define MAX_MESH_VERTEX_BUFFERS 9

// Very simple 3D vector functions

static void vec3_min(vec3* result, vec3 a, vec3 b) {
    *result[0] = a[0] < b[0] ? a[0] : b[0];
    *result[1] = a[1] < b[1] ? a[1] : b[1];
    *result[2] = a[2] < b[2] ? a[2] : b[2];
}

static void vec3_max(vec3* result, vec3 a, vec3 b) {
    *result[0] = a[0] > b[0] ? a[0] : b[0];
    *result[1] = a[1] > b[1] ? a[1] : b[1];
    *result[2] = a[2] > b[2] ? a[2] : b[2];
}

// Someone in my calc class saw this function, and no joke,
// called this "The Oppenheimer Strat"
// Thank you I guess?? It's just using "gotos" for better memory management.
static cgltf_result load_gltf_callback(
        const struct cgltf_memory_options* mem_options,
        const struct cgltf_file_options* file_options,
        const char* path,
        cgltf_size* size,
        void** data) {
    cgltf_result result = cgltf_result_success;

    // Open the file using SDL_RWops
    SDL_RWops* file = SDL_RWFromFile(path, "rb");
    if (file == NULL) {
        result = cgltf_result_io_error;
        goto cleanup_callback;
    }

    // Get the file size and allocate memory
    int64_t file_size = SDL_RWsize(file);
    uint8_t* file_buffer = (uint8_t*)SDL_malloc(file_size);
    if (file_buffer == NULL) {
        result = cgltf_result_io_error;
        goto cleanup_callback;
    }

    if(SDL_RWread(file, file_buffer, 1, file_size) != file_size) {
        result = cgltf_result_io_error;
        goto cleanup_callback;
    }

    *size = file_size;
    *data = file_buffer;

cleanup_callback:
    if (file) {
        SDL_RWclose(file);
    }

    if (result != cgltf_result_success) {
        free(file_buffer);
    }

    return result;
}

static void free_gltf_callback(
        const struct cgltf_memory_options* mem_options,
        const struct cgltf_file_options* file_options,
        void* data) {
    free(data); // Yea.. pretty much.
}

// Inspired by the glTF loader from Raylib
// Credits go to raysan5
Model load_model_gltf(const char* path) {
    /*********************************************************************************************

        Function implemented by Wilhem Barbier(@wbrbr), with modifications by Tyler Bezera(@gamerfiend)
        Transform handling implemented by Paul Melis (@paulmelis).
        Reviewed by Ramon Santamaria (@raysan5)

        FEATURES:
          - Supports .gltf and .glb files
          - Supports embedded (base64) or external textures
          - Supports PBR metallic/roughness flow, loads material textures, values and colors
                     PBR specular/glossiness flow and extended texture flows not supported
          - Supports multiple meshes per model (every primitives is loaded as a separate mesh)
          - Supports basic animations
          - Transforms, including parent-child relations, are applied on the mesh data, but the
            hierarchy is not kept (as it can't be represented).
          - Mesh instances in the glTF file (i.e. same mesh linked from multiple nodes)
            are turned into separate raylib Meshes.

        RESTRICTIONS:
          - Only triangle meshes supported
          - Vertex attribute types and formats supported:
              > Vertices (position): vec3: float
              > Normals: vec3: float
              > Texcoords: vec2: float
              > Colors: vec4: u8, u16, f32 (normalized)
              > Indices: u16, u32 (truncated to u16)
          - Scenes defined in the glTF file are ignored. All nodes in the file
            are used.

    ***********************************************************************************************/

    // Macro to simplify attributes loading code
    #define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr) LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, srcType)

    #define LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, dstType) \
    { \
        int n = 0; \
        srcType *buffer = (srcType *)accesor->buffer_view->buffer->data + accesor->buffer_view->offset/sizeof(srcType) + accesor->offset/sizeof(srcType); \
        for (unsigned int k = 0; k < accesor->count; k++) \
        {\
            for (int l = 0; l < numComp; l++) \
            {\
                dstPtr[numComp*k + l] = (dstType)buffer[n + l];\
            }\
            n += (int)(accesor->stride/sizeof(srcType));\
        }\
    }

    Model model = {0};
    uint64_t data_size = 0;

    SDL_RWops* file = SDL_RWFromFile(path, "rb");
    if (file == NULL) {
        goto cleanup_model;
    }

    int64_t file_size = SDL_RWsize(file);
    uint8_t* file_buffer = (uint8_t*)SDL_malloc(file_size);

    if (file_buffer == NULL) {
        goto cleanup_model;
    }

    if(SDL_RWread(file, file_buffer, 1, file_size) != file_size) {
        goto cleanup_model;
    }

    SDL_Log("Successfully read %s\n", path);

    cgltf_options options = {0};
    options.file.read = load_gltf_callback;
    options.file.release = free_gltf_callback;
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&options, file_buffer, file_size, &data);

    if(result == cgltf_result_success) {
        switch(data->file_type) {
            case cgltf_file_type_glb:
                SDL_Log("MODEL: [%s] Model basic data (glb) loaded sucessfully\n", path);
                break;
            case cgltf_file_type_gltf:
                SDL_Log("MODEL: [%s] Model basic data (gltf) loaded sucessfully\n", path);
                break;
            default:
                SDL_Log("MODEL: [%s] Model format not supported\n", path);
                break;
        }

        // Best to keep track of the data sizes, even if they are irrelevant
        SDL_Log("MODEL: [%s] Model has %zu meshes\n", path, data->meshes_count);
        SDL_Log("MODEL: [%s] Model has %zu materials\n", path, data->materials_count);
        SDL_Log("MODEL: [%s] Model has %zu buffer\n", path, data->buffers_count);
        SDL_Log("MODEL: [%s] Model has %zu images\n", path, data->images_count);
        SDL_Log("MODEL: [%s] Model has %zu textures\n", path, data->textures_count);

        // Load the model data
        result = cgltf_load_buffers(&options, data, path);
        if(result != cgltf_result_success) {
            SDL_Log("MODEL: [%s] Model buffers failed to load\n", path);
        }

        uint32_t primitive_count = 0;

        for(uint32_t i=0; i<data->nodes_count; i++) {
            cgltf_node* node = &data->nodes[i];
            cgltf_mesh* mesh = node->mesh;

            if(!mesh) {
                continue;
            }

            for(uint32_t j=0; j<mesh->primitives_count; j++) {
                if(mesh->primitives[j].type == cgltf_primitive_type_triangles) {
                    primitive_count++;
                }
            }
        }

        SDL_Log("MODEL: [%s] Model has %d primitives (triangles only)\n", path, primitive_count);

        model.mesh_count = primitive_count;
        model.meshes = (Mesh*)calloc(primitive_count, sizeof(Mesh));

        // If needed, we can put a materials function here.

        uint32_t mesh_index = 0;
        for(uint32_t i=0; i<data->nodes_count; i++) {
            cgltf_node* node = &data->nodes[i];
            cgltf_mesh* mesh = node->mesh;

            if(!mesh) {
                continue;
            }

            cgltf_float world_transform[16];
            cgltf_node_transform_world(node, world_transform);

            mat4 world_matrix;
            memcpy(world_matrix, world_transform, sizeof(float) * 16);

            mat4 normal_matrix;
            glm_mat4_inv(world_matrix, normal_matrix);
            glm_mat4_transpose(normal_matrix);

            for(uint32_t j=0; j<mesh->primitives_count; j++) {
                if(mesh->primitives[j].type != cgltf_primitive_type_triangles) {
                    continue;
                }

                for(uint32_t k=0; k<mesh->primitives[j].attributes_count; k++) {

                    // Probably should have made this a switch statement
                    // but I don't care.
                    //
                    // Vertices
                    if(mesh->primitives[j].attributes[k].type == cgltf_attribute_type_position) {
                            cgltf_accessor* attribute = mesh->primitives[j].attributes[k].data;
                        if(attribute->type == cgltf_type_vec3
                        && attribute->component_type == cgltf_component_type_r_32f) {
                            model.meshes[mesh_index].vertex_count = attribute->count;
                            model.meshes[mesh_index].vertices = (float*)malloc(sizeof(float) * attribute->count * 3);

                            LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[mesh_index].vertices);

                            float* vertices = model.meshes[mesh_index].vertices;
                            for(uint32_t l=0; l<attribute->count; l++) {
                                vec3 v = { vertices[3*k], vertices[3*k+1], vertices[3*k+2] };
                                vec3 vt;

                                glm_mat4_mulv3(world_matrix, v, 1.0f, vt);
                                vertices[3*k]   = vt[0];
                                vertices[3*k+1] = vt[1];
                                vertices[3*k+2] = vt[2];
                            }
                        }else {
                            SDL_Log("MODEL: [%s] Model has unsupported attribute type\n", path);
                        }

                    // Normals
                    }else if(mesh->primitives[j].attributes[k].type == cgltf_attribute_type_normal) {
                        cgltf_accessor* attribute = mesh->primitives[j].attributes[k].data;

                        if(attribute->type == cgltf_type_vec3 && attribute->component_type == cgltf_component_type_r_32f) {
                            model.meshes[mesh_index].normals = (float*)malloc(sizeof(float) * attribute->count * 3);

                            LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[mesh_index].normals);

                            float* normals = model.meshes[mesh_index].normals;
                            for(uint32_t l=0; l<attribute->count; l++) {
                                vec3 n = { normals[3*k], normals[3*k+1], normals[3*k+2] };
                                vec3 nt;

                                glm_mat4_mulv3(normal_matrix, n, 0.0f, nt);
                                normals[3*k]   = nt[0];
                                normals[3*k+1] = nt[1];
                                normals[3*k+2] = nt[2];
                            }
                        }else {
                            SDL_Log("MODEL: [%s] Model has unsupported attribute type\n", path);
                        }

                    // Tangents
                    }else if(mesh->primitives[j].attributes[k].type == cgltf_attribute_type_tangent) {
                        cgltf_accessor* attribute = mesh->primitives[j].attributes[k].data;

                        if(attribute->type == cgltf_type_vec4 && attribute->component_type == cgltf_component_type_r_32f) {
                            model.meshes[mesh_index].tangents = (float*)malloc(sizeof(float) * attribute->count * 4);

                            LOAD_ATTRIBUTE(attribute, 4, float, model.meshes[mesh_index].tangents);

                            float* tangents = model.meshes[mesh_index].tangents;
                            for(uint32_t l=0; l<attribute->count; l++) {
                                vec3 t = { tangents[3*k], tangents[3*k+1], tangents[3*k+2] };
                                vec3 tt;

                                glm_mat4_mulv3(world_matrix, t, 0.0f, tt);

                                tangents[3*k]   = tt[0];
                                tangents[3*k+1] = tt[1];
                                tangents[3*k+2] = tt[2];
                            }
                        }else {
                            SDL_Log("MODEL: [%s] Model has unsupported attribute type\n", path);
                        }
                    }

                    // Texcoords
                    else if(mesh->primitives[j].attributes[k].type == cgltf_attribute_type_texcoord) {

                        // Support up to 2 texture coordinates attributes
                        float *texcoordPtr = NULL;
                        cgltf_accessor *attribute = mesh->primitives[j].attributes[k].data;

                        if(attribute->type == cgltf_type_vec2) {
                            if(attribute->component_type == cgltf_component_type_r_32f) {
                                texcoordPtr = (float*)malloc(sizeof(float) * attribute->count * 2);
                                LOAD_ATTRIBUTE(attribute, 2, float, texcoordPtr);
                            }else if(attribute->component_type == cgltf_component_type_r_8u) {
                                texcoordPtr = (float*)malloc(sizeof(float) * attribute->count * 2);

                                uint8_t* temp = (uint8_t*)malloc(sizeof(uint8_t) * attribute->count * 2);
                                LOAD_ATTRIBUTE(attribute, 2, uint8_t, temp);

                                for(uint32_t l=0; l<attribute->count * 2; l++) {
                                    texcoordPtr[l] = (float)temp[l] / 255.0f;
                                }

                                free(temp);
                            }else if(attribute->component_type == cgltf_component_type_r_16u) {
                                texcoordPtr = (float*)malloc(sizeof(float) * attribute->count * 2);

                                uint16_t* temp = (uint16_t*)malloc(sizeof(uint16_t) * attribute->count * 2);
                                LOAD_ATTRIBUTE(attribute, 2, uint16_t, temp);

                                for(uint32_t l=0; l<attribute->count * 2; l++) {
                                    texcoordPtr[l] = (float)temp[l] / 65535.0f;
                                }

                                free(temp);
                            }else {
                                SDL_Log("MODEL: [%s] Texcoord attribute has unsupported component type\n", path);
                            }
                        }else {
                            SDL_Log("MODEL: [%s] Texcoord attribute has unsupported type\n", path);
                        }

                        uint32_t index = mesh->primitives[j].attributes[k].index;
                        if(index == 0) {
                            model.meshes[mesh_index].texcoords = texcoordPtr;
                        }else if(index == 1) {
                            model.meshes[mesh_index].texcoords2 = texcoordPtr;
                        }else {
                            SDL_Log("MODEL: [%s] Model has unsupported texcoord index\n", path);
                            if(texcoordPtr) {
                                free(texcoordPtr);
                            }
                        }
                    }
                }


                // Indices
                if(mesh->primitives[j].indices != NULL && mesh->primitives[j].indices->buffer_view != NULL) {
                    cgltf_accessor* attribute = mesh->primitives[j].indices;
                    model.meshes[mesh_index].triangle_count = attribute->count / 3;

                    if(attribute->component_type == cgltf_component_type_r_16u) {
                        model.meshes[mesh_index].indices = (uint16_t*)malloc(attribute->count * sizeof(uint16_t));
                        LOAD_ATTRIBUTE(attribute, 1, uint16_t, model.meshes[mesh_index].indices);
                    }else if(attribute->component_type == cgltf_component_type_r_8u) {
                        model.meshes[mesh_index].indices = (uint16_t*)malloc(attribute->count * sizeof(uint16_t));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, uint8_t, model.meshes[mesh_index].indices, uint16_t);
                    }else if(attribute->component_type == cgltf_component_type_r_32u) {
                        model.meshes[mesh_index].indices = (uint16_t*)malloc(attribute->count * sizeof(uint16_t));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, uint32_t, model.meshes[mesh_index].indices, uint16_t);
                    }else {
                        SDL_Log("MODEL: [%s] Model has unsupported index type\n", path);
                    }
                }else {
                    model.meshes[mesh_index].triangle_count = model.meshes[mesh_index].vertex_count / 3;
                }

                mesh_index++;
            }
        }

        // Free the data
        cgltf_free(data);
    }else {
        SDL_Log("MODEL: [%s] Model failed to load\n", path);
    }

cleanup_model:
    #undef LOAD_ATTRIBUTE
    #undef LOAD_ATTRIBUTE_CAST

    if (file) {
        SDL_RWclose(file);
    }

    if (file_buffer != NULL) {
        free(file_buffer);
    }

    return model;
}

void destroy_mesh(Mesh mesh) {
    opengl_destroy_vertex_array(mesh.vaoID);

    if(mesh.vboID) {
        for(uint32_t i=0; i<MAX_MESH_VERTEX_BUFFERS; i++) {
            opengl_destroy_vertex_buffer(mesh.vboID[i]);
        }
    }

    free(mesh.vboID);

    free(mesh.vertices);
    free(mesh.normals);
    free(mesh.tangents);
    free(mesh.texcoords);
    free(mesh.texcoords2);
    free(mesh.indices);
}

void destroy_model(Model* model) {
    for(uint32_t i=0; i<model->mesh_count; i++) {
        destroy_mesh(model->meshes[i]);
    }

    free(model->meshes);
    free(model);
}

AABB get_mesh_AABB(Mesh mesh) {
    vec3 min = GLM_VEC3_ZERO_INIT;
    vec3 max = GLM_VEC3_ZERO_INIT;

    if(mesh.vertices != NULL) {
        {
            vec3 temp = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
            glm_vec3_copy(temp, min);
            glm_vec3_copy(temp, max);
        }

        for(uint32_t i=1; i<mesh.vertex_count; i++) {
            vec3_min(&min, min, (vec3){ mesh.vertices[3*i], mesh.vertices[3*i+1], mesh.vertices[3*i+2] });
            vec3_max(&max, max, (vec3){ mesh.vertices[3*i], mesh.vertices[3*i+1], mesh.vertices[3*i+2] });
        }
    }

    AABB aabb = {0};
    glm_vec3_copy(min, aabb.min);
    glm_vec3_copy(max, aabb.max);

    return aabb;
}

AABB get_model_AABB(Model model) {
    AABB aabb = {0};
    vec3 out = {0};

    if(model.mesh_count > 0) {
        vec3 temp = {0};
        aabb = get_mesh_AABB(model.meshes[0]);

        for(uint32_t i=1; i<model.mesh_count; i++) {
            AABB mesh_aabb = get_mesh_AABB(model.meshes[i]);
            vec3_min(&temp, aabb.min, mesh_aabb.min);
            vec3_max(&temp, aabb.max, mesh_aabb.max);
            glm_vec3_copy(temp, aabb.min);
            glm_vec3_copy(temp, aabb.max);
        }
    }


    glm_mat4_mulv(model.transform, aabb.min, out);
    glm_vec3_copy(out, aabb.min);

    glm_mat4_mulv(model.transform, aabb.max, out);
    glm_vec3_copy(out, aabb.max);

    return aabb;
}

#undef CGLTF_IMPLEMENTATION
