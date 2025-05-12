#include <buffers.h>
#include <opengl.h>

void create_vertex_buffer(unsigned* vao, vertices v) {
    opengl_gen_vertex_arrays(1, vao);
    opengl_bind_vertex_array(*vao);

    unsigned vbo;

    #ifdef EMSCRIPTEN
    const size_t size_vertices = sizeof(vertices) * 2;
    #else
    const size_t size_vertices = sizeof(vertices);
    #endif

    vbo = opengl_load_vertex_buffer(&v, size_vertices);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    opengl_bind_vertex_array(0);
}

void draw_vertex_buffer(unsigned vao, unsigned num_vertices) {
    opengl_bind_vertex_array(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices);
    opengl_bind_vertex_array(0);
}
