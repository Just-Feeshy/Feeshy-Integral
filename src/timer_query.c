#include <timer_query.h>
#include <opengl.h>

static GLuint query;

void gpu_timer_query_init() {
    glGenQueries(1, &query);
}

void gpu_timer_query_begin() {
    glBeginQuery(GL_TIME_ELAPSED, query);
}

void gpu_timer_query_end() {
    glEndQuery(GL_TIME_ELAPSED);
}

uint32_t gpu_timer_query_result() {
    GLuint timeElapsed = 1;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &timeElapsed);
    return timeElapsed;
}
