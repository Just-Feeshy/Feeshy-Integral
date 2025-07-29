#ifdef WINDOWS
#include <SDL.h>
#endif

#include <program.h>
#include <uniform_manager.h>

const int width = 1920;
const int height = 1080;

void init() {
    uniform_manager_init();
    program_init("Eath", width, height);
}

int main(int argc, char** argv) {
    init();

    program_loop();
    program_destroy();
    return 0;
}
