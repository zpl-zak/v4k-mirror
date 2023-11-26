#include "v4k.h"

int main() {
    window_create(65.0, 0 );

    gui_pushskin(gui_skinned("golden.ase", 4.0f));
    skinned_t *skinned = C_CAST(skinned_t*, gui_userdata());

    vec4 pos = vec4(400,400,100, 30);

    while( window_swap() && !input(KEY_ESC) ) { // game loop
        vec4 panel_pos = vec4(0, 0, window_width(), window_height());
        
        if (ui_panel("Atlas", 0)) {
            ui_atlas(&skinned->atlas);
            ui_panel_end();
        }

        if (ui_panel("GUI", 0)) {
            ui_float("Scale", &skinned->scale);

            ui_panel_end();
        }

        if (input_down(MOUSE_R)) {
            pos.x = input(MOUSE_X);
            pos.y = input(MOUSE_Y);
        }
        if (input(MOUSE_R)) {
            pos.z = input(MOUSE_X)-pos.x;
            pos.w = input(MOUSE_Y)-pos.y;
        }

        //
        gui_panel(panel_pos, 0);
        if (gui_button(pos, 0)) {
            printf("%s\n", "Button pressed!");
        }

        gui_panel(vec4(40,140, 320, 40), "vial");
        gui_panel(vec4(40+9*skinned->scale,140+2*skinned->scale, 200, 64), "hp");

        gui_panel(vec4(40,230, 280, 40), "vial");
        gui_panel(vec4(40+9*skinned->scale,230+2*skinned->scale, 280-18*skinned->scale, 64), "mp");
    }

    gui_popskin();
}
