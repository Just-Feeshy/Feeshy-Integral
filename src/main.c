#include <program.h>
#include <screen.h>
#include <uniform_manager.h>

const int width = 800;
const int height = 600;

int main(int argc, char** argv) {
    uniform_manager_init();
    program_init("Eath", width, height);
    screen_init(800, 600);

    program_loop();
    program_destroy();
    return 0;
}
