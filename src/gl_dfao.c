#define CGLTF_IMPLEMENTATION
#define GRID_SIZE 100

#include <gl_dfao.h>
#include <opengl.h>
#include <assert.h>
#include <aabb.h>
#include <model.h>

bool init_dfao(uint32_t* fbo) {
    glGenFramebuffers(1, fbo);
    assert(glGetError() == GL_NO_ERROR);

    return true;
}

// I setup my texture in the framebuffer object this way because I don't really know
// or care to know how nuklear handles there textures.
void generate_dfao_scene(uint32_t* fbo, uint32_t source_tex_id) {
    GLenum status = 0;

    int32_t current_texture_2D;
    int32_t source_width = 0, source_height = 0;

    // Store off currently selected 2D texture
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture_2D);

    // Get our source texture size
    glBindTexture(GL_TEXTURE_2D, source_tex_id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &source_width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &source_height);

    // Put back the original texture
    glBindTexture(GL_TEXTURE_2D, current_texture_2D);

    // Setup our draw framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source_tex_id, 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

}

void destroy_dfao(uint32_t* fbo) {
    glDeleteFramebuffers(1, fbo);
    assert(glGetError() == GL_NO_ERROR);
}
