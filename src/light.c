#include <light.h>

void init_brdf_light_block(brdf_light_block* block) {
    *block = (brdf_light_block){
        .kB = 0.1f,
        .kT = 0.1f,
        .kR = 0.1f,
        .g = 0.1f,
    };
}
