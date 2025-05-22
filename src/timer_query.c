#include <timer_query.h>
#include <opengl.h>

static GLuint query;

void gpu_timer_query_init() {
    glGenQueries(1, &query);
}

void gpu_timer_query_begin() {
    #ifdef USE_ANGLE
    glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
    #else
    glBeginQuery(GL_TIME_ELAPSED, query);
    #endif
}

void gpu_timer_query_end() {
    #ifdef USE_ANGLE
    glEndQuery(GL_ANY_SAMPLES_PASSED);
    #else
    glEndQuery(GL_TIME_ELAPSED);
    #endif
}

uint32_t gpu_timer_query_result() {
    GLuint timeElapsed = 1;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &timeElapsed);
    return timeElapsed;
}
