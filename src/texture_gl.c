#define FNL_IMPL 1

#include <texture.h>
#include <opengl.h>
#include <more_math.h>
#include <FastNoiseLite.h>

#define K1 0.366025404 // (sqrt(3)-1)/2;
#define K2 0.211324865 // (3-sqrt(3))/6;

void texture_init(texture* tex, image* img) {
    tex->width = img->width;
    tex->height = img->height;
    tex->depth = 1;

    switch(img->channels) {
        case 1: tex->image_format = GL_RED_FORMAT; break;
        case 2: tex->image_format = GL_RG_FORMAT; break;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, tex->image_format, tex->width, tex->height, 0, tex->image_format, GL_UNSIGNED_BYTE, img->data);
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

image* create_simplex_noise(int width, int height) {
    image* img = (image*)malloc(sizeof(image));
    img->width = width;
    img->height = height;
    img->channels = 1;
    img->data = (uint8_t*)malloc(width * height * img->channels);

    fnl_state noise = fnlCreateState();
    noise.seed = 1337;
    noise.octaves = 6;
    noise.frequency = 0.01f;
    noise.noise_type = FNL_NOISE_CELLULAR;
    noise.fractal_type = FNL_FRACTAL_PINGPONG;

    for(int y=0; y<height; y++) {
        for(int x=0; x<width; x++) {
            float n = fnlGetNoise2D(&noise, x, y);
            uint8_t value = (uint8_t)fclamp((n - 0.5f) * -170.0f, 0.0f, 255.0f);
            img->data[y * width + x] = value;
        }
    }

    return img;
}
