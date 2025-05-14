#include <geometry_pass.h>
#include <opengl.h>

geometry_pass geometry_pass_init(RenderCallback render_callback, int width, int height) {
    geometry_pass pass = {0};
    pass.texture.width = width;
    pass.texture.height = height;

    glGenFramebuffers(1, &pass.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, pass.framebuffer);

    glGenTextures(1, &pass.texture.texture);
    glBindTexture(GL_TEXTURE_2D, pass.texture.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass.texture.texture, 0);

    glGenRenderbuffers(1, &pass.depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, pass.depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pass.depth_buffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer not complete\n");
        exit(EXIT_FAILURE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    pass.render_callback = render_callback;
    return pass;
}

void geometry_pass_render(geometry_pass pass) {
    glBindFramebuffer(GL_FRAMEBUFFER, pass.framebuffer);
    glViewport(0, 0, pass.texture.width, pass.texture.height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pass.render_callback(); // Here
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pass.texture.texture);
}

void geometry_pass_destroy(geometry_pass pass) {
    glDeleteFramebuffers(1, &pass.framebuffer);
    pass.framebuffer = 0;
}
