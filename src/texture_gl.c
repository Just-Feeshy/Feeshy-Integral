#include <texture.h>
#include <opengl.h>
#include <more_math.h>

#define K1 0.366025404 // (sqrt(3)-1)/2;
#define K2 0.211324865 // (3-sqrt(3))/6;

void hash_noise(vec2 p, vec2 result) {
    float dot1 = glm_vec2_dot(p, (vec2){127.1f, 311.7f});
    float dot2 = glm_vec2_dot(p, (vec2){269.5f, 183.3f});

    float sin1 = sinf(dot1) * 43758.5453123f;
    float sin2 = sinf(dot2) * 43758.5453123f;

    result[0] = -1.0f + 2.0f * (sinf(sin1) - floorf(sinf(sin1)));
    result[1] = -1.0f + 2.0f * (sinf(sin2) - floorf(sin2));
}

static float simplex_noise(vec2 p) {
vec2 i, a, b, c, o;
    glm_vec2_adds(p, (p[0] + p[1]) * K1, i);
    glm_vec2_floor(i, i);

    glm_vec2_sub(p, i, a);
    glm_vec2_adds(a, (i[0] + i[1]) * K2, a);

    float m = (a[0] > a[1]) ? 1.0f : 0.0f;
    glm_vec2_copy((vec2){m, 1.0f - m}, o);

    glm_vec2_sub(a, o, b);
    glm_vec2_adds(b, K2, b);

    glm_vec2_sub(a, (vec2){1.0f, 1.0f}, c);
    glm_vec2_adds(c, 2.0f * K2, c);

    vec3 h = {0.5f - glm_vec2_dot(a, a), 0.5f - glm_vec2_dot(b, b), 0.5f - glm_vec2_dot(c, c)};
    glm_vec3_maxv(h, (vec3){0.0f, 0.0f, 0.0f}, h);

    glm_vec3_mul(h, h, h);

    vec2 r0, r1, r2;
    hash_noise(i, r0);
    hash_noise((vec2){i[0] + o[0], i[1] + o[1]}, r1);
    hash_noise((vec2){i[0] + 1.0f, i[1] + 1.0f}, r2);

    float n0 = glm_vec2_dot(a, r0);
    float n1 = glm_vec2_dot(b, r1);
    float n2 = glm_vec2_dot(c, r2);

    vec3 contributions = {n0, n1, n2};
    glm_vec3_mul(h, h, h);
    glm_vec3_mul(h, contributions, h);

    float result = glm_vec3_dot(h, (vec3){70.0f, 70.0f, 70.0f});
    return result;
}

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
}

void texture_bind(texture* tex, unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, (unsigned)tex->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

image* create_simplex_noise(int width, int height) {
    image* img = (image*)malloc(sizeof(image));
    img->width = width;
    img->height = height;
    img->channels = 1;
    img->data = (uint8_t*)malloc(width * height * img->channels);

    for(int y=0; y<height; y++) {
        for(int x=0; x<width; x++) {
            float n = simplex_noise((vec2){x, y});
            printf("%f\n", n);
            img->data[y * width + x] = (uint8_t)(n * 255);
        }
    }

    return img;
}
