#include "v4k.h"
/*
float* billboard(mat44 modelview, bool billboard_x, bool billboard_y, bool billboard_z) {
    if( billboard_x ) { modelview[0*4+0] = 1; modelview[0*4+1] = 0; modelview[0*4+2] = 0; }
    if( billboard_y ) { modelview[1*4+0] = 0; modelview[1*4+1] = 1; modelview[1*4+2] = 0; }
    if( billboard_z ) { modelview[2*4+0] = 0; modelview[2*4+1] = 0; modelview[2*4+2] = 1; }
    return modelview;
}
*/
const char *billboard_name(bool billboard_x, bool billboard_y, bool billboard_z) {
    // Spherical billboarding makes the object to always face the camera no matter the position of the camera on an imaginary sphere.
    // Cylindrical billboarding makes the object to face the camera only when the camera looks at the right or at the left.
    return
    billboard_x && billboard_y && billboard_z ? "(spherical billboard)" : billboard_x && billboard_z ? "(cylindrical billboard)" : "";
}
int main() {
    // create the window
    window_create( 0.5f, 0 );
    window_color( RGB3(40,40,50) );

    model_t cube = model("Alien.fbx", 0);

    // create (fps) camera
    camera_t cam = camera();

    // main loop
    while (window_swap()) {
        if (input(KEY_ESC))
            break;

        // fps camera
        bool active = ui_hover() || ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdecq, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);

        // spin cube
        // rotate44(cube->transform, 1, 1,1,1);

        static int billboard_x = 0; if( input_down(KEY_1) ) billboard_x ^= 1;
        static int billboard_y = 0; if( input_down(KEY_2) ) billboard_y ^= 1;
        static int billboard_z = 0; if( input_down(KEY_3) ) billboard_z ^= 1;
        cube.billboard = (billboard_x << 2)|(billboard_y << 1)|(billboard_z << 0);

        model_render(cube, cam.proj, cam.view, cube.pivot);

        window_title(va("billboard_x: %s, billboard_y: %s, billboard_z: %s %s", billboard_x ? "on":"off", billboard_y ? "on":"off", billboard_z ? "on":"off", billboard_name(billboard_x,billboard_y,billboard_z)));
    }
}
