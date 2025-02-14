#include <light.h>

brdf_light_block create_brdf_light_block() {
    return (brdf_light_block){
        .kB = 0.1f,
        .kT = 0.1f,
        .kR = 0.1f,
        .g = 0.1f,
    };
}
