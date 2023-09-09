#define COOK_ON_DEMAND 1
#include "v4k.h"

#define TEMP_GPU

#ifdef TEMP_GPU
#define TEX_WIDTH 256
// #define TEX_WIDTH 64
#else
#define TEX_WIDTH 64

void temp_calc(vec4 *pixels){
    // flood it
    for (int y = 1; y < TEX_WIDTH-1; y++){
        for (int x = 1; x < TEX_WIDTH-1; x++){
            int idx = (y * TEX_WIDTH) + x;
            vec4 *m = &pixels[idx];

            vec4 sum = vec4(0,0,0,0);
            for (int cy = -1; cy <= 1; ++cy) {
                for (int cx = -1; cx <= 1; ++cx) {
                    sum = add4(sum, pixels[((y+cy)*TEX_WIDTH)+(x+cx)]);
                }
            }

            *m = scale4(sum,1/9.0f);
        }
    }
}

#endif

int main() {
    // 75% sized, MSAAx2
    window_create(50, WINDOW_SQUARE);
    window_title(__FILE__);
    // window_fps_lock(1);

    texture_t tex;

    vec4 *img = REALLOC(0, TEX_WIDTH*TEX_WIDTH*sizeof(vec4));
    for (int i=0; i <TEX_WIDTH*TEX_WIDTH; i++){
        img[i] = vec4(0.3,0.3,0.3,1);
    }
#ifdef TEMP_GPU
    tex = texture_create(TEX_WIDTH, TEX_WIDTH, 4, img, TEXTURE_LINEAR|TEXTURE_FLOAT);
    unsigned comp = compute(vfs_read("shaders/temperature.glsl"));
    shader_bind(comp);
    shader_image(tex, 0, 0, 0, READ_WRITE);
#else
    tex = texture_create(TEX_WIDTH, TEX_WIDTH, 4, img, TEXTURE_LINEAR|TEXTURE_FLOAT);
#endif

    while ( window_swap() && !input_down(KEY_ESC) ){
        if (input(KEY_F5)) window_reload();

#ifdef TEMP_GPU
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img);
#endif
        for (int i=0; i <TEX_WIDTH*4; i++){
            img[i] = vec4(0,0,1,1);
            img[(TEX_WIDTH - 4) * TEX_WIDTH+i] = vec4(1,0,0,1);
        }

        int enabled = !ui_active() && !ui_hover();
        vec4 mouse = enabled ? vec4(input(MOUSE_X),input(MOUSE_Y),input(MOUSE_L),input(MOUSE_R)) : vec4(0,0,0,0); // x,y,l,r
        int strong = input(KEY_LSHIFT)?15:1;
        int x = (int)clampf(floorf(TEX_WIDTH/(float)window_width() * mouse.x), 0, TEX_WIDTH-1);
        int y = (int)clampf(floorf(TEX_WIDTH/(float)window_height() * mouse.y), 0, TEX_WIDTH-1);

        if (mouse.z){
            img[(y*TEX_WIDTH)+x] = vec4(strong*1,0,0,1);
        }

        if (mouse.w){
            img[(y*TEX_WIDTH)+x] = vec4(0,0,strong*1,1);
        }

#ifdef TEMP_GPU        
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEX_WIDTH, TEX_WIDTH, 0, GL_RGBA, GL_FLOAT, img);
        shader_bind(comp);
        compute_dispatch(TEX_WIDTH/16, TEX_WIDTH/16, 1);
        image_write_barrier();
#else
        temp_calc(img);
        texture_update(&tex, TEX_WIDTH, TEX_WIDTH, 4, img, TEXTURE_LINEAR|TEXTURE_FLOAT);
#endif

        fullscreen_quad_rgb(tex, 2.2);
    }

    return 0;
}
