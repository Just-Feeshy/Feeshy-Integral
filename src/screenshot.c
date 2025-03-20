#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYFD_FORCE_WCHAR

#include <screenshot.h>
#include <stb_image_write.h>
#include <tinyfiledialogs.h>
#include <program.h>
#include <texture.h>
#include <opengl.h>
#include <stdlib.h>

// https://github.com/vallentin/GLCollection/blob/master/screenshot.cpp
// I just copied this function from the above link
// I can't be bothered to write my own flip function as that's not the point of this project
static void flipY(int width, int height, uint8_t* data) {
    char rgb[3];

	for (int y = 0; y < height / 2; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int top = (x + y * width) * 3;
			int bottom = (x + (height - y - 1) * width) * 3;

			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

// Can't believe this worked on the first try
// Now I can say that I'm a professional C programmer (I'm not)
// and that this project is multi-threaded
static int save_screenshot(void* raw_image) {
    image img = *(image*)raw_image;
    const char* path = tinyfd_saveFileDialog("Save Screenshot", "screenshot.png", 1, (const char*[]){"*.png"}, "PNG Files");

    if(path) {
        if(!stbi_write_png(path, img.width, img.height, 3, img.data, 0)) {
            printf("Failed to write screenshot!\n");
        }
    }

    return 0;
}

void capture_screenshot() {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(1);
    program_set_as_escaped();

    GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

    int w = viewport[2];
    int h = viewport[3];

    uint8_t* data = malloc((size_t)(3 * w * h));

    if(data == NULL) {
        printf("Failed to allocate memory for screenshot!\n");
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);

    image img = {w, h, 3, data};
    flipY(w, h, data);

    // Yeah, I know, I'm taking it too far
    int threadReturnValue;
    SDL_Thread* screenshot_thread = SDL_CreateThread(save_screenshot, "screenshot", &img);
    SDL_WaitThread(screenshot_thread, &threadReturnValue);

    if(threadReturnValue != 0) {
        printf("Failed to save screenshot!\n");
    }

    free(data);
}
