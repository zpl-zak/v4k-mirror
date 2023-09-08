#include "v4k.h"

int main() {
    // 75% sized, MSAAx2
    window_create(50, WINDOW_SQUARE);
    window_title(__FILE__);

    unsigned comp = compute(vfs_read("shaders/compute-test.glsl"));
    texture_t tex = texture_create(512, 512, 4, 0, TEXTURE_LINEAR|TEXTURE_FLOAT);

    while ( window_swap() && !input_down(KEY_ESC) ){
        if (input(KEY_F5)) window_reload();
        shader_bind(comp);
        shader_float("t", (float)window_time());
        shader_image(tex, 0, 0, -1, READ);
        dispatch(512, 512, 1);
        image_write_barrier();
        fullscreen_quad_rgb(tex, 2.2);
    }

    return 0;
}
