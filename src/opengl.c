#include <opengl.h>

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
