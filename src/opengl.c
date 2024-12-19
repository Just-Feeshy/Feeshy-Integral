#include <opengl.h>
#include <shader.h>

void opengl_init(SDL_Window* window) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void opengl_begin(SDL_Window* window) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    glViewport(0, 0, w, h);
}

void opengl_clear() {
    glClear(GL_COLOR_BUFFER_BIT);
}

const uint32_t opengl_get_integerv(GLenum pname) {
    int value;
    glGetIntegerv(pname, &value);
    return value;
}


// Shader For OpenGL

void apply_shader_type(shader* shader_obj, shader_type type) {
    switch(type) {
        case SHADER_VERTEX:
            shader_obj->type = GL_VERTEX_SHADER;
            break;
        case SHADER_FRAGMENT:
            shader_obj->type = GL_FRAGMENT_SHADER;
            break;
        case SHADER_GEOMETRY:
            shader_obj->type = GL_GEOMETRY_SHADER;
            break;
        default:
            shader_obj->type = GL_VERTEX_SHADER;
            break;
    }

    if(shader_obj->shader) {
        printf("Shader already exists\n");
        free(shader_obj->shader);
    }

    shader_obj->shader = malloc(sizeof(uint32_t));
    if (!shader_obj->shader) {
        fprintf(stderr, "Failed to allocate memory for shader\n");
        return;
    }

    *shader_obj->shader = glCreateShader(shader_obj->type);
}

void compile_shader(shader* shader_obj) {
    glShaderSource(*shader_obj->shader, 1, (const char**)&shader_obj->source, (int*)&shader_obj->size);
    glCompileShader(*shader_obj->shader);
    print_shader_log_info(*shader_obj->shader);

    int compile_status;
    glGetShaderiv(*shader_obj->shader, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE) {
        printf("Failed to compile shader\n");
        exit(1);
    }
}

void print_shader_log_info(unsigned shader) {
    int length = 0;
    int maxLength = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        char log[length];
        glGetShaderInfoLog(shader, length, &maxLength, log);

        fprintf(stderr, "Shader log: %s\n", log);
    }
}

void destroy_core_shader(shader* shader_obj) {
    glDeleteShader(*shader_obj->shader);
}
