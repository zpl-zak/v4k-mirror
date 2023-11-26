#include "v4k.h"

int main() {
    window_create(65.0, 0 );

    gui_pushskin(gui_skinned("golden.ase", 4.0));
    atlas_t *atlas = &C_CAST(skinned_t*, gui_userdata())->atlas;

    vec4 pos = vec4(100,100,850,750);

    while( window_swap() && !input(KEY_ESC) ) { // game loop
        vec4 panel_pos = vec4(0, 0, window_width(), window_height());
        
        if (ui_panel("Atlas", 0)) {
            ui_atlas(atlas);
            ui_panel_end();
        }

        if (input_down(MOUSE_R)) {
            pos.x = input(MOUSE_X);
            pos.y = input(MOUSE_Y);
        }
        if (input(MOUSE_R)) {
            pos.z = input(MOUSE_X);
            pos.w = input(MOUSE_Y);
        }

        //

        gui_panel(panel_pos, 0);
        if (gui_button(pos, 0)) {
            printf("%s\n", "Button pressed!");
        }
    }

    gui_popskin();
}
