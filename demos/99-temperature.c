#include "v4k.h"

int TEMP_GPU = 1;

#define TEX_WIDTH 1024

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

int main() {
    window_create(50, WINDOW_SQUARE|WINDOW_VSYNC_DISABLED);
    window_title(__FILE__);
    window_fps_unlock();

    texture_t tex;

    vec4 *img = REALLOC(0, TEX_WIDTH*TEX_WIDTH*sizeof(vec4));
    for (int i=0; i <TEX_WIDTH*TEX_WIDTH; i++){
        img[i] = vec4(0.3,0.3,0.3,1);
    }

    tex = texture_create(TEX_WIDTH, TEX_WIDTH, 4, img, TEXTURE_LINEAR|TEXTURE_FLOAT);
    unsigned comp = compute(vfs_read("shaders/temperature.glsl"));
    shader_bind(comp);
    shader_image(tex, 0, 0, 0, BUFFER_READ_WRITE);

    while ( window_swap() && !input_down(KEY_ESC) ){
        if (input_down(KEY_F5)) window_reload();
        if (input_down(KEY_F8)) TEMP_GPU ^= 1;

        if (TEMP_GPU) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img);
        }

        for (int i=0; i <TEX_WIDTH*64; i++){
            img[i] = vec4(0,0,1,1);
            img[(TEX_WIDTH - 64) * TEX_WIDTH+i] = vec4(1,0,0,1);
        }

        int enabled = !ui_active() && !ui_hover();
        vec4 mouse = enabled ? vec4(input(MOUSE_X),input(MOUSE_Y),input(MOUSE_L),input(MOUSE_R)) : vec4(0,0,0,0); // x,y,l,r
        int strong = input(KEY_LSHIFT)?5:1;
        int x = (int)clampf(floorf(TEX_WIDTH/(float)window_width() * mouse.x), 1, TEX_WIDTH-2);
        int y = (int)clampf(floorf(TEX_WIDTH/(float)window_height() * mouse.y), 1, TEX_WIDTH-2);

        if (mouse.z || mouse.w){
            for (int cy = -5*strong; cy <= 5*strong; ++cy) {
                for (int cx = -5*strong; cx <= 5*strong; ++cx) {
                    int px = (int)clampf(cx+x, 1, TEX_WIDTH-2);
                    int py = (int)clampf(cy+y, 1, TEX_WIDTH-2);
                    img[(py*TEX_WIDTH)+px] = vec4(mouse.z,0,mouse.w,1);
                }
            }
        }

        if (TEMP_GPU) {
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEX_WIDTH, TEX_WIDTH, 0, GL_RGBA, GL_FLOAT, img);
            shader_bind(comp);
            compute_dispatch(TEX_WIDTH/16, TEX_WIDTH/16, 1);
            write_barrier_image();
        } else {
            temp_calc(img);
            texture_update(&tex, TEX_WIDTH, TEX_WIDTH, 4, img, TEXTURE_LINEAR|TEXTURE_FLOAT);
        }
        fullscreen_quad_rgb(tex);
        ddraw_text2d(vec2(0,0), va("mode: %s\nimage res: %d\ninputs: %.01f %.01f %.01f %.01f %s", TEMP_GPU?"GPU compute":"CPU", TEX_WIDTH, mouse.x, mouse.y, mouse.z, mouse.w, strong>1?"lshift":""));
        ddraw_text2d(vec2(0,window_height() - 20), va("delta: %.2f ms", window_delta()*1000));
    }

    return 0;
}
