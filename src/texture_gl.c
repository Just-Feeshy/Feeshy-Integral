#include <texture.h>
#include <opengl.h>

void texture_init(texture* tex, int width, int height, unsigned channels, uint8_t* data) {
    tex->width = width;
    tex->height = height;
    tex->depth = 1;

    switch(channels) {
        case 1: tex->image_format = GL_RED; break;
        case 2: tex->image_format = GL_RG; break;
        case 3: tex->image_format = GL_RGB; break;
        case 4: tex->image_format = GL_RGBA; break;
        default: fprintf(stderr, "Invalid number of channels\n"); exit(EXIT_FAILURE);
    }

    if(data == NULL) {
        fprintf(stderr, "Invalid data\n");
        exit(EXIT_FAILURE);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (unsigned*)&tex->texture);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, tex->image_format, tex->width, tex->height, 0, tex->image_format, GL_UNSIGNED_BYTE, data);
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
}
