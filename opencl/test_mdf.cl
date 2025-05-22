#include <mesh.clh>
#include <aabb.clh>

__kernel void test_kernel(__global float* distance_field) {
    int coord[3];
    coord[0] = get_global_id(0);
    coord[1] = get_global_id(1);
    coord[2] = get_global_id(2);
    int index = coord[0]
            + coord[1] * get_global_size(0)
            + coord[2] * get_global_size(0) * get_global_size(1);
    distance_field[index] = 1.0f;
    // write_imagef(out, coord, (float4)(1.0f, 0.0f, 0.0f, 1.0f));
}
