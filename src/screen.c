#include <screen.h>

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


vertices v;

void screen_init(int w, int h) {
    if (w == 0 || h == 0) {
        return;
    }

    v = (vertices){
        .bottom_left_x = 0.0f,
        .bottom_left_y = 0.0f,

        .bottom_right_x = w,
        .bottom_right_y = 0.0f,

        .top_right_x = w,
        .top_right_y = h,

        .top_left_x = 0.0f,
        .top_left_y = h,
    };
}

void screen_render() {
}
