#include <opengl.h>
#include <shader.h>

<<<<<<< Updated upstream
void opengl_init(SDL_Window* window) {
=======
#ifdef USE_EGL2
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray = NULL;

#ifdef EXT_disjoint_timer_query
PFNGLGENQUERIESEXTPROC glGenQueries = NULL;
PFNGLBEGINQUERYEXTPROC glBeginQuery = NULL;
PFNGLENDQUERYEXTPROC glEndQuery = NULL;
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuiv = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v = NULL;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui_UNI = NULL;
#else
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectui_UNI = NULL;
#endif

#endif

#else

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui_UNI = NULL;
#else
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectui_UNI = NULL;
#endif

#endif

void opengl_init(SDL_Window* window) {

    #if defined(USE_EGL2)
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)SDL_GL_GetProcAddress("glGenVertexArraysOES");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)SDL_GL_GetProcAddress("glBindVertexArrayOES");

    #ifdef EXT_disjoint_timer_query
    glGenQueries = (PFNGLGENQUERIESEXTPROC)SDL_GL_GetProcAddress("glGenQueriesEXT");
    glBeginQuery = (PFNGLBEGINQUERYEXTPROC)SDL_GL_GetProcAddress("glBeginQueryEXT");
    glEndQuery = (PFNGLENDQUERYEXTPROC)SDL_GL_GetProcAddress("glEndQueryEXT");

    #if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    glGetQueryObjectui_UNI = (PFNGLGETQUERYOBJECTUI64VEXTPROC)SDL_GL_GetProcAddress("glGetQueryObjectui64vEXT");
    #else
    glGetQueryObjectui_UNI = (PFNGLGETQUERYOBJECTUIVEXTPROC)SDL_GL_GetProcAddress("glGetQueryObjectuivEXT");
    #endif

    #endif

    #else

    #if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    glGetQueryObjectui_UNI = (PFNGLGETQUERYOBJECTUI64VPROC)SDL_GL_GetProcAddress("glGetQueryObjectui64v");
    #else
    glGetQueryObjectui_UNI = (PFNGLGETQUERYOBJECTUIVPROC)SDL_GL_GetProcAddress("glGetQueryObjectuiv");
    #endif

    #endif

>>>>>>> Stashed changes
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void opengl_begin(SDL_Window* window) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    glViewport(0, 0, w, h);
}

<<<<<<< Updated upstream
=======
void opengl_gen_vertex_arrays(GLsizei n, GLuint* arrays) {
    #ifdef USE_EGL2
    if(!glGenVertexArrays) {
        printf("glGenVertexArrays not found\n");
        return;
    }
    #endif

    glGenVertexArrays(n, arrays);
}

void opengl_bind_vertex_array(GLuint array) {
    #ifdef USE_EGL2
    if(!glBindVertexArray) {
        printf("glBindVertexArray not found\n");
        return;
    }
    #endif

    glBindVertexArray(array);
}

void opengl_gen_queries(GLsizei n, GLuint* ids) {
    #ifdef EXT_disjoint_timer_query
    if(!glGenQueries) {
        printf("glGenQueries not found\n");
        return;
    }
    #endif

    glGenQueries(n, ids);
}

void opengl_begin_query(GLenum target, GLuint id) {
    #ifdef EXT_disjoint_timer_query
    if(!glBeginQuery) {
        printf("glBeginQuery not found\n");
        return;
    }
    #endif

    glBeginQuery(target, id);
}

void opengl_end_query(GLenum target) {
    #ifdef EXT_disjoint_timer_query
    if(!glEndQuery) {
        printf("glEndQuery not found\n");
        return;
    }
    #endif

    glEndQuery(target);
}

// TODO: Support for 32-bit systems
void opengl_get_query_objectuiv(GLuint id, GLenum pname, GLuint* params) {
    #ifdef EXT_disjoint_timer_query
    if(!glGetQueryObjectui_UNI) {
        printf("glGetQueryObjectui_UNI not found\n");
        return;
    }
    #endif

    glGetQueryObjectui_UNI(id, pname, params);
}

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
            shader_obj->type = GL_GEOMETRY_SHADER;
=======
            #ifdef GL_OES_geometry_shader
                #if defined(USE_EGL2)
                shader_obj->type = GL_GEOMETRY_SHADER_OES;
                #else
                shader_obj->type = GL_GEOMETRY_SHADER;
                #endif
            #else
            printf("Geometry shader not supported\n");
            exit(1);
            #endif

>>>>>>> Stashed changes
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
