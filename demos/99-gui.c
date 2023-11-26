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

        gui_panel(vec4(40,140, 320, 20*skinned->scale), "vial");
        gui_panel(vec4(40,140, 200, 14*skinned->scale), "hp");
        gui_panel(vec4(40,240, 240, 20*skinned->scale), "vial");
        gui_panel(vec4(40,240, 160, 14*skinned->scale), "mp");

        vec2 badge_size = gui_getskinsize("badge");
        badge_size.x += 2; // padding
        gui_panel(vec4(60+badge_size.x*0,320, 1, 1), "badge");
        gui_panel(vec4(60+badge_size.x*1,320, 1, 1), "badge");
        gui_panel(vec4(60+badge_size.x*2,320, 1, 1), "badge_empty");
    }

    gui_popskin();
}
