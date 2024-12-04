#include <program.h>

int main(int argc, char** argv) {
    program_init("Eath", 800, 600);
    program_loop();
    program_destroy();
    return 0;
}
