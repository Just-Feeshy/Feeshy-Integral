#include <buffers.h>
#include <opengl.h>

void create_vertex_buffer(unsigned* vao, vertices* v) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    unsigned vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), v, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void draw_vertex_buffer(unsigned vao, unsigned num_vertices) {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    glBindVertexArray(0);
}
