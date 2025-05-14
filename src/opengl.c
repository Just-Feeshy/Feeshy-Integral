#include <opengl.h>
#include <shader.h>
#include <utils.h>
#include <assert.h>

void opengl_init(SDL_Window* window) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void opengl_begin(SDL_Window* window) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    glViewport(0, 0, w, h);
}

GLuint opengl_load_vertex_buffer(const void* buffer, size_t size) {
    GLuint buffer_id;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
    return buffer_id;
}

void opengl_gen_vertex_arrays(GLsizei n, GLuint* arrays) {
    glGenVertexArrays(n, arrays);
}

void opengl_set_vertex_attr(uint32_t index, uint32_t size, uint32_t type, bool normalized, uint32_t stride, uint32_t offset) {
    size_t offset_native = offset;
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset_native);
}

void opengl_set_vertex_attr_default(uint32_t index, const void* buffer, uint32_t attr_type, uint32_t count) {
    switch(attr_type) {
        case GL_SHADER_ATTR_FLOAT: if(count == 1) glVertexAttrib1fv(index, (const GLfloat*)buffer); break;
        case GL_SHADER_ATTR_VEC2: glVertexAttrib2fv(index, (const GLfloat*)buffer); break;
        case GL_SHADER_ATTR_VEC3: glVertexAttrib3fv(index, (const GLfloat*)buffer); break;
        case GL_SHADER_ATTR_VEC4: glVertexAttrib4fv(index, (const GLfloat*)buffer); break;
        default: SDL_Log("Unknown attribute type %d\n", attr_type); break;
    }
}

void opengl_destroy_vertex_array(GLuint array) {
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &array);
}

void opengl_destroy_vertex_buffer(GLuint buffer) {
    glDeleteBuffers(1, &buffer);
}

void opengl_bind_vertex_array(GLuint array) {
    glBindVertexArray(array);
}

void opengl_clear() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
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

    if(!*shader_obj->shader) {
        fprintf(stderr, "Failed to create shader\n");
        free(shader_obj->shader);
        return;
    }
}

void compile_shader(shader* shader_obj) {
    glShaderSource(*shader_obj->shader, 1, (const char**)&shader_obj->source, (int*)&shader_obj->size);
    glCompileShader(*shader_obj->shader);
    print_shader_log_info(*shader_obj->shader);

    int compile_status;
    glGetShaderiv(*shader_obj->shader, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE) {
        char log[512];
        glGetShaderInfoLog(*shader_obj->shader, 512, NULL, log);
        printf("Shader compilation failed: %s\n", log);
    }

    assert(compile_status == GL_TRUE);
}

void print_shader_log_info(unsigned shader) {
    if (!shader) {
        fprintf(stderr, "Invalid Shader ID %u\n", shader);
        return;
    }

    int length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    if (length > 1) {
        char* log = (char*)mem_alloca(length);
        if (!log) {
            fprintf(stderr, "Memory allocation failed!\n");
            return;
        }

        glGetShaderInfoLog(shader, length, NULL, log);
        log[length - 1] = '\0';  // Ensure null termination

        fprintf(stderr, "Shader Log: %s\n", log);
    }
}

void destroy_core_shader(shader* shader_obj) {
    glDeleteShader(*shader_obj->shader);
}
