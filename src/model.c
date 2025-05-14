#define CGLTF_IMPLEMENTATION
#include <model.h>
#include <opengl.h>
#include <core.h>
#include <cgltf.h>
#include <string.h>

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
#define MAX_MATERIAL_MAPS 12

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
    if(file) {
        SDL_RWclose(file);
    }

    if(result != cgltf_result_success) {
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

static image load_image_from_gltf(cgltf_image* gltf_image, const char* path) {
    image img = {0};

    if(gltf_image == NULL) {
        return img;
    }

    if(gltf_image->uri) {
        if(strlen(gltf_image->uri) > 5
        && gltf_image->uri[0] == 'd'
        && gltf_image->uri[1] == 'a'
        && gltf_image->uri[2] == 't'
        && gltf_image->uri[3] == 'a'
        && gltf_image->uri[4] == ':') {
            int i = 0;
            while(gltf_image->uri[i] != ',' && gltf_image->uri[i] != '\0') {
                i++;
            }

            if(gltf_image->uri[i] == '\0') {
                SDL_Log("Invalid GLTF data URI: %s\n", gltf_image->uri);
            }else {
                int base64_len = (int)strlen(gltf_image->uri + i + 1);
                while(gltf_image->uri[i + base64_len] == '=') base64_len--;
                int number_of_encoded_bits = base64_len * 6 - (base64_len * 6) % 8;
                int out_size = number_of_encoded_bits / 8;
                void* data = NULL;

                cgltf_options options = {0};
                options.file.read = load_gltf_callback;
                options.file.release = free_gltf_callback;
                cgltf_result result = cgltf_load_buffer_base64(&options, out_size, gltf_image->uri + i + 1, &data);

                if(result == cgltf_result_success) {
                    img = load_image_raw(data, out_size);
                    free(data);
                }
            }
        }else {
            char full_path[MAX_FILEPATH_LENGTH];
            SDL_snprintf(full_path, MAX_FILEPATH_LENGTH, "%s/%s", path, gltf_image->uri);
            img = load_image(full_path);
        }
    }else if(gltf_image->buffer_view != NULL && gltf_image->buffer_view->buffer->data != NULL) {
        uint8_t* data = (uint8_t*)malloc(gltf_image->buffer_view->size);
        int offset = gltf_image->buffer_view->offset;
        int stride = gltf_image->buffer_view->stride ? gltf_image->buffer_view->stride : 1;

        for(uint32_t i=0; i<gltf_image->buffer_view->size; i++) {
            data[i] = ((uint8_t*)gltf_image->buffer_view->buffer->data)[offset];
            offset += stride;
        }

        img = load_image_raw(data, (uint32_t)gltf_image->buffer_view->size);
        free(data);
    }

    return img;
}

// Inspired by the glTF loader from Raylib
// Credits go to raysan5
// Modified slightly by me
static Model load_model_gltf(const char* path) {
    /*********************************************************************************************

        Function implemented by Wilhem Barbier(@wbrbr), with modifications by Tyler Bezera(@gamerfiend)
        Transform handling implemented by Paul Melis (@paulmelis).
        Reviewed by Ramon Santamaria (@raysan5)
        Further modified done by Diego Fonseca (@just-feeshy)

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

    #define LOAD_ATTRIBUTE_CAST(accessor, numComp, srcType, dstPtr, dstType) \
    { \
        uint8_t* raw = (uint8_t*)accessor->buffer_view->buffer->data; \
        raw += accessor->buffer_view->offset + accessor->offset; \
        int stride = accessor->stride ? accessor->stride : sizeof(srcType) * numComp; \
        \
        for (unsigned int k = 0; k < accessor->count; k++) { \
            srcType* src = (srcType*)(raw + k * stride); \
            for (int l = 0; l < numComp; l++) { \
                dstPtr[numComp * k + l] = (dstType)src[l]; \
            } \
        } \
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
        model.material_count = data->materials_count + 1;
        model.materials = (Material*)calloc(model.material_count, sizeof(Material));
        model.materials[0] = load_material_default();
        model.mesh_material = (int*)calloc(model.mesh_count, sizeof(int));

        for(uint32_t i=0, j=1; i<data->materials_count; i++, j++) {
            model.materials[j] = load_material_default();
            const char* tex_path = get_directory_path(path);

            if(data->materials[i].has_pbr_metallic_roughness) {
                printf("MODEL: [%s] Material %d has PBR metallic roughness\n", path, i);

                // Base color texture
                if(data->materials[i].pbr_metallic_roughness.base_color_texture.texture) {
                    image im_albedo = load_image_from_gltf(data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, tex_path);
                    if(im_albedo.data != NULL) {
                        model.materials[j].maps[MATERIAL_MAP_ALBEDO].texture = texture_init(im_albedo);
                        free(im_albedo.data);
                    }
                }

                /*
                // Normal texture
                if(data->materials[i].normal_texture.texture) {
                    image im_normal = load_image_from_gltf(data->materials[i].normal_texture.texture->image, tex_path);

                    if(im_normal.data) {
                        model.materials[j].maps[MATERIAL_MAP_NORMAL].texture = texture_init(im_normal);
                        free(im_normal.data);
                    }
                }

                // Occlusion texture (Most important)
                if(data->materials[i].occlusion_texture.texture) {
                    image im_occlusion = load_image_from_gltf(data->materials[i].occlusion_texture.texture->image, tex_path);

                    if(im_occlusion.data) {
                        model.materials[j].maps[MATERIAL_MAP_OCCLUSION].texture = texture_init(im_occlusion);
                        free(im_occlusion.data);
                    }
                }
                */
            }
        }

        uint32_t mesh_index = 0;
        for(uint32_t i=0; i<data->nodes_count; i++) {
            cgltf_node* node = &data->nodes[i];
            cgltf_mesh* mesh = node->mesh;

            if(!mesh) {
                continue;
            }

            cgltf_float world_transform[16];
            cgltf_node_transform_world(node, world_transform);

            mat4 world_matrix = {
                world_transform[0], world_transform[4], world_transform[8],  world_transform[12],
                world_transform[1], world_transform[5], world_transform[9],  world_transform[13],
                world_transform[2], world_transform[6], world_transform[10], world_transform[14],
                world_transform[3], world_transform[7], world_transform[11], world_transform[15]
            };

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
                                vec4 pos = { vertices[3*l], vertices[3*l+1], vertices[3*l+2], 1.0f };
                                vec4 transformed;

                                glm_mat4_mulv(world_matrix, pos, transformed);

                                vertices[3*l]   = transformed[0];
                                vertices[3*l+1] = transformed[1];
                                vertices[3*l+2] = transformed[2];
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
                                vec4 n = { normals[3*l], normals[3*l+1], normals[3*l+2], 1.0f };
                                vec4 transformed;

                                glm_mat4_mulv(normal_matrix, n, transformed);

                                normals[3*l]   = transformed[0];
                                normals[3*l+1] = transformed[1];
                                normals[3*l+2] = transformed[2];
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
                                vec4 t = { tangents[3*l], tangents[3*l+1], tangents[3*l+2], 1.0f };
                                vec4 transformed;

                                glm_mat4_mulv(world_matrix, t, transformed);

                                tangents[3*l]   = transformed[0];
                                tangents[3*l+1] = transformed[1];
                                tangents[3*l+2] = transformed[2];
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
                            if(texcoordPtr) {
                                free(texcoordPtr);
                            }
                        }
                    }
                }


                // Indices
                if(mesh->primitives[j].indices != NULL && mesh->primitives[j].indices->buffer_view != NULL) {
                    cgltf_accessor* attribute = mesh->primitives[j].indices;
                    model.meshes[mesh_index].triangle_count = (uint32_t)attribute->count / 3;

                    #ifdef SUPPORT_32_BIT_INDICES
                    #define INDICES_TYPE uint32_t
                    #else
                    #define INDICES_TYPE uint16_t
                    #endif

                    if(attribute->component_type == cgltf_component_type_r_16u) {

                        model.meshes[mesh_index].indices = malloc(attribute->count * sizeof(INDICES_TYPE));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, uint16_t, model.meshes[mesh_index].indices, INDICES_TYPE);
                    }else if(attribute->component_type == cgltf_component_type_r_8u) {

                        model.meshes[mesh_index].indices = malloc(attribute->count * sizeof(INDICES_TYPE));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, uint8_t, model.meshes[mesh_index].indices, INDICES_TYPE);
                    }else if(attribute->component_type == cgltf_component_type_r_32u) {

                        model.meshes[mesh_index].indices = malloc(attribute->count * sizeof(INDICES_TYPE));
                        LOAD_ATTRIBUTE_CAST(attribute, 1, uint32_t, model.meshes[mesh_index].indices, INDICES_TYPE);

                        #ifndef SUPPORT_32_BIT_INDICES
                        SDL_Log("MODEL: [%s] Model has 32 bit indices, truncating to 16 bit\n", path);
                        #endif
                    }else {
                        SDL_Log("MODEL: [%s] Model has unsupported index type\n", path);
                    }

                    #undef INDICES_TYPE
                }else {
                    model.meshes[mesh_index].triangle_count = model.meshes[mesh_index].vertex_count / 3;
                }

                for(uint32_t l=0; l<data->materials_count; l++) {
                    if(&data->materials[l] == mesh->primitives[j].material) {
                        model.mesh_material[mesh_index] = l + 1;
                        break;
                    }
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

Material load_material_default() {
    Material material = {0};
    material.maps = (MaterialMap*)calloc(MAX_MATERIAL_MAPS, sizeof(MaterialMap));
    material.maps[MATERIAL_MAP_ALBEDO].texture = NULL;
    return material;
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

    for(uint32_t i=0; i<model->material_count; i++) {
        free(model->materials[i].maps[MATERIAL_MAP_ALBEDO].texture);
        free(model->materials[i].maps[MATERIAL_MAP_NORMAL].texture);
        free(model->materials[i].maps[MATERIAL_MAP_OCCLUSION].texture);
        free(model->materials[i].maps[MATERIAL_MAP_ROUGHNESS].texture);
        free(model->materials[i].maps[MATERIAL_MAP_METALNESS].texture);
        free(model->materials[i].maps[MATERIAL_MAP_EMISSION].texture);
        free(model->materials[i].maps[MATERIAL_MAP_HEIGHT].texture);
        free(model->materials[i].maps[MATERIAL_MAP_IRRADIANCE].texture);
        free(model->materials[i].maps[MATERIAL_MAP_PREFILTER].texture);
        free(model->materials[i].maps[MATERIAL_MAP_BRDF].texture);

        free(model->materials[i].maps);
    }

    free(model->meshes);
    free(model->materials);
    free(model->mesh_material);
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

#include <opengl.h>
#include <uniform_manager.h>

void upload_mesh(Mesh* mesh) {
    if(mesh->vaoID > 0) {
        SDL_Log("MODEL: Mesh already uploaded\n");
        return;
    }

    mesh->vboID = (uint32_t*)calloc(MAX_MESH_VERTEX_BUFFERS, sizeof(uint32_t));

    mesh->vaoID = 0;
    mesh->vboID[POSITION_ATTR_LOCATION] = 0;
    mesh->vboID[TEXCOORD_ATTR_LOCATION] = 0;

    opengl_gen_vertex_arrays(1, &mesh->vaoID);
    opengl_bind_vertex_array(mesh->vaoID);

    float* vertices = mesh->vertices;
    mesh->vboID[POSITION_ATTR_LOCATION] = opengl_load_vertex_buffer(vertices, sizeof(float) * mesh->vertex_count * 3);
    opengl_set_vertex_attr(POSITION_ATTR_LOCATION, 3, GL_FLOAT, 0, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(POSITION_ATTR_LOCATION);

    mesh->vboID[TEXCOORD_ATTR_LOCATION] = opengl_load_vertex_buffer(mesh->texcoords, sizeof(float) * mesh->vertex_count * 2);
    opengl_set_vertex_attr(TEXCOORD_ATTR_LOCATION, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(TEXCOORD_ATTR_LOCATION);

    if(mesh->normals != NULL) {
        void* normals = mesh->normals;
        mesh->vboID[NORMAL_ATTR_LOCATION] = opengl_load_vertex_buffer(normals, sizeof(float) * mesh->vertex_count * 3);
        opengl_set_vertex_attr(NORMAL_ATTR_LOCATION, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(NORMAL_ATTR_LOCATION);
    }else {
        float vertices[3] = {0.0f, 0.0f, 1.0f};
        opengl_set_vertex_attr_default(NORMAL_ATTR_LOCATION, vertices, GL_SHADER_ATTR_VEC3, 3);
        glDisableVertexAttribArray(NORMAL_ATTR_LOCATION);
    }

    if(mesh->indices != NULL) {
        mesh->vboID[INDICES_ATTR_LOCATION] = opengl_load_vertex_buffer(mesh->indices, sizeof(uint16_t) * mesh->triangle_count * 3);
    }

    if(mesh->vaoID > 0) {
        SDL_Log("MODEL: [ID %i] Mesh uploaded to VRAM successfully\n", mesh->vaoID);
    }else {
        SDL_Log("VBO: Mesh uploaded successfully to VRAM\n");
    }

    glBindVertexArray(0);
}

Model load_model(const char* path) {
    Model model = load_model_gltf(path);
    glm_mat4_identity(model.transform);

    if(model.mesh_count != 0 && model.meshes != NULL) {
        for(uint32_t i=0; i<model.mesh_count; i++) {
            upload_mesh(&model.meshes[i]);
        }
    }else {
        SDL_Log("MODEL: [%s] Model has no meshes\n", path);
    }

    return model;
}

void draw_model(Model model) {
    for(uint32_t i=0; i<model.mesh_count; i++) {
        draw_mesh(model.meshes[i], model.materials[model.mesh_material[i]]);
    }
}

void draw_mesh(Mesh mesh, Material material) {
    for(uint32_t i=0; i<MAX_MATERIAL_MAPS; i++) {
        if(material.maps[i].texture == NULL) {
            continue;
        }


        if(material.maps[i].texture->texture > 0) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, material.maps[i].texture->texture);
            set_uniform_int("u_texture", i);
        }
    }

    glBindVertexArray(mesh.vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboID[POSITION_ATTR_LOCATION]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vboID[TEXCOORD_ATTR_LOCATION]);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if(mesh.indices != NULL) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vboID[INDICES_ATTR_LOCATION]);

        #ifdef SUPPORT_32_BIT_INDICES
        glDrawElements(GL_TRIANGLES, mesh.triangle_count * 3, GL_UNSIGNED_INT, 0);
        #else
        glDrawElements(GL_TRIANGLES, mesh.triangle_count * 3, GL_UNSIGNED_SHORT, 0);
        #endif
    }else {
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
