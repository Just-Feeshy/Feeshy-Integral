#include <program.h>
#include <screen.h>
#include <uniform_manager.h>
#include <world.h>

const int width = 800;
const int height = 600;

void init() {
    uniform_manager_init();
    program_init("Eath", width, height);
    world_init();
    screen_init(800, 600);
}


int main(int argc, char** argv) {
    init();

    program_loop();
    program_destroy();
    return 0;
}
