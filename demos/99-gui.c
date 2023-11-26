#include "v4k.h"

int main() {
    window_create(75.0, 0); // 75% size, no extra flags

    font_face(FONT_FACE2, "lilita_one_regular.ttf", 32.0f, FONT_EU | FONT_2048);

    gui_pushskin(gui_skinned("golden.ase", 1.0f));
    atlas_t *atlas = &C_CAST(skinned_t*, gui_userdata())->atlas;

    vec4 pos = vec4(100,100,350,300);

    while( window_swap() && !input(KEY_ESC) ) { // game loop
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
        if (gui_button(pos, "button")) {
            printf("%s\n", "Button pressed!");
        }

        font_color(FONT_COLOR9, WHITE);
        font_print(va(FONT_MIDDLE FONT_CENTER FONT_FACE2 FONT_COLOR9 "%s", "Hello"));
    }

    gui_popskin();
}
