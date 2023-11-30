#include "v4k.h"

int main() {
    window_create(65.0, 0 );

    gui_pushskin(gui_skinned("golden.ase", 4.0f));
    skinned_t *skinned = C_CAST(skinned_t*, gui_userdata());

    vec4 pos = vec4(400,400,100, 30);

    float testval=7.5f;
    float testval2=7.5f;

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
        gui_panel(panel_pos, "panel");
            if (gui_button_label(FONT_H1 "PRESS", pos, 0)) {
                printf("%s\n", "Button pressed!");
            }

            gui_rect(vec4(40,140, 320, 20*skinned->scale), "vial");
            gui_rect(vec4(40,140, 200, 14*skinned->scale), "hp");
            gui_rect(vec4(40,240, 240, 20*skinned->scale), "vial");
            gui_rect(vec4(40,240, 160, 14*skinned->scale), "mp");

            vec2 badge_size = gui_getskinsize("badge", 0);
            badge_size.x += 2; // padding
            gui_rect(vec4(60+badge_size.x*0,320, 1, 1), "badge");
            gui_rect(vec4(60+badge_size.x*1,320, 1, 1), "badge");
            gui_rect(vec4(60+badge_size.x*2,320, 1, 1), "badge_empty");

            vec2 slider_size = gui_getskinsize("slider", 0);
            gui_slider(vec4(60, 480, 80*skinned->scale, 1), 0, 0.0f, 15.0f, 1.0f, &testval);
            gui_slider_label(va(FONT_H1 "%.02f", testval2), vec4(60, 480+slider_size.y+10, 120*skinned->scale, 1), 0, -5.0f, 20.0f, 0.0f, &testval2);
        gui_panel_end();
    }

    gui_popskin();
}
