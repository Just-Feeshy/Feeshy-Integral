#pragma once

typedef struct vertices {
    float bottom_left_x;
    float bottom_left_y;

    float bottom_right_x;
    float bottom_right_y;

    float top_right_x;
    float top_right_y;

    float top_left_x;
    float top_left_y;
} vertices;

void create_vertex_buffer(unsigned* VAO, vertices* v);
void draw_vertex_buffer(unsigned VAO, unsigned num_vertices);
