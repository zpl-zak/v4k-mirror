// hello ui: config, window, system, ui, video
// - rlyeh, public domain.
//
// Compile with:
//    `make     demos\01-ui.c` (windows)
// `sh MAKE.bat demos/01-ui.c` (linux, osx)

#include "v4k.h"

int main() {
    window_create(80, 0);
    window_title(__FILE__);

    camera_t cam = camera();
    vec3 route[] = {
        vec3(0.0f, 0.0f, 5.0f),
        vec3(0.0f, 0.0f, 10.0f),
        vec3(0.0f, 2.0f, 20.0f),
        vec3(0.0f, 8.0f, 30.0f),
        vec3(0.0f, 12.0f, 40.0f),
        vec3(0.0f, 0.0f, 80.0f),
    };

    #define route_len (sizeof route / sizeof route[0])
    
    // app loop
    while( window_swap() ) {
        // input controls
        if( input(KEY_ESC) ) break;

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        ddraw_ground(0);

        for (int i = 0; i < route_len; i++) {
            
        }
    }
}
