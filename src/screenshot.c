#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <screenshot.h>
#include <stb_image_write.h>
#include <opengl.h>
#include <stdlib.h>

static void flipY(int width, int height, char** data) {
    int rowLength = width * 3; // Aka. stride
    char* temp = malloc(rowLength);

    if(temp == NULL) {
        printf("Failed to allocate memory for screenshot!\n");
        return;
    }

    for(int i = 0; i < height / 2; i++) {
        int top = i * rowLength;
        int bottom = (height - i - 1) * rowLength;

        memcpy(temp, *data + top, rowLength);
        memcpy(*data + top, *data + bottom, rowLength);
        memcpy(*data + bottom, temp, rowLength);
    }

    free(temp);
}

void capture_screenshot(int w, int h) {
    char* data = malloc(4 * w * h);

    if(data == NULL) {
        printf("Failed to allocate memory for screenshot!\n");
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);

    flipY(w, h, &data);

    if(!stbi_write_png("screenshot.png", w, h, 4, data, w * 4)) {
        printf("Failed to write screenshot!\n");
    }
}
