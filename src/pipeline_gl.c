#include <pipeline.h>
#include <utils.h>

void pipeline_init(graphics_pipeline* pipeline) {
    memset(pipeline, 0, sizeof(graphics_pipeline));

    pipeline->pipeline_core.textureCount = 0;
    pipeline->pipeline_core.textures = (char**)mem_alloca(16 * sizeof(char*));

    for (int i = 0; i < 16; i++) {
        pipeline->pipeline_core.textures[i] = (char*)mem_alloca(128 * sizeof(char));
        pipeline->pipeline_core.textures[i][0] = '\0';
    }

    pipeline->pipeline_core.textureValues = (int*)mem_alloca(16 * sizeof(int));
    pipeline->pipeline_core.programId = glCreateProgram();

    pipeline->pipeline_core.programId = pipeline->pipeline_core.programId;
    snprintf(pipeline->pipeline_core.textures[0], 128, "u_texture");
}

void pipeline_destroy(graphics_pipeline* pipeline) {
    for (int i = 0; i < 16; i++) {
        free(pipeline->pipeline_core.textures[i]);
    }

    free(pipeline->pipeline_core.textures);
    free(pipeline->pipeline_core.textureValues);
    glDeleteProgram(pipeline->pipeline_core.programId);
}

void pipeline_compile(uint32_t num_shdrs, graphics_pipeline* pipeline, shader* shaders[num_shdrs]) {
    for(int i = 0; i < num_shdrs; i++) {
        shader* current = shaders[i];
        compile_shader(current);
        glAttachShader(pipeline->pipeline_core.programId, *current->shader);

        for (int j = 0; j < current->num_attrs; j++) {
            shader_attribute* attr = current->attributes + j;
            glBindAttribLocation(pipeline->pipeline_core.programId, attr->location, attr->name);
        }
    }

    glLinkProgram(pipeline->pipeline_core.programId);
    print_shader_log_info(pipeline->pipeline_core.programId);

    int link_status;
    glGetProgramiv(pipeline->pipeline_core.programId, GL_LINK_STATUS, &link_status);

    if (link_status == GL_FALSE) {
        printf("Failed to link program\n");
        exit(1);
    }
}
