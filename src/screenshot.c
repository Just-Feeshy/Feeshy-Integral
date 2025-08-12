#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef EMSCRIPTEN
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#else
#include <SDL_thread.h>
#include <SDL_mutex.h>
#endif

#include <screenshot.h>
#include <stb_image_write.h>
#include <program.h>
#include <texture.h>
#include <opengl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>  // _mkdir
#else
#include <unistd.h>
#endif

SDL_Thread* screenshot_thread = NULL;
SDL_mutex* screenshot_mutex = NULL;
SDL_cond* screenshot_cond = NULL;

image current_image;
bool screenshot_pending = false;
bool screenshot_thread_running = true;

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
//
// Okay, in a seriousness, the reason why it's multi-threaded is because
// it's because the screenshot function is blocking the main thread which
// causes the program to freeze for a few seconds and loop the screenshot
// input event since the freeze causes the program to not process any events
// and repeat the last event over and over again
//
// Plus, I don't want to block the main thread while saving the screenshot
// keeping it as smooth as possible without any lag
static void save_screenshot(void* raw_image) {
    SDL_Delay(100);  // Give the program some time to process the screenshot input event

    image img = *(image*)raw_image;
    char path[128];
    snprintf(path, sizeof(path), "screenshots/screenshot_%d.png", SDL_GetTicks());

#ifdef _WIN32
    _mkdir("../screenshot");
#else
    mkdir("../screenshot", 0755);  // Read/write/search for owner, read/search for others
#endif

    if(!stbi_write_png(path, img.width, img.height, 3, img.data, 0)) {
        printf("Failed to write screenshot!\n");
    }
}

int screenshot_worker(void* unused) {
    while (screenshot_thread_running) {
        SDL_LockMutex(screenshot_mutex);

        // Wait until there's a job or we’re shutting down
        while (!screenshot_pending && screenshot_thread_running)
            SDL_CondWait(screenshot_cond, screenshot_mutex);

        if (screenshot_pending) {
            // Copy image data locally if needed (optional safety)
            image img = current_image;
            screenshot_pending = false;
            SDL_UnlockMutex(screenshot_mutex);

            // Do the actual work (this part runs unlocked)
            save_screenshot(&img);
        } else {
            SDL_UnlockMutex(screenshot_mutex);
        }
    }

    return 0;
}

static void trigger_screenshot(image* img) {
    SDL_LockMutex(screenshot_mutex);

    if (!screenshot_pending) {
        current_image = *img;
        screenshot_pending = true;
        SDL_CondSignal(screenshot_cond);
    }

    SDL_UnlockMutex(screenshot_mutex);
}

void screenshot_init() {
    screenshot_mutex = SDL_CreateMutex();
    screenshot_cond = SDL_CreateCond();
    screenshot_thread = SDL_CreateThread(screenshot_worker, "screenshot_worker", NULL);
}

void capture_screenshot() {
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

    glFlush();
    trigger_screenshot(&img);

    free(data);
}
