#include <program.h>
#include <screen.h>

const int width = 800;
const int height = 600;

int main(int argc, char** argv) {
    program_init("Eath", width, height);
    screen_init(width, height);

    program_loop();
    program_destroy();
    return 0;
}
