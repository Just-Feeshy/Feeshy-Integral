#include <geometry_pass.h>
#include <opengl.h>
#include <assert.h>

geometry_pass geometry_pass_init(RenderCallback render_callback, int width, int height, int texture_count) {
    assert(texture_count > 0);

    geometry_pass pass = {0};
    pass.texture_count = texture_count;
    pass.textures = (texture*)malloc(sizeof(texture) * texture_count);
    pass.textures[0].width = width;
    pass.textures[0].height = height;
    pass.textures[0].depth = 1;
    pass.textures[0].type = GL_TEXTURE_2D;
    pass.activate_wireframe = false;
    pass.pipeline = (graphics_pipeline*)malloc(sizeof(graphics_pipeline));
    pipeline_init(pass.pipeline);

    glGenFramebuffers(1, &pass.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, pass.framebuffer);

    glGenTextures(1, &pass.textures[0].texture);
    glBindTexture(GL_TEXTURE_2D, pass.textures[0].texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass.textures[0].texture, 0);

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

void geometry_pass_add_texture(geometry_pass* pass, texture tex) {
    pass->textures = (texture*)realloc(pass->textures, sizeof(texture) * (pass->texture_count + 1));
    assert(pass->textures != NULL);
    pass->textures[pass->texture_count] = tex;
    pass->texture_count++;
}

void geometry_pass_render(geometry_pass pass, uint32_t texture_offset) {
    glBindFramebuffer(GL_FRAMEBUFFER, pass.framebuffer);
    glViewport(0, 0, pass.textures[0].width, pass.textures[0].height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef EMSCRIPTEN
    if(pass.activate_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif

    pipeline_set(pass.pipeline);
    pass.render_callback(); // Here

#ifndef EMSCRIPTEN
    if(pass.activate_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for(uint32_t i=0; i<pass.texture_count; i++) {
        texture_bind(&pass.textures[i], i + texture_offset);
        // printf("Texture %d bound to unit %d\n", pass.textures[i].texture, i + texture_offset);
    }
}

void geometry_pass_destroy(geometry_pass pass) {
    glDeleteFramebuffers(1, &pass.framebuffer);
    pass.framebuffer = 0;

    for(uint32_t i=0; i<pass.texture_count; i++) {
        glDeleteTextures(1, &pass.textures[i].texture);
        pass.textures[i].texture = 0;
    }

    pipeline_destroy(pass.pipeline);

    free(pass.textures);
    free(pass.pipeline);
}
