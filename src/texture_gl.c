#include <texture.h>
#include <opengl.h>

void texture_init(texture* tex, image* img) {
    tex->width = img->width;
    tex->height = img->height;
    tex->depth = 1;

    switch(img->channels) {
        case 1: tex->image_format = GL_RED; break;
        case 2: tex->image_format = GL_RG; break;
        case 3: tex->image_format = GL_RGB; break;
        case 4: tex->image_format = GL_RGBA; break;
        default: fprintf(stderr, "Invalid number of channels\n"); exit(EXIT_FAILURE);
    }

    if(img->data == NULL) {
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

    glTexImage2D(GL_TEXTURE_2D, 0, tex->image_format, tex->width, tex->height, 0, tex->image_format, GL_UNSIGNED_BYTE, img->data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void texture_array_init(unsigned layer_count, texture* tex, image images[layer_count]) {
    tex->width = images[0].width;
    tex->height = images[0].height;
    tex->depth = layer_count;

    for (int i = 0; i < layer_count; i++) {
        if (images[i].data == NULL) {
            fprintf(stderr, "Invalid data for image layer %d\n", i);
            exit(EXIT_FAILURE);
        }

        if (images[i].width != tex->width || images[i].height != tex->height) {
            fprintf(stderr, "Inconsistent image dimensions at layer %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    switch(images[0].channels) {
        case 1: tex->image_format = GL_RED; break;
        case 2: tex->image_format = GL_RG; break;
        case 3: tex->image_format = GL_RGB; break;
        case 4: tex->image_format = GL_RGBA; break;
        default: fprintf(stderr, "Invalid number of channels\n"); exit(EXIT_FAILURE);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (unsigned*)&tex->texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, tex->image_format, tex->width, tex->height, tex->depth, 0, tex->image_format, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < layer_count; i++) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tex->width, tex->height, 1, tex->image_format, GL_UNSIGNED_BYTE, images[i].data);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void texture_array_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
